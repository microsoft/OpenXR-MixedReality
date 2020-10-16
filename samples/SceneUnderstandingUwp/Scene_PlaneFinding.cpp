//*********************************************************
//    Copyright (c) Microsoft. All rights reserved.
//
//    Apache 2.0 License
//
//    You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
//    implied. See the License for the specific language governing
//    permissions and limitations under the License.
//
//*********************************************************
#include "pch.h"
#include <future>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <XrUtility/XrString.h>
#include <XrUtility/XrSceneUnderstanding.h>
#include <pbr/GltfLoader.h>
#include <SampleShared/FileUtility.h>
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>
#include <XrSceneLib/SpaceObject.h>

//
// This sample shows the usage of scene understanding extensions
// When the user is walking around, newly discovered planes and meshes are displayed
//

using namespace std::chrono_literals;
using TimePoint = engine::FrameTime::clock::time_point;

namespace wrt {
    using namespace winrt::Windows::Storage;
    using namespace winrt::Windows::Storage::Streams;
    using winrt::Windows::Foundation::AsyncStatus;
    using winrt::Windows::Foundation::IAsyncOperation;
    using winrt::Windows::Security::Cryptography::CryptographicBuffer;
} // namespace wrt

namespace {
    const winrt::hstring SceneFileName(L"su_scene.bin");
    constexpr auto UpdateInterval = 2s; // Time to wait between SU requests
    constexpr float ScanRadius = 4.8f;  // meters
    constexpr size_t TextureSideLength = 32;

    struct SceneVisuals {
        SceneVisuals() = default;
        SceneVisuals(std::vector<std::shared_ptr<engine::Object>> visuals, std::vector<XrSceneObjectKeyMSFT> keys, xr::SceneHandle scene)
            : visuals(std::move(visuals))
            , keys(std::move(keys))
            , scene(std::move(scene)) {
        }

        std::vector<std::shared_ptr<engine::Object>> visuals;
        std::vector<XrSceneObjectKeyMSFT> keys;
        xr::SceneHandle scene;
    };

    SceneVisuals CreateSceneVisuals(const xr::ExtensionContext& extensions,
                                    const Pbr::Resources& pbrResources,
                                    const std::shared_ptr<Pbr::Material>& material,
                                    xr::SceneHandle scene);
    std::vector<uint32_t> CreateTileTextureBytes();
    wrt::IAsyncOperation<wrt::IBuffer> ReadBufferAsync(winrt::hstring filename);
    void SerializeScene(const xr::ExtensionContext& extensions,
                        xr::SceneObserverHandle&& sceneObserver,
                        XrSpace space,
                        XrTime time,
                        const XrSceneSphereBoundMSFT& sphere);

    struct PlaneFindingScene : public engine::Scene {
        explicit PlaneFindingScene(engine::Context& context)
            : Scene(context)
            , m_extensions(context.Extensions)
            , m_material{CreateTextureMaterial()}
            , m_nextUpdate{engine::FrameTime::clock::now() + UpdateInterval} {
            XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(context.Session.Handle, &spaceCreateInfo, m_viewSpace.Put()));
            Enable();
        }

