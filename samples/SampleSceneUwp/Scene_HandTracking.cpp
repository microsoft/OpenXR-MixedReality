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
#include <XrUtility/XrHand.h>
#include "PbrModelObject.h"
#include "Scene.h"

using namespace DirectX;

namespace {

    //
    // This sample displays hand tracking inputs appears as hand mesh, or joint axes.
    // User can clap their hands to toggle between different display mode
    // It also demos a simple procedural coloring of the hand mesh using the "open palm" reference hand input.
    //
    struct HandTrackingScene : public Scene {
        HandTrackingScene(SceneContext& sceneContext)
            : Scene(sceneContext) {
            m_jointMaterial = Pbr::Material::CreateFlat(m_sceneContext.PbrResources, Pbr::RGBA::White, 0.85f, 0.01f);
            m_meshMaterial = Pbr::Material::CreateFlat(m_sceneContext.PbrResources, Pbr::RGBA::White, 1, 0);

            auto createJointObjects = [&](HandData& handData) {
                auto jointModel = std::make_shared<Pbr::Model>();
                Pbr::PrimitiveBuilder primitiveBuilder;

                XrHandPoseTypeInfoMSFT poseTypeCreateInfo{XR_TYPE_HAND_POSE_TYPE_INFO_MSFT};
                poseTypeCreateInfo.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
                for (XrHandJointMSFT joint : xr::HandJoints) {
                    XrHandJointSpaceCreateInfoMSFT jointCreateInfo{XR_TYPE_HAND_JOINT_SPACE_CREATE_INFO_MSFT, &poseTypeCreateInfo};
                    jointCreateInfo.joint = joint;
                    jointCreateInfo.handTracker = handData.TrackerHandle.Get();
                    jointCreateInfo.poseInJointSpace = xr::math::Pose::Identity();

                    JointData& jointData = handData.JointData[xr::JointToIndex(joint)];
                    CHECK_XRCMD(m_sceneContext.Extensions.xrCreateHandJointSpaceMSFT(
                        m_sceneContext.Session, &jointCreateInfo, jointData.JointSpace.Put()));

                    // Create a axis object attached to each joint space
                    jointData.NodeIndex = jointModel->AddNode(DirectX::XMMatrixIdentity(), Pbr::RootNodeIndex, "joint");
                    primitiveBuilder.AddAxis(1.0f, 0.5f, jointData.NodeIndex);
                }

                // Now that the axis have been added for each joint into the primitive builder,
                // it can be baked into the model as a single primitive.
                jointModel->AddPrimitive(Pbr::Primitive(m_sceneContext.PbrResources, primitiveBuilder, m_jointMaterial));
                handData.JointModel = AddSceneObject(std::make_shared<PbrModelObject>(std::move(jointModel)));
                handData.JointModel->SetVisible(false);
            };

            // For each hand, initialize the joint objects, hand mesh buffers and corresponding spaces.
            const std::tuple<XrHandMSFT, HandData&> hands[] = {{XrHandMSFT::XR_HAND_LEFT_MSFT, m_leftHandData},
                                                               {XrHandMSFT::XR_HAND_RIGHT_MSFT, m_rightHandData}};
            for (const auto& [hand, handData] : hands) {
                XrHandTrackerCreateInfoMSFT createInfo{XR_TYPE_HAND_TRACKER_CREATE_INFO_MSFT};
                createInfo.hand = hand;
                CHECK_XRCMD(m_sceneContext.Extensions.xrCreateHandTrackerMSFT(
                    m_sceneContext.Session, &createInfo, handData.TrackerHandle.Put(m_sceneContext.Extensions.xrDestroyHandTrackerMSFT)));

                createJointObjects(handData);

                // Pre-allocate buffers for hand mesh indices and vertices
                const XrSystemHandTrackingMeshPropertiesMSFT& handMeshSystemProperties = sceneContext.System.HandMeshProperties;
                handData.IndexBuffer = std::make_unique<uint32_t[]>(handMeshSystemProperties.maxHandMeshIndexCount);
                handData.VertexBuffer = std::make_unique<XrHandMeshVertexMSFT[]>(handMeshSystemProperties.maxHandMeshVertexCount);

                handData.meshState.indexBuffer.indexCapacityInput = handMeshSystemProperties.maxHandMeshIndexCount;
                handData.meshState.indexBuffer.indices = handData.IndexBuffer.get();
                handData.meshState.vertexBuffer.vertexCapacityInput = handMeshSystemProperties.maxHandMeshVertexCount;
                handData.meshState.vertexBuffer.vertices = handData.VertexBuffer.get();

                XrHandMeshSpaceCreateInfoMSFT meshSpaceCreateInfo{XR_TYPE_HAND_MESH_SPACE_CREATE_INFO_MSFT};
                meshSpaceCreateInfo.handTracker = handData.TrackerHandle.Get();
                meshSpaceCreateInfo.poseInHandMeshSpace = xr::math::Pose::Identity();
                meshSpaceCreateInfo.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
                CHECK_XRCMD(m_sceneContext.Extensions.xrCreateHandMeshSpaceMSFT(
                    m_sceneContext.Session, &meshSpaceCreateInfo, handData.MeshSpace.Put()));

                meshSpaceCreateInfo.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;
                CHECK_XRCMD(m_sceneContext.Extensions.xrCreateHandMeshSpaceMSFT(
                    m_sceneContext.Session, &meshSpaceCreateInfo, handData.ReferenceMeshSpace.Put()));
            }

            // Set a clap detector that will toggle the display mode.
            m_clapDetector = std::make_unique<SpaceCollider>(
                SpaceCollider::Condition{m_leftHandData.JointData[xr::JointToIndex(XR_HAND_JOINT_PALM_MSFT)].JointSpace.Get(),
                                         m_rightHandData.JointData[xr::JointToIndex(XR_HAND_JOINT_PALM_MSFT)].JointSpace.Get(),
                                         /* DistanceTolerance */ 0.02f},
                [this]() { m_mode = (HandDisplayMode)(((uint32_t)m_mode + 1) % (uint32_t)HandDisplayMode::Count); });
        }

