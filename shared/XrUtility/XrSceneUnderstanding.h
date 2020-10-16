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
#pragma once

#include "XrStruct.h"
#include "XrHandle.h"
#include "XrMath.h"
#include "XrExtensionContext.h"

namespace xr {
    class SceneObserverHandle : public xr::UniqueExtHandle<XrSceneObserverMSFT> {};
    class SceneHandle : public xr::UniqueExtHandle<XrSceneMSFT> {};

    struct SceneMesh {
        std::vector<XrVector3f> positions;
        std::vector<uint32_t> indices;
    };

    inline xr::SceneObserverHandle CreateSceneObserver(const xr::ExtensionContext& extensions, XrSession session) {
        xr::SceneObserverHandle sceneObserverHandle;
        XrSceneObserverCreateInfoMSFT createInfo{XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT};
        CHECK_XRCMD(
            extensions.xrCreateSceneObserverMSFT(session, &createInfo, sceneObserverHandle.Put(extensions.xrDestroySceneObserverMSFT)));
        return sceneObserverHandle;
    }

    inline xr::SceneHandle CreateScene(const xr::ExtensionContext& extensions, XrSceneObserverMSFT sceneObserver) {
        xr::SceneHandle sceneHandle;
        XrSceneCreateInfoMSFT createInfo{XR_TYPE_SCENE_CREATE_INFO_MSFT};
        CHECK_XRCMD(extensions.xrCreateSceneMSFT(sceneObserver, &createInfo, sceneHandle.Put(extensions.xrDestroySceneMSFT)));
        return sceneHandle;
    }

    inline std::vector<XrSceneObjectMSFT> GetSceneObjects(const xr::ExtensionContext& extensions, XrSceneMSFT scene) {
        // Get list of scene object atoms using 2-call
        uint32_t sceneObjectCountOutput = 0;
        CHECK_XRCMD(extensions.xrGetSceneObjectsMSFT(scene, 0, &sceneObjectCountOutput, nullptr));

        std::vector<XrSceneObjectMSFT> sceneObjectVector(sceneObjectCountOutput, {XR_TYPE_SCENE_OBJECT_MSFT});
        CHECK_XRCMD(extensions.xrGetSceneObjectsMSFT(
            scene, static_cast<uint32_t>(sceneObjectVector.size()), &sceneObjectCountOutput, sceneObjectVector.data()));
        sceneObjectVector.resize(sceneObjectCountOutput);
        return sceneObjectVector;
    }

    inline std::vector<XrSceneMeshKeyMSFT> GetMeshKeys(const xr::ExtensionContext& extensions,
                                                       XrSceneMSFT scene,
                                                       XrSceneObjectKeyMSFT sceneObjectKey) {
        XrSceneObjectPropertiesGetInfoMSFT getInfo{XR_TYPE_SCENE_OBJECT_PROPERTIES_GET_INFO_MSFT};
        getInfo.sceneObjectKey = sceneObjectKey;
        XrSceneObjectPropertiesMSFT properties{XR_TYPE_SCENE_OBJECT_PROPERTIES_MSFT};
        XrSceneMeshKeysMSFT meshKeys{XR_TYPE_SCENE_MESH_KEYS_MSFT};
        xr::InsertExtensionStruct(properties, meshKeys);
        CHECK_XRCMD(extensions.xrGetSceneObjectPropertiesMSFT(scene, &getInfo, &properties));
        std::vector<XrSceneMeshKeyMSFT> meshKeysVector(meshKeys.meshKeyCountOutput);
        if (meshKeys.meshKeyCountOutput > 0) {
            meshKeys.meshKeyCapacityInput = meshKeys.meshKeyCountOutput;
            meshKeys.meshKeys = meshKeysVector.data();
            CHECK_XRCMD(extensions.xrGetSceneObjectPropertiesMSFT(scene, &getInfo, &properties));
            meshKeysVector.resize(meshKeys.meshKeyCountOutput);
        }
        return meshKeysVector;
    }

    inline std::vector<XrScenePlaneKeyMSFT> GetPlaneKeys(const xr::ExtensionContext& extensions,
                                                         XrSceneMSFT scene,
                                                         XrSceneObjectKeyMSFT sceneObjectKey) {
        XrSceneObjectPropertiesGetInfoMSFT getInfo{XR_TYPE_SCENE_OBJECT_PROPERTIES_GET_INFO_MSFT};
        getInfo.sceneObjectKey = sceneObjectKey;
        XrSceneObjectPropertiesMSFT properties{XR_TYPE_SCENE_OBJECT_PROPERTIES_MSFT};
        XrScenePlaneKeysMSFT planeKeys{XR_TYPE_SCENE_PLANE_KEYS_MSFT};
        xr::InsertExtensionStruct(properties, planeKeys);
        CHECK_XRCMD(extensions.xrGetSceneObjectPropertiesMSFT(scene, &getInfo, &properties));
        std::vector<XrScenePlaneKeyMSFT> planeKeysVector(planeKeys.planeKeyCountOutput);
        if (planeKeys.planeKeyCountOutput > 0) {
            planeKeys.planeKeyCapacityInput = planeKeys.planeKeyCountOutput;
            planeKeys.planeKeys = planeKeysVector.data();
            CHECK_XRCMD(extensions.xrGetSceneObjectPropertiesMSFT(scene, &getInfo, &properties));
            planeKeysVector.resize(planeKeys.planeKeyCountOutput);
        }
        return planeKeysVector;
    }

    inline xr::SceneMesh GetSceneMesh(const xr::ExtensionContext& extensions, XrSceneMSFT scene, XrSceneMeshKeyMSFT meshKey) {
        XrSceneMeshGetInfoMSFT meshGetInfo{XR_TYPE_SCENE_MESH_GET_INFO_MSFT};
        meshGetInfo.sceneMeshKey = meshKey;

        XrSceneMeshMSFT mesh{XR_TYPE_SCENE_MESH_MSFT};
        CHECK_XRCMD(extensions.xrGetSceneMeshMSFT(scene, &meshGetInfo, &mesh));

        xr::SceneMesh meshData;
        meshData.positions.resize(mesh.vertexCountOutput);
        meshData.indices.resize(mesh.indexCountOutput);
        mesh.vertexCapacityInput = static_cast<uint32_t>(meshData.positions.size());
        mesh.indexCapacityInput = static_cast<uint32_t>(meshData.indices.size());
        mesh.vertices = meshData.positions.data();
        mesh.indices = meshData.indices.data();
        CHECK_XRCMD(extensions.xrGetSceneMeshMSFT(scene, &meshGetInfo, &mesh));
        meshData.positions.resize(mesh.vertexCountOutput);
        meshData.indices.resize(mesh.indexCountOutput);
        return meshData;
    }

    inline void ReadSerializedScene(const xr::ExtensionContext& extensions, XrSceneMSFT scene, std::basic_ostream<uint8_t>& stream) {
        constexpr uint32_t BufferSize = 8192;
        std::array<uint8_t, BufferSize> buffer;
        uint32_t readOutput = 0;
        do {
            // xrGetSceneSerializedDataMSFT does not use 2-call idiom. It behaves more like fread where readOutput will output
            // how many bytes were read. The function should be called until readOutput outputs zero.
            CHECK_XRCMD(extensions.xrGetSceneSerializedDataMSFT(scene, BufferSize, &readOutput, buffer.data()));
            stream.write(buffer.data(), readOutput);
        } while (readOutput > 0);
    }

} // namespace xr
