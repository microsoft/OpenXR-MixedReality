// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>

using namespace DirectX;
using namespace xr::math;

namespace {

    //
    // This sample displays hand tracking inputs appears as hand mesh, or joint axes.
    // User can clap their hands to toggle between different display mode
    // It also demos a simple procedural coloring of the hand mesh using the "open palm" reference hand input.
    //
    struct HandTrackingScene : public engine::Scene {
        HandTrackingScene(engine::Context& context, bool enableHandMesh)
            : Scene(context)
            , m_enableHandMesh(enableHandMesh) {
            const std::tuple<XrHandEXT, HandData&> hands[] = {{XrHandEXT::XR_HAND_LEFT_EXT, m_leftHandData},
                                                              {XrHandEXT::XR_HAND_RIGHT_EXT, m_rightHandData}};

            m_jointMaterial = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::RGBA::White, 0.85f, 0.01f);

            auto createJointObjects = [&](HandData& handData) {
                auto jointModel = std::make_shared<Pbr::Model>();
                Pbr::PrimitiveBuilder primitiveBuilder;

                // Create a axis object attached to each joint location
                for (uint32_t k = 0; k < std::size(handData.PbrNodeIndices); k++) {
                    handData.PbrNodeIndices[k] = jointModel->AddNode(DirectX::XMMatrixIdentity(), Pbr::RootNodeIndex, "joint");
                    primitiveBuilder.AddAxis(1.0f, 0.5f, handData.PbrNodeIndices[k]);
                }

                // Now that the axis have been added for each joint into the primitive builder,
                // it can be baked into the model as a single primitive.
                jointModel->AddPrimitive(Pbr::Primitive(m_context.PbrResources, primitiveBuilder, m_jointMaterial));
                handData.JointModel = AddObject(std::make_shared<engine::PbrModelObject>(std::move(jointModel)));
                handData.JointModel->SetVisible(false);
            };

            // For each hand, initialize the joint objects and corresponding space.
            for (const auto& [hand, handData] : hands) {
                XrHandTrackerCreateInfoEXT createInfo{XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT};
                createInfo.hand = hand;
                createInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
                CHECK_XRCMD(m_context.Extensions.xrCreateHandTrackerEXT(
                    m_context.Session.Handle, &createInfo, handData.TrackerHandle.Put(m_context.Extensions.xrDestroyHandTrackerEXT)));

                createJointObjects(handData);
            }

            if (enableHandMesh) {
                m_mode = HandDisplayMode::Mesh;
                m_meshMaterial = Pbr::Material::CreateFlat(m_context.PbrResources, Pbr::RGBA::White, 1, 0);

                // For each hand, initialize hand mesh buffer and corresponding space.
                for (const auto& [hand, handData] : hands) {
                    // Initialize buffers to receive hand mesh indices and vertices
                    const XrSystemHandTrackingMeshPropertiesMSFT& handMeshSystemProperties = context.System.HandMeshProperties;
                    handData.IndexBuffer = std::make_unique<uint32_t[]>(handMeshSystemProperties.maxHandMeshIndexCount);
                    handData.VertexBuffer = std::make_unique<XrHandMeshVertexMSFT[]>(handMeshSystemProperties.maxHandMeshVertexCount);

                    handData.meshState.indexBuffer.indexCapacityInput = handMeshSystemProperties.maxHandMeshIndexCount;
                    handData.meshState.indexBuffer.indices = handData.IndexBuffer.get();
                    handData.meshState.vertexBuffer.vertexCapacityInput = handMeshSystemProperties.maxHandMeshVertexCount;
                    handData.meshState.vertexBuffer.vertices = handData.VertexBuffer.get();

                    XrHandMeshSpaceCreateInfoMSFT meshSpaceCreateInfo{XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT};
                    meshSpaceCreateInfo.poseInHandMeshSpace = xr::math::Pose::Identity();
                    meshSpaceCreateInfo.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
                    CHECK_XRCMD(m_context.Extensions.xrCreateHandMeshSpaceMSFT(
                        handData.TrackerHandle.Get(), &meshSpaceCreateInfo, handData.MeshSpace.Put()));

                    meshSpaceCreateInfo.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;
                    CHECK_XRCMD(m_context.Extensions.xrCreateHandMeshSpaceMSFT(
                        handData.TrackerHandle.Get(), &meshSpaceCreateInfo, handData.ReferenceMeshSpace.Put()));
                }
            }