        void OnUpdate(const FrameTime& frameTime) override {
            // Detect hand clap to toggle hand display mode.
            m_clapDetector->Update(frameTime.PredictedDisplayTime);

            for (HandData& handData : {std::ref(m_leftHandData), std::ref(m_rightHandData)}) {
                bool jointsVisible = m_mode == HandDisplayMode::Joints;
                bool meshVisible = m_mode == HandDisplayMode::Mesh;

                XrHandTrackerStateMSFT handTrackerState{XR_TYPE_HAND_TRACKER_STATE_MSFT};
                CHECK_XRCMD(m_sceneContext.Extensions.xrGetHandTrackerStateMSFT(
                    handData.TrackerHandle.Get(), frameTime.PredictedDisplayTime, &handTrackerState));

                if (!handTrackerState.isActive) {
                    jointsVisible = false;
                    meshVisible = false;
                }

                if (jointsVisible) {
                    jointsVisible = UpdateJoints(handData, m_sceneContext.SceneSpace, frameTime.PredictedDisplayTime);
                }

                if (meshVisible) {
                    meshVisible = UpdateMesh(handData, m_sceneContext.SceneSpace, frameTime.PredictedDisplayTime);
                }

                handData.JointModel->SetVisible(jointsVisible);
                if (handData.MeshSceneObject != nullptr) { // Pbr mesh object creation is deferred.
                    handData.MeshSceneObject->SetVisible(meshVisible);
                }
            }
        }

        struct JointData {
            xr::SpaceHandle JointSpace;
            Pbr::NodeIndex_t NodeIndex;
        };

        struct HandData {
            xr::HandTrackerHandle TrackerHandle;

            // Data to display hand joints tracking
            std::shared_ptr<PbrModelObject> JointModel;
            std::array<JointData, xr::HandJointCount> JointData;

            // Data to display hand mesh tracking
            xr::SpaceHandle MeshSpace;
            xr::SpaceHandle ReferenceMeshSpace;
            std::vector<XMFLOAT4> VertexColors;
            std::shared_ptr<PbrModelObject> MeshSceneObject;

            // Data to process open-palm reference hand.
            XrHandMeshMSFT meshState{XR_TYPE_HAND_MESH_MSFT};
            std::unique_ptr<uint32_t[]> IndexBuffer{};
            std::unique_ptr<XrHandMeshVertexMSFT[]> VertexBuffer{};
        };

        bool UpdateJoints(HandData& handData, XrSpace referenceSpace, XrTime time) {
            bool jointsVisible = false;
            XrHandJointRadiusMSFT jointRadius{XR_TYPE_HAND_JOINT_RADIUS_MSFT};
            XrSpaceLocation jointLocation{XR_TYPE_SPACE_LOCATION, &jointRadius};
            for (const auto& [jointSpace, nodeIndex] : handData.JointData) {
                CHECK_XRCMD(xrLocateSpace(jointSpace.Get(), referenceSpace, time, &jointLocation));
                if (xr::math::Pose::IsPoseValid(jointLocation)) {
                    Pbr::Node& jointNode = handData.JointModel->GetModel()->GetNode(nodeIndex);

                    const float radius = jointRadius.radius;
                    jointNode.SetTransform(XMMatrixScaling(radius, radius, radius) * xr::math::LoadXrPose(jointLocation.pose));

                    jointsVisible = true;
                }
            }

            return jointsVisible;
        }