        ~PlaneFindingScene() override {
            // Stop the worker thread first before destroying this class
            if (m_future.valid()) {
                m_future.get();
            }
            if (m_serializeFuture.valid()) {
                m_serializeFuture.get();
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            m_lastTimeOfUpdate = frameTime.PredictedDisplayTime;
            // Check if the background thread finished creating a new group of scene objects.
            if (m_scanState == ScanState::Processing && m_future.valid() && m_future.wait_for(0s) == std::future_status::ready) {
                for (const std::shared_ptr<engine::Object>& object : m_sceneVisuals.visuals) {
                    RemoveObject(object);
                }
                m_sceneVisuals = m_future.get();
                for (const std::shared_ptr<engine::Object>& object : m_sceneVisuals.visuals) {
                    AddObject(object);
                    object->SetVisible(false);
                }
                m_scanState = ScanState::Idle;
                m_nextUpdate = frameTime.Now + UpdateInterval;
            }

            if (m_sceneVisuals.scene) {
                XrSceneObjectsLocateInfoMSFT locateInfo{XR_TYPE_SCENE_OBJECTS_LOCATE_INFO_MSFT};
                locateInfo.baseSpace = m_context.AppSpace;
                locateInfo.time = frameTime.PredictedDisplayTime;
                locateInfo.sceneObjectCount = static_cast<uint32_t>(m_sceneVisuals.keys.size());
                locateInfo.sceneObjectKeys = m_sceneVisuals.keys.data();

                m_sceneObjectLocations.resize(m_sceneVisuals.keys.size());
                XrSceneObjectLocationsMSFT locations{XR_TYPE_SCENE_OBJECT_LOCATIONS_MSFT};
                locations.sceneObjectCount = static_cast<uint32_t>(m_sceneObjectLocations.size());
                locations.sceneObjectLocations = m_sceneObjectLocations.data();

                CHECK_XRCMD(m_extensions.xrLocateSceneObjectsMSFT(m_sceneVisuals.scene.Get(), &locateInfo, &locations));
                for (size_t i = 0; i < m_sceneVisuals.keys.size(); ++i) {
                    const XrSceneObjectLocationMSFT& location = m_sceneObjectLocations[i];
                    const auto& object = m_sceneVisuals.visuals[i];
                    if (xr::math::Pose::IsPoseValid(location.locationFlags)) {
                        object->Pose() = location.pose;
                        object->SetVisible(true);
                    } else {
                        object->SetVisible(false);
                    }
                }
            }

            XrSpaceLocation viewInLocal{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInLocal));
            if (xr::math::Pose::IsPoseValid(viewInLocal)) {
                m_sphere.center = viewInLocal.pose.position;
                m_sphere.radius = ScanRadius;
            }

            if (m_readSceneTask != nullptr) {
                if (m_readSceneTask.Status() == wrt::AsyncStatus::Completed) {
                    wrt::IBuffer buffer = m_readSceneTask.get();
                    if (buffer != nullptr) {
                        winrt::com_array<uint8_t> data;
                        wrt::CryptographicBuffer::CopyToByteArray(buffer, data);

                        // Deserialize
                        XrDeserializeSceneFragmentMSFT fragment{static_cast<uint32_t>(data.size()), data.data()};
                        XrDeserializeSceneInfoMSFT deserializeInfo{XR_TYPE_DESERIALIZE_SCENE_INFO_MSFT};
                        deserializeInfo.fragmentCount = 1;
                        deserializeInfo.fragments = &fragment;
                        CHECK_XRCMD(m_extensions.xrDeserializeSceneMSFT(m_sceneObserver.Get(), &deserializeInfo));

                        m_scanState = ScanState::Waiting;
                    }
                }
                if (m_readSceneTask.Status() != wrt::AsyncStatus::Started) {
                    m_readSceneTask = nullptr;
                }
            } else if (m_scanState == ScanState::Waiting) {
                // Check if the results are available
                XrSceneComputeStateMSFT state{};
                CHECK_XRCMD(m_extensions.xrGetSceneComputeStateMSFT(m_sceneObserver.Get(), &state));
                if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
                    xr::SceneHandle scene = xr::CreateScene(m_extensions, m_sceneObserver.Get());
                    // Send the scene compute result to the background thread for processing
                    m_future = std::async(std::launch::async,
                                          &CreateSceneVisuals,
                                          std::cref(m_extensions),
                                          std::cref(m_context.PbrResources),
                                          m_material,
                                          std::move(scene));
                    m_scanState = ScanState::Processing;
                }
            } else if (m_scanState == ScanState::Idle) {
                // no active query, start one if enough time has passed
                if (frameTime.Now > m_nextUpdate) {
                    XrNewSceneComputeInfoMSFT computeInfo{XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT};

                    computeInfo.bounds.space = m_context.AppSpace;
                    computeInfo.bounds.time = m_lastTimeOfUpdate;
                    computeInfo.bounds.sphereCount = 1;
                    computeInfo.bounds.spheres = &m_sphere;

                    // Start the async query
                    CHECK_XRCMD(m_extensions.xrComputeNewSceneMSFT(m_sceneObserver.Get(), &computeInfo));

                    m_nextUpdate = frameTime.Now + UpdateInterval;
                    m_scanState = ScanState::Waiting;
                }
            }
        }

        void OnActiveChanged() override {
            if (IsActive()) {
                Enable();
            } else {
                Disable();
            }
        }

    private:
        enum class ScanState { Idle, Waiting, Processing };

        void Enable() {
            m_sceneObserver = xr::CreateSceneObserver(m_extensions, m_context.Session.Handle);
            m_scanState = ScanState::Idle;
            m_readSceneTask = ReadBufferAsync(SceneFileName);
        }

        void Disable() {
            for (const std::shared_ptr<engine::Object>& object : m_sceneVisuals.visuals) {
                RemoveObject(object);
            }
            m_sceneVisuals = {};

            // Stop the worker thread before clearing sceneObserver because the thread has access to it.
            if (m_future.valid()) {
                m_future.get();
            }
            if (m_sceneObserver) {
                m_serializeFuture = std::async(std::launch::async,
                                               &SerializeScene,
                                               std::cref(m_extensions),
                                               std::move(m_sceneObserver),
                                               m_context.AppSpace,
                                               m_lastTimeOfUpdate,
                                               std::cref(m_sphere));
            }
        }

        std::shared_ptr<Pbr::Material> CreateTextureMaterial() {
            ID3D11Device* const device = m_context.PbrResources.GetDevice().get();
            auto material = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::FromSRGB(DirectX::Colors::White), 1.0f, 0.0f);
            const std::vector<uint32_t> rgba = CreateTileTextureBytes();
            const uint32_t ByteArraySize = static_cast<uint32_t>(rgba.size() * sizeof(uint32_t));
            auto tileTexture = Pbr::Texture::CreateTexture(device,
                                                           reinterpret_cast<const uint8_t*>(rgba.data()),
                                                           ByteArraySize,
                                                           TextureSideLength,
                                                           TextureSideLength,
                                                           DXGI_FORMAT_R8G8B8A8_UNORM);
            const winrt::com_ptr<ID3D11SamplerState> sampler = Pbr::Texture::CreateSampler(device, D3D11_TEXTURE_ADDRESS_WRAP);
            material->SetTexture(Pbr::ShaderSlots::BaseColor, tileTexture.get(), sampler.get());
            return material;
        }

        const xr::ExtensionContext& m_extensions;
        const std::shared_ptr<Pbr::Material> m_material;
        SceneVisuals m_sceneVisuals;
        xr::SceneObserverHandle m_sceneObserver;
        xr::SpaceHandle m_viewSpace;
        XrTime m_lastTimeOfUpdate{};
        XrSceneSphereBoundMSFT m_sphere{};
        std::vector<XrSceneObjectLocationMSFT> m_sceneObjectLocations;
        std::future<SceneVisuals> m_future;
        std::future<void> m_serializeFuture;
        TimePoint m_nextUpdate{};
        ScanState m_scanState{ScanState::Idle};
        wrt::IAsyncOperation<wrt::IBuffer> m_readSceneTask{nullptr};
    };

    Pbr::RGBAColor GetColor(XrSceneObjectKindTypeMSFT kind) {
        using namespace DirectX;
        // The lighting system makes a lot of the colors too bright so multiply them to tone them down
        constexpr auto scaleColor = [](const XMVECTORF32& color, float scale) {
            return Pbr::FromSRGB(color * XMVECTORF32{scale, scale, scale, 1});
        };
        switch (kind) {
        case XR_SCENE_OBJECT_KIND_TYPE_CEILING_MSFT:
            return Pbr::FromSRGB(Colors::Green);
        case XR_SCENE_OBJECT_KIND_TYPE_FLOOR_MSFT:
            return scaleColor(Colors::Blue, 0.5f);
        case XR_SCENE_OBJECT_KIND_TYPE_PLATFORM_MSFT:
            return scaleColor(Colors::Orange, 0.6f);
        case XR_SCENE_OBJECT_KIND_TYPE_WALL_MSFT:
            return scaleColor(Colors::Tomato, 0.5f);
        case XR_SCENE_OBJECT_KIND_TYPE_BACKGROUND_MSFT:
            return scaleColor(Colors::Cyan, 0.8f);
        case XR_SCENE_OBJECT_KIND_TYPE_UNKNOWN_MSFT:
            return scaleColor(Colors::Purple, 0.8f);
        default:
            return Pbr::FromSRGB(Colors::White);
        }
    }

    bool ShouldRender(XrSceneObjectKindTypeMSFT kind) {
        switch (kind) {
        case XR_SCENE_OBJECT_KIND_TYPE_CEILING_MSFT:
        case XR_SCENE_OBJECT_KIND_TYPE_FLOOR_MSFT:
        case XR_SCENE_OBJECT_KIND_TYPE_PLATFORM_MSFT:
        case XR_SCENE_OBJECT_KIND_TYPE_WALL_MSFT:
        case XR_SCENE_OBJECT_KIND_TYPE_BACKGROUND_MSFT:
            return true;
        default:
            return false;
        }
    }

    void FillMeshPrimitiveBuilder(const std::vector<XrVector3f>& positions,
                                  const std::vector<uint32_t>& indices,
                                  const Pbr::RGBAColor& color,
                                  Pbr::PrimitiveBuilder& builder) {
        using namespace DirectX;
        const size_t indexCount = indices.size();
        builder.Vertices.clear();
        builder.Indices.clear();
        builder.Vertices.reserve(indexCount);
        builder.Indices.reserve(indexCount);

        auto appendVertex = [&builder](const XMVECTOR& pos, Pbr::Vertex& vertex) {
            XMStoreFloat3(&vertex.Position, pos);
            builder.Indices.push_back(static_cast<uint32_t>(builder.Vertices.size()));
            builder.Vertices.push_back(vertex);
        };

        // Create 3 vertices per triangle where the normal is perpendicular to the surface
        // in order to make the triangle edges sharper.
        for (size_t index = 2; index < indices.size(); index += 3) {
            auto v0 = xr::math::LoadXrVector3(positions[indices[index - 2]]);
            auto v1 = xr::math::LoadXrVector3(positions[indices[index - 1]]);
            auto v2 = xr::math::LoadXrVector3(positions[indices[index]]);

            Pbr::Vertex vertex{};
            vertex.Color0 = color;
            XMStoreFloat4(&vertex.Tangent, XMVectorSetW(XMVector3Normalize(v1 - v0), 1.0f));
            // CCW winding order
            XMStoreFloat3(&vertex.Normal, XMVector3Normalize(XMVector3Cross(v1 - v0, v2 - v0)));
            appendVertex(v0, vertex);
            appendVertex(v2, vertex);
            appendVertex(v1, vertex);
        }
    }

    void ComputeNewScene(const xr::ExtensionContext& extensions,
                         XrSceneObserverMSFT sceneObserver,
                         XrSpace space,
                         XrTime time,
                         const XrSceneSphereBoundMSFT& sphere) {
    }

    wrt::IAsyncOperation<wrt::IBuffer> ReadBufferAsync(winrt::hstring filename) {
        wrt::StorageFolder storageFolder = wrt::ApplicationData::Current().LocalFolder();
        // People typically have the debugger setup to break when winrt::hresult_error is thrown so to avoid people thinking
        // there is an error this function uses TryGetItemAsync to check for null if the file doesn't exist.
        wrt::IStorageItem item = co_await storageFolder.TryGetItemAsync(filename);
        if (item != nullptr) {
            wrt::StorageFile file = item.try_as<wrt::StorageFile>();
            if (file != nullptr) {
                co_return co_await wrt::FileIO::ReadBufferAsync(file);
            }
        }
        return nullptr;
    }

    std::vector<uint32_t> CreateTileTextureBytes() {
        constexpr size_t Size = TextureSideLength;
        constexpr uint32_t White = 0xFFFFFFFF;
        constexpr uint32_t Black = 0x000000FF;
        constexpr size_t Last = Size - 1;
        std::vector<uint32_t> rgba(Size * Size, White);
        // make the border black
        for (size_t col = 0; col < Size; ++col) {
            rgba[col] = Black;
            rgba[Last + col] = Black;
        }
        for (size_t row = 0; row < Size * Size; row += Size) {
            rgba[row] = Black;
            rgba[row + Last] = Black;
        }
        return rgba;
    }

    XrSceneObjectKindTypeMSFT GetSceneObjectKind(const xr::ExtensionContext& extensions,
                                                 XrSceneMSFT scene,
                                                 XrSceneObjectKeyMSFT sceneObjectKey) {
        XrSceneObjectPropertiesMSFT properties{XR_TYPE_SCENE_OBJECT_PROPERTIES_MSFT};
        XrSceneObjectKindMSFT kind{XR_TYPE_SCENE_OBJECT_KIND_MSFT};
        properties.next = &kind;

        XrSceneObjectPropertiesGetInfoMSFT getInfo{XR_TYPE_SCENE_OBJECT_PROPERTIES_GET_INFO_MSFT};
        getInfo.sceneObjectKey = sceneObjectKey;
        CHECK_XRCMD(extensions.xrGetSceneObjectPropertiesMSFT(scene, &getInfo, &properties));
        return kind.kind;
    }

    void AddMeshPrimitive(const xr::ExtensionContext& extensions,
                          const Pbr::Resources& pbrResources,
                          const std::shared_ptr<Pbr::Material>& material,
                          XrSceneMSFT scene,
                          XrSceneObjectKindTypeMSFT kind,
                          XrSceneMeshKeyMSFT meshKey,
                          Pbr::PrimitiveBuilder& builder,
                          std::shared_ptr<Pbr::Model>& model) {
        xr::SceneMesh mesh = GetSceneMesh(extensions, scene, meshKey);
        if (mesh.indices.empty() || mesh.positions.empty()) {
            return;
        }

        FillMeshPrimitiveBuilder(mesh.positions, mesh.indices, GetColor(kind), builder);
        model->AddPrimitive(Pbr::Primitive(pbrResources, builder, material));
    }

    void AddPlanePrimitive(const xr::ExtensionContext& extensions,
                           const Pbr::Resources& pbrResources,
                           const std::shared_ptr<Pbr::Material>& material,
                           XrSceneMSFT scene,
                           XrScenePlaneKeyMSFT planeKey,
                           XrSceneObjectKindTypeMSFT kind,
                           std::shared_ptr<Pbr::Model>& model) {
        XrScenePlanePropertiesGetInfoMSFT planeGetInfo{XR_TYPE_SCENE_PLANE_PROPERTIES_GET_INFO_MSFT};
        planeGetInfo.scenePlaneKey = planeKey;
        XrScenePlanePropertiesMSFT planeProperties{XR_TYPE_SCENE_PLANE_PROPERTIES_MSFT};
        CHECK_XRCMD(extensions.xrGetScenePlanePropertiesMSFT(scene, &planeGetInfo, &planeProperties));

        const DirectX::XMFLOAT2 extents{planeProperties.extents.width, planeProperties.extents.height};
        constexpr float TextureScale = 5.0f;
        const DirectX::XMFLOAT2 textureCoord{extents.x * TextureScale, extents.y * TextureScale};
        model->AddPrimitive(Pbr::Primitive(
            pbrResources, Pbr::PrimitiveBuilder().AddQuad(extents, textureCoord, Pbr::RootNodeIndex, GetColor(kind)), material));
    }

    SceneVisuals CreateSceneVisuals(const xr::ExtensionContext& extensions,
                                    const Pbr::Resources& pbrResources,
                                    const std::shared_ptr<Pbr::Material>& material,
                                    xr::SceneHandle scene) {
        std::vector<std::shared_ptr<engine::Object>> visuals;
        std::vector<XrSceneObjectKeyMSFT> keys;
        Pbr::PrimitiveBuilder builder;
        for (const XrSceneObjectMSFT& xrSceneObject : xr::GetSceneObjects(extensions, scene.Get())) {
            const XrSceneObjectKeyMSFT sceneObjectKey = xrSceneObject.sceneObjectKey;
            const XrSceneObjectKindTypeMSFT kind = GetSceneObjectKind(extensions, scene.Get(), sceneObjectKey);
            if (!ShouldRender(kind)) {
                continue;
            }
            auto model = std::make_shared<Pbr::Model>();
            // Build primitives from the plane data
            for (const XrScenePlaneKeyMSFT planeKey : xr::GetPlaneKeys(extensions, scene.Get(), sceneObjectKey)) {
                AddPlanePrimitive(extensions, pbrResources, material, scene.Get(), planeKey, kind, model);
            }
            if (model->GetPrimitiveCount() > 0) {
                auto object = std::make_shared<engine::PbrModelObject>(std::move(model));
                visuals.push_back(std::move(object));
                keys.push_back(sceneObjectKey);
            }
        }
        return SceneVisuals(std::move(visuals), std::move(keys), std::move(scene));
    }

    void SerializeScene(const xr::ExtensionContext& extensions,
                        xr::SceneObserverHandle&& sceneObserver,
                        XrSpace space,
                        XrTime time,
                        const XrSceneSphereBoundMSFT& sphere) {
        assert(sceneObserver);
        XrSceneComputeStateMSFT state;
        do {
            // wait for the current query to finish
            std::this_thread::sleep_for(50ms);
            CHECK_XRCMD(extensions.xrGetSceneComputeStateMSFT(sceneObserver.Get(), &state));
        } while (state == XR_SCENE_COMPUTE_STATE_UPDATING_MSFT);

        XrNewSceneComputeInfoMSFT computeInfo{XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT};

        // Chaining in XrSerializeSceneMSFT causes xrComputeNewSceneMSFT to serialize the scene.
        XrSerializeSceneMSFT serializeScene{XR_TYPE_SERIALIZE_SCENE_MSFT};
        xr::InsertExtensionStruct(computeInfo, serializeScene);

        computeInfo.bounds.space = space;
        computeInfo.bounds.time = time;
        computeInfo.bounds.sphereCount = 1;
        computeInfo.bounds.spheres = &sphere;

        // Start the async query
        CHECK_XRCMD(extensions.xrComputeNewSceneMSFT(sceneObserver.Get(), &computeInfo));

        for (;;) {
            std::this_thread::sleep_for(250ms);
            CHECK_XRCMD(extensions.xrGetSceneComputeStateMSFT(sceneObserver.Get(), &state));
            if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
                wrt::StorageFolder storageFolder{wrt::ApplicationData::Current().LocalFolder()};
                wrt::StorageFile storageFile =
                    storageFolder.CreateFileAsync(SceneFileName, wrt::CreationCollisionOption::ReplaceExisting).get();
                std::basic_ofstream<uint8_t> outfile;
                outfile.exceptions(std::ios::failbit | std::ios::badbit);
                outfile.open(storageFile.Path().c_str(), std::ofstream::binary);
                xr::SceneHandle scene = xr::CreateScene(extensions, sceneObserver.Get());
                xr::ReadSerializedScene(extensions, scene.Get(), outfile);
                outfile.close();
                break;
            }
        }
    }

} // namespace

std::unique_ptr<engine::Scene> TryCreatePlaneFindingScene(engine::Context& context) {
    return context.Extensions.SupportsSceneUnderstanding ? std::make_unique<PlaneFindingScene>(context) : nullptr;
}