            // Set a clap detector that will toggle the display mode.
            m_clapDetector = std::make_unique<StateChangeDetector>(
                [this](XrTime time) {
                    const XrHandJointLocationEXT& leftPalmLocation = m_leftHandData.JointLocations[XR_HAND_JOINT_PALM_EXT];
                    const XrHandJointLocationEXT& rightPalmLocation = m_rightHandData.JointLocations[XR_HAND_JOINT_PALM_EXT];

                    if (xr::math::Pose::IsPoseValid(leftPalmLocation) && xr::math::Pose::IsPoseValid(rightPalmLocation)) {
                        const XMVECTOR leftPalmPosition = xr::math::LoadXrVector3(leftPalmLocation.pose.position);
                        const XMVECTOR rightPalmPosition = xr::math::LoadXrVector3(rightPalmLocation.pose.position);
                        const float distance = XMVectorGetX(XMVector3Length(XMVectorSubtract(leftPalmPosition, rightPalmPosition)));
                        return distance - leftPalmLocation.radius - rightPalmLocation.radius < 0.02f /*meter*/;
                    }

                    return false;
                },
                [this]() {
                    // We can only change mode if Mesh is supported
                    if (m_enableHandMesh) {
                        m_mode = (HandDisplayMode)(((uint32_t)m_mode + 1) % (uint32_t)HandDisplayMode::Count);
                    }
                });
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            for (HandData& handData : {std::ref(m_leftHandData), std::ref(m_rightHandData)}) {
                XrHandJointsLocateInfoEXT locateInfo{XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT};
                locateInfo.baseSpace = m_context.AppSpace;
                locateInfo.time = frameTime.PredictedDisplayTime;

                XrHandJointLocationsEXT locations{XR_TYPE_HAND_JOINT_LOCATIONS_EXT};
                locations.jointCount = (uint32_t)handData.JointLocations.size();
                locations.jointLocations = handData.JointLocations.data();
                CHECK_XRCMD(m_context.Extensions.xrLocateHandJointsEXT(handData.TrackerHandle.Get(), &locateInfo, &locations));

                bool jointsVisible = m_mode == HandDisplayMode::Joints;
                bool meshVisible = m_mode == HandDisplayMode::Mesh;

                if (jointsVisible) {
                    jointsVisible = UpdateJoints(handData, m_context.AppSpace, frameTime.PredictedDisplayTime);
                }

                if (meshVisible) {
                    meshVisible = UpdateMesh(handData, m_context.AppSpace, frameTime.PredictedDisplayTime);
                }

                handData.JointModel->SetVisible(jointsVisible);
                if (handData.MeshObject != nullptr) { // Pbr mesh object creation is deferred.
                    handData.MeshObject->SetVisible(meshVisible);
                }
            }

            // Detect hand clap to toggle hand display mode.
            m_clapDetector->Update(frameTime.PredictedDisplayTime);
        }

        struct HandData {
            xr::HandTrackerHandle TrackerHandle;

            // Data to display hand joints tracking
            std::shared_ptr<engine::PbrModelObject> JointModel;
            std::array<Pbr::NodeIndex_t, XR_HAND_JOINT_COUNT_EXT> PbrNodeIndices{};
            std::array<XrHandJointLocationEXT, XR_HAND_JOINT_COUNT_EXT> JointLocations{};

            // Data to display hand mesh tracking
            xr::SpaceHandle MeshSpace;
            xr::SpaceHandle ReferenceMeshSpace;
            std::vector<XMFLOAT4> VertexColors;
            std::shared_ptr<engine::PbrModelObject> MeshObject;

            // Data to process open-palm reference hand.
            XrHandMeshMSFT meshState{XR_TYPE_HAND_MESH_MSFT};
            std::unique_ptr<uint32_t[]> IndexBuffer{};
            std::unique_ptr<XrHandMeshVertexMSFT[]> VertexBuffer{};

            HandData() = default;
            HandData(HandData&&) = delete;
            HandData(const HandData&) = delete;
        };