        bool UpdateMesh(HandData& handData, XrSpace referenceSpace, XrTime time) {
            XrHandMeshUpdateInfoMSFT meshUpdateInfo{XR_TYPE_HAND_MESH_UPDATE_INFO_MSFT};
            meshUpdateInfo.time = time;
            meshUpdateInfo.handPoseType = XR_HAND_POSE_TYPE_TRACKED_MSFT;
            CHECK_XRCMD(m_sceneContext.Extensions.xrUpdateHandMeshMSFT(handData.TrackerHandle.Get(), &meshUpdateInfo, &handData.meshState));

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

                if (!handData.MeshSceneObject) {
                    // The hand mesh scene object doesn't exist yet and must be created.
                    Pbr::Primitive surfacePrimitive(m_sceneContext.PbrResources, meshBuilder, m_meshMaterial, false /* updatableBuffers */);

                    auto surfaceModel = std::make_shared<Pbr::Model>();
                    surfaceModel->AddPrimitive(std::move(surfacePrimitive));

                    handData.MeshSceneObject = AddSceneObject(std::make_shared<PbrModelObject>(surfaceModel));
                } else {
                    // Update vertices and indices of the existing hand mesh scene object's primitive.
                    handData.MeshSceneObject->GetModel()->GetPrimitive(0).UpdateBuffers(
                        m_sceneContext.Device.get(), m_sceneContext.DeviceContext.get(), meshBuilder);
                }
            }

            if (handData.MeshSceneObject) {
                XrSpaceLocation meshLocation{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(handData.MeshSpace.Get(), referenceSpace, time, &meshLocation));
                if (xr::math::Pose::IsPoseValid(meshLocation)) {
                    handData.MeshSceneObject->Pose() = meshLocation.pose;
                    return true;
                }
            }

            return false;
        }

        void ComputeHandMeshColor(HandData& handData, XrTime time) {
            // Compute a color for each vertex on hand mesh based on relative position to an open palm reference hand.
            // Use the middle finger tip and wrist joints to normalize the vertical range.
            // Use the little finger tip and thumb tip joints to normalize the horizonal range.
            XrVector3f vZero{}, vOne{}, hZero{}, hOne{};
            for (const auto& [joint, jointPosition] :
                 std::vector<std::tuple<XrHandJointMSFT, XrVector3f&>>{{XR_HAND_JOINT_MIDDLE_TIP_MSFT, vZero},
                                                                       {XR_HAND_JOINT_WRIST_MSFT, vOne},
                                                                       {XR_HAND_JOINT_LITTLE_TIP_MSFT, hZero},
                                                                       {XR_HAND_JOINT_THUMB_TIP_MSFT, hOne}}) {
                XrHandPoseTypeInfoMSFT handPoseTypeInfo{XR_TYPE_HAND_POSE_TYPE_INFO_MSFT};
                handPoseTypeInfo.handPoseType = XR_HAND_POSE_TYPE_REFERENCE_OPEN_PALM_MSFT;
                XrHandJointSpaceCreateInfoMSFT createInfo{XR_TYPE_HAND_JOINT_SPACE_CREATE_INFO_MSFT, &handPoseTypeInfo};
                createInfo.handTracker = handData.TrackerHandle.Get();
                createInfo.poseInJointSpace = xr::math::Pose::Identity();
                createInfo.joint = joint;

                xr::SpaceHandle jointSpace;
                CHECK_XRCMD(m_sceneContext.Extensions.xrCreateHandJointSpaceMSFT(m_sceneContext.Session, &createInfo, jointSpace.Put()));

                XrSpaceLocation jointLocation{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(jointSpace.Get(), handData.ReferenceMeshSpace.Get(), time, &jointLocation));

                assert(xr::math::Pose::IsPoseValid(jointLocation));
                jointPosition = jointLocation.pose.position;
            }

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
        class SpaceCollider {
        public:
            struct Condition {
                XrSpace A;
                XrSpace B;
                float DistanceTolerance;
            };

            SpaceCollider() = delete;
            SpaceCollider(SpaceCollider&) = delete;
            SpaceCollider(SpaceCollider&&) = delete;

            SpaceCollider(Condition condition, std::function<void()> callback)
                : m_condition(std::move(condition))
                , m_callback(callback) {
            }

            void Update(XrTime time) {
                XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(m_condition.A, m_condition.B, time, &location));
                if (xr::math::Pose::IsPoseValid(location)) {
                    const auto position = xr::math::LoadXrVector3(location.pose.position);
                    const float distance = XMVectorGetX(XMVector3Length(position));
                    const bool state = distance < m_condition.DistanceTolerance;
                    const bool trigger = lastState.has_value() && !lastState.value() && state;
                    lastState = state;
                    if (trigger) {
                        m_callback();
                    }
                }
            }

        private:
            Condition m_condition;
            std::function<void()> m_callback;
            std::optional<bool> lastState{};
        };

        enum class HandDisplayMode { Mesh, Joints, Count };
        HandDisplayMode m_mode{HandDisplayMode::Mesh};

        std::shared_ptr<Pbr::Material> m_meshMaterial, m_jointMaterial;

        HandData m_leftHandData;
        HandData m_rightHandData;
        std::unique_ptr<SpaceCollider> m_clapDetector;
    };
} // namespace

std::unique_ptr<Scene> TryCreateHandTrackingScene(SceneContext& sceneContext) {
    if (!sceneContext.Extensions.SupportsHandJointTracking || !sceneContext.System.HandTrackingProperties.supportsHandTracking) {
        return nullptr;
    }

    if (!sceneContext.Extensions.SupportsHandMeshTracking || !sceneContext.System.HandMeshProperties.supportsHandTrackingMesh) {
        return nullptr;
    }

    return std::make_unique<HandTrackingScene>(sceneContext);
}