        bool UpdateJoints(HandData& handData, XrSpace referenceSpace, XrTime time) {
            bool jointsVisible = false;

            for (uint32_t k = 0; k < XR_HAND_JOINT_COUNT_EXT; k++) {
                if (xr::math::Pose::IsPoseValid(handData.JointLocations[k])) {
                    Pbr::Node& jointNode = handData.JointModel->GetModel()->GetNode(handData.PbrNodeIndices[k]);

                    const float radius = handData.JointLocations[k].radius;
                    jointNode.SetTransform(XMMatrixScaling(radius, radius, radius) * xr::math::LoadXrPose(handData.JointLocations[k].pose));

                    jointsVisible = true;
                }
            }

            return jointsVisible;
        }

        bool UpdateMesh(HandData& handData, XrSpace referenceSpace, XrTime time) {
            XrHandMeshUpdateInfoMSFT meshUpdateInfo{XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT};
            meshUpdateInfo.time = time;
            meshUpdateInfo.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
            CHECK_XRCMD(m_context.Extensions.xrUpdateHandMeshMSFT(handData.TrackerHandle.Get(), &meshUpdateInfo, &handData.meshState));

            if (!handData.meshState.isActive) {
                return false;
            }

            bool indicesChanged = handData.meshState.indexBufferChanged;
            bool verticesChanged = handData.meshState.vertexBufferChanged;

            if (indicesChanged) {
                // Index buffer is changed, recalculate vertices color based on neutral hand pose.
                ComputeHandMeshColor(handData, time);
            }

            if (verticesChanged) {
                Pbr::PrimitiveBuilder meshBuilder = CreateHandMeshPrimitiveBuilder(handData.meshState.indexBuffer.indices,
                                                                                   handData.meshState.indexBuffer.indexCountOutput,
                                                                                   handData.meshState.vertexBuffer.vertices,
                                                                                   handData.meshState.vertexBuffer.vertexCountOutput,
                                                                                   handData.VertexColors);

                if (!handData.MeshObject) {
                    // The hand mesh scene object doesn't exist yet and must be created.
                    Pbr::Primitive surfacePrimitive(m_context.PbrResources, meshBuilder, m_meshMaterial, false /* updatableBuffers */);

                    auto surfaceModel = std::make_shared<Pbr::Model>();
                    surfaceModel->AddPrimitive(std::move(surfacePrimitive));

                    handData.MeshObject = AddObject(std::make_shared<engine::PbrModelObject>(surfaceModel));
                } else {
                    // Update vertices and indices of the existing hand mesh scene object's primitive.
                    handData.MeshObject->GetModel()->GetPrimitive(0).UpdateBuffers(
                        m_context.Device.get(), m_context.DeviceContext.get(), meshBuilder);
                }
            }

            if (handData.MeshObject) {
                XrSpaceLocation meshLocation{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(handData.MeshSpace.Get(), referenceSpace, time, &meshLocation));
                if (xr::math::Pose::IsPoseValid(meshLocation)) {
                    handData.MeshObject->Pose() = meshLocation.pose;
                    return true;
                }
            }

            return false;
        }

        void ComputeHandMeshColor(HandData& handData, XrTime time) {
            // Compute a color for each vertex on hand mesh based on relative position to an open palm reference hand.
            // Use the middle finger tip and wrist joints to normalize the vertical range.
            // Use the little finger tip and thumb tip joints to normalize the horizonal range.

            XrHandPoseTypeInfoMSFT poseTypeInfo{XR_TYPE_HAND_POSE_TYPE_INFO_MSFT};
            poseTypeInfo.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;

            XrHandJointsLocateInfoEXT locateInfo{XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT, &poseTypeInfo};
            locateInfo.baseSpace = handData.ReferenceMeshSpace.Get();
            locateInfo.time = time;

            XrHandJointLocationsEXT locations{XR_TYPE_HAND_JOINT_LOCATIONS_EXT};
            locations.jointCount = (uint32_t)handData.JointLocations.size();
            locations.jointLocations = handData.JointLocations.data();

            CHECK_XRCMD(m_context.Extensions.xrLocateHandJointsEXT(handData.TrackerHandle.Get(), &locateInfo, &locations));
            assert(locations.isActive);

            const XrVector3f& vZero = handData.JointLocations[XR_HAND_JOINT_MIDDLE_TIP_EXT].pose.position;
            const XrVector3f& vOne = handData.JointLocations[XR_HAND_JOINT_WRIST_EXT].pose.position;
            const XrVector3f& hZero = handData.JointLocations[XR_HAND_JOINT_LITTLE_TIP_EXT].pose.position;
            const XrVector3f& hOne = handData.JointLocations[XR_HAND_JOINT_THUMB_TIP_EXT].pose.position;

            const XrHandMeshVertexBufferMSFT& vertexBuffer = handData.meshState.vertexBuffer;
            handData.VertexColors.resize(vertexBuffer.vertexCountOutput);

            // Calculate the normalized length of a vertex to a line segment defined by two point [zero, one].
            auto weight = [](const XrVector3f& v, const XrVector3f& zero, const XrVector3f& one) -> float {
                const XrVector3f dm = {one.x - zero.x, one.y - zero.y, one.z - zero.z};
                const XrVector3f dv = {v.x - zero.x, v.y - zero.y, v.z - zero.z};
                const float lm = dm.x * dm.x + dm.y * dm.y + dm.z * dm.z;
                const float vm = dv.x * dm.x + dv.y * dm.y + dv.z * dm.z;

                return std::min(1.f, std::max(0.f, vm / lm));
            };

            for (uint32_t i = 0; i < vertexBuffer.vertexCountOutput; i++) {
                const XrVector3f& vertexPosition = vertexBuffer.vertices[i].position;
                const float v = weight(vertexPosition, vZero, vOne);
                const float h = weight(vertexPosition, hZero, hOne);
                // Pick a simple psuedo color map to visualize figers in colors.
                handData.VertexColors[i] = {v, (1 - h), h, 1};
            }
        }

        Pbr::PrimitiveBuilder CreateHandMeshPrimitiveBuilder(uint32_t* indices,
                                                             uint32_t indexCount,
                                                             XrHandMeshVertexMSFT* vertices,
                                                             uint32_t vertexCount,
                                                             const std::vector<XMFLOAT4>& vertexColors) {
            Pbr::PrimitiveBuilder builder;
            builder.Vertices.resize(vertexCount);

            builder.Indices = std::vector<uint32_t>(indices, indices + indexCount);

            for (uint32_t i = 0; i < vertexCount; i++, vertices++) {
                Pbr::Vertex& vertex = builder.Vertices[i];

                vertex.Position = xr::math::cast(vertices->position);
                vertex.Normal = xr::math::cast(vertices->normal);
                vertex.Color0 = vertexColors[i];

                bool xDominant = std::abs(vertex.Normal.x) > std::abs(vertex.Normal.y);
                XMVECTOR basis = xDominant ? g_XMIdentityR1 : g_XMIdentityR0;
                XMVECTOR normal = XMLoadFloat3(&vertex.Normal);
                XMVECTOR tangent = XMVector3Cross(normal, basis);
                XMStoreFloat4(&vertex.Tangent, tangent);

                XMStoreFloat2(&vertex.TexCoord0, g_XMZero);
                vertex.ModelTransformIndex = Pbr::RootNodeIndex; // Index into the node transforms
            }

            return builder;
        }

        // Detects two spaces collide to each other
        class StateChangeDetector {
        public:
            StateChangeDetector(std::function<bool(XrTime)> getState, std::function<void()> callback)
                : m_getState(std::move(getState))
                , m_callback(std::move(callback)) {
            }

            void Update(XrTime time) {
                bool state = m_getState(time);
                if (m_lastState != state) {
                    m_lastState = state;
                    if (state) { // trigger on rising edge
                        m_callback();
                    }
                }
            }

        private:
            const std::function<bool(XrTime)> m_getState;
            const std::function<void()> m_callback;
            std::optional<bool> m_lastState{};
        };

        bool m_enableHandMesh{false};
        enum class HandDisplayMode { Mesh, Joints, Count };
        HandDisplayMode m_mode{HandDisplayMode::Joints};

        std::shared_ptr<Pbr::Material> m_meshMaterial, m_jointMaterial;

        HandData m_leftHandData;
        HandData m_rightHandData;
        std::unique_ptr<StateChangeDetector> m_clapDetector;
    };
} // namespace

std::unique_ptr<engine::Scene> TryCreateHandTrackingScene(engine::Context& context) {
    if (!context.Extensions.SupportsHandJointTracking || !context.System.HandTrackingProperties.supportsHandTracking) {
        return nullptr;
    }

    const bool enableHandMesh = context.Extensions.SupportsHandMeshTracking && context.System.HandMeshProperties.supportsHandTrackingMesh;

    return std::make_unique<HandTrackingScene>(context, enableHandMesh);
}
