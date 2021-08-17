#include "pch.h"
#include <unordered_set>
#include <DirectXCollision.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <XrUtility/XrString.h>
#include <XrUtility/XrSceneUnderstanding.hpp>
#include <pbr/GltfLoader.h>
#include <SampleShared/FileUtility.h>
#include <SampleShared/TextureUtility.h>
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>
#include <XrSceneLib/SpaceObject.h>

//
// This sample displays a cube that can be placed on physical surfaces.
// The user can point their hands at a surface to display the surface's plane
// and then air tap to place a cube on that surface.
// Once a cube has been placed on a surface then that surface will continue to be displayed.
//

using namespace std::chrono_literals;
using TimePoint = engine::FrameTime::clock::time_point;

namespace {
    using namespace DirectX;
    constexpr auto UpdateInterval = 5s; // Time to wait between SU requests
    constexpr float ScanRadius = 4.8f;  // meters
    constexpr size_t TextureSideLength = 32;
    constexpr float CubeSideLength = 0.1f;
    constexpr int LeftHand = 0;
    constexpr int RightHand = 1;
    constexpr int HandCount = 2;

    enum class RaycastAction { Activate, Searching };

    struct DistanceSortableIndices {
        float distance;
        XrPosef hitPose;
        size_t planeIndex;

        bool operator<(const DistanceSortableIndices& other) const noexcept {
            return distance < other.distance;
        }
    };

    struct SceneVisuals {
        SceneVisuals() = default;
        SceneVisuals(std::unique_ptr<xr::su::Scene> scene,
                     std::vector<XrUuidMSFT> componentIds,
                     std::vector<xr::su::ScenePlane> planes,
                     std::vector<std::shared_ptr<engine::Object>> visuals)
            : scene(std::move(scene))
            , componentIds(std::move(componentIds))
            , planes(std::move(planes))
            , visuals(std::move(visuals)) {
        }

        std::unique_ptr<xr::su::Scene> scene;
        std::vector<XrUuidMSFT> componentIds;
        std::vector<xr::su::ScenePlane> planes;
        std::vector<std::shared_ptr<engine::Object>> visuals;

        void ForEachEngineObject(const std::function<void(const std::shared_ptr<engine::Object>&)>& func) {
            for (const auto& visual : visuals) {
                func(visual);
            }
        }
    };

    struct PlacedObject {
        xr::SpatialAnchorHandle anchor;
        xr::SpaceHandle space;
        std::shared_ptr<engine::Object> visual;
    };

    struct IHandRayListener {
        virtual void RayUpdated(int hand, const XrPosef& handPose, XrTime time, RaycastAction raycastAction) = 0;
        virtual void RayLost(int hand, XrTime time) = 0;
    };

    bool XM_CALLCONV RayIntersectQuad(DirectX::FXMVECTOR rayPosition,
                                      DirectX::FXMVECTOR rayDirection,
                                      DirectX::FXMVECTOR v0,
                                      DirectX::FXMVECTOR v1,
                                      DirectX::FXMVECTOR v2,
                                      DirectX::FXMVECTOR v3,
                                      XrPosef* hitPose,
                                      float& distance);
    Pbr::RGBAColor GetColor(XrSceneObjectTypeMSFT type);
    std::shared_ptr<Pbr::Material> CreateTextureMaterial(Pbr::Resources& pbr);
    SceneVisuals CreateSceneVisuals(const xr::ExtensionDispatchTable& extensions,
                                    const Pbr::Resources& pbrResources,
                                    const std::shared_ptr<Pbr::Material>& material,
                                    std::unique_ptr<xr::su::Scene> scene);

    struct HandRays {
        HandRays(engine::Context& context, sample::ActionContext& actionContext, IHandRayListener& rayListener)
            : m_context{context}
            , m_rayListener(rayListener) {
            const std::vector<std::string> subactionPaths = {"/user/hand/right", "/user/hand/left"};
            m_placementActionSet = &actionContext.CreateActionSet("placement_actions", "Placement Actions");
            m_aimPoseAction =
                m_placementActionSet->CreateAction("place_pointer", "Place Pointer", XR_ACTION_TYPE_POSE_INPUT, subactionPaths);
            m_placeObjectAction =
                m_placementActionSet->CreateAction("place_object", "Place Object", XR_ACTION_TYPE_BOOLEAN_INPUT, subactionPaths);

            actionContext.SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                                                            {
                                                                {m_aimPoseAction, "/user/hand/right/input/aim"},
                                                                {m_aimPoseAction, "/user/hand/left/input/aim"},
                                                                {m_placeObjectAction, "/user/hand/right/input/trigger"},
                                                                {m_placeObjectAction, "/user/hand/left/input/trigger"},
                                                            });

            actionContext.SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                                                            {
                                                                {m_aimPoseAction, "/user/hand/right/input/aim"},
                                                                {m_aimPoseAction, "/user/hand/left/input/aim"},
                                                                {m_placeObjectAction, "/user/hand/right/input/select"},
                                                                {m_placeObjectAction, "/user/hand/left/input/select"},
                                                            });
            if (context.Extensions.SupportsHandInteraction) {
                actionContext.SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                {
                                                                    {m_aimPoseAction, "/user/hand/right/input/aim"},
                                                                    {m_aimPoseAction, "/user/hand/left/input/aim"},
                                                                    {m_placeObjectAction, "/user/hand/right/input/select"},
                                                                    {m_placeObjectAction, "/user/hand/left/input/select"},
                                                                });
            }

            // Aim space
            XrActionSpaceCreateInfo actionSpaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            actionSpaceCreateInfo.action = m_aimPoseAction;
            actionSpaceCreateInfo.poseInActionSpace = xr::math::Pose::Identity();

            actionSpaceCreateInfo.subactionPath = m_context.Instance.LeftHandPath;
            CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &actionSpaceCreateInfo, m_leftPointerSpace.Put()));

            actionSpaceCreateInfo.subactionPath = m_context.Instance.RightHandPath;
            CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &actionSpaceCreateInfo, m_rightPointerSpace.Put()));
        }

        void OnUpdate(const engine::FrameTime& frameTime) {
            const auto SetPointerVisibilityAndLocation = [&](engine::PbrModelObject& object, XrSpace space) {
                object.SetVisible(false);
                XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(space, m_context.AppSpace, frameTime.PredictedDisplayTime, &spaceLocation));
                if (xr::math::Pose::IsPoseValid(spaceLocation)) {
                    object.Pose() = spaceLocation.pose;
                    object.SetVisible(m_placementActionSet->Active());
                }
            };

            assert(m_leftPointerObject != nullptr);
            SetPointerVisibilityAndLocation(*m_leftPointerObject, m_leftPointerSpace.Get());
            SetPointerVisibilityAndLocation(*m_rightPointerObject, m_rightPointerSpace.Get());

            const auto checkHandActivation = [&](int hand, XrPath handPath, XrSpace space) {
                XrActionStateBoolean actionState{XR_TYPE_ACTION_STATE_BOOLEAN};
                XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                getInfo.action = m_placeObjectAction;
                getInfo.subactionPath = handPath;
                CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &actionState));
                const RaycastAction raycastAction =
                    actionState.changedSinceLastSync && actionState.currentState ? RaycastAction::Activate : RaycastAction::Searching;

                XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(space, m_context.AppSpace, frameTime.PredictedDisplayTime, &location));
                if (xr::math::Pose::IsPoseValid(location)) {
                    m_rayListener.RayUpdated(hand, location.pose, frameTime.PredictedDisplayTime, raycastAction);
                } else {
                    m_rayListener.RayLost(hand, frameTime.PredictedDisplayTime);
                }
            };
            checkHandActivation(LeftHand, m_context.Instance.LeftHandPath, m_leftPointerSpace.Get());
            checkHandActivation(RightHand, m_context.Instance.RightHandPath, m_rightPointerSpace.Get());
        }

        void SetPointerObjects(std::shared_ptr<engine::PbrModelObject> left, std::shared_ptr<engine::PbrModelObject> right) {
            m_leftPointerObject = std::move(left);
            m_rightPointerObject = std::move(right);
        }

    private:
        engine::Context& m_context;
        IHandRayListener& m_rayListener;

        sample::ActionSet* m_placementActionSet{nullptr};
        XrAction m_placeObjectAction{XR_NULL_HANDLE};
        XrAction m_aimPoseAction{XR_NULL_HANDLE};
        xr::SpaceHandle m_leftPointerSpace;
        xr::SpaceHandle m_rightPointerSpace;

        std::shared_ptr<engine::PbrModelObject> m_leftPointerObject;
        std::shared_ptr<engine::PbrModelObject> m_rightPointerObject;
    };

    struct PlacementScene : public engine::Scene, public IHandRayListener {
        explicit PlacementScene(engine::Context& context)
            : Scene(context)
            , m_extensions(context.Extensions)
            , m_planeMaterial(CreateTextureMaterial(context.PbrResources))
            , m_nextUpdate{engine::FrameTime::clock::now() + UpdateInterval}
            , m_handRays{context, ActionContext(), *this} {
            XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(context.Session.Handle, &spaceCreateInfo, m_viewSpace.Put()));

            constexpr float AxisLength = 0.2f;
            m_previewCubes[LeftHand] = engine::CreateAxis(m_context.PbrResources, AxisLength);
            m_previewCubes[RightHand] = engine::CreateAxis(m_context.PbrResources, AxisLength);
            for (const auto& object : m_previewCubes) {
                object->SetVisible(false);
                AddObject(object);
            }
            m_sceneBounds.sphereBounds.push_back({{}, ScanRadius});
        }

        ~PlacementScene() override {
            // Stop the worker thread first before destroying this class
            if (m_future.valid()) {
                m_future.wait();
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            if (m_sceneObserver == nullptr) {
                Enable();
            }
            m_lastTimeOfUpdate = frameTime.PredictedDisplayTime;

            // Check if the background thread finished creating a new group of visuals.
            if (m_scanState == ScanState::Processing && m_future.valid() && m_future.wait_for(0s) == std::future_status::ready) {
                m_sceneVisuals.ForEachEngineObject([this](const std::shared_ptr<engine::Object>& object) { RemoveObject(object); });
                m_sceneVisuals = m_future.get();
                m_sceneVisuals.ForEachEngineObject([this](const std::shared_ptr<engine::Object>& object) { AddObject(object); });
                m_scanState = ScanState::Idle;
                m_nextUpdate = frameTime.Now + UpdateInterval;
            }

            // Update the location of all scene objects
            if (m_sceneVisuals.scene) {
                xr::su::LocateObjects(m_sceneVisuals.scene->Handle(),
                                      m_extensions,
                                      m_context.AppSpace,
                                      frameTime.PredictedDisplayTime,
                                      m_sceneVisuals.componentIds,
                                      m_componentLocations);
                for (size_t i = 0; i < m_sceneVisuals.componentIds.size(); ++i) {
                    const XrSceneComponentLocationMSFT& location = m_componentLocations[i];
                    const std::shared_ptr<engine::Object>& object = m_sceneVisuals.visuals[i];
                    const xr::su::ScenePlane& scenePlane = m_sceneVisuals.planes[i];
                    if (m_visiblePlanes.count(scenePlane.id) != 0 || scenePlane.id == m_highlightedPlanes[LeftHand] ||
                        scenePlane.id == m_highlightedPlanes[RightHand]) {
                        object->SetVisible(true);
                    }
                    if (xr::math::Pose::IsPoseValid(location.flags)) {
                        object->Pose() = location.pose;
                    } else {
                        object->SetVisible(false);
                    }
                }
            }

            XrSpaceLocation viewInLocal{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInLocal));
            if (xr::math::Pose::IsPoseValid(viewInLocal)) {
                m_sceneBounds.sphereBounds[0].center = viewInLocal.pose.position;
            }

            UpdatePlacedObjects(frameTime.PredictedDisplayTime);
            m_handRays.OnUpdate(frameTime);

            if (m_scanState == ScanState::Waiting) {
                // Check if the results are available
                const XrSceneComputeStateMSFT state = m_sceneObserver->GetSceneComputeState();
                if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
                    // Send the scene compute result to the background thread for processing
                    m_future = std::async(std::launch::async,
                                          &CreateSceneVisuals,
                                          std::cref(m_extensions),
                                          std::cref(m_context.PbrResources),
                                          m_planeMaterial,
                                          m_sceneObserver->CreateScene());
                    m_scanState = ScanState::Processing;
                } else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
                    sample::Trace(L"Compute completed with error");
                    m_scanState = ScanState::Idle;
                }
            } else if (m_scanState == ScanState::Idle) {
                // no active query, start one if enough time has passed
                if (frameTime.Now > m_nextUpdate) {
                    // Start the async query
                    m_sceneBounds.space = m_context.AppSpace;
                    m_sceneBounds.time = m_lastTimeOfUpdate;
                    static const std::vector<XrSceneComputeFeatureMSFT> Features{XR_SCENE_COMPUTE_FEATURE_PLANE_MSFT,
                                                                                 XR_SCENE_COMPUTE_FEATURE_PLANE_MESH_MSFT};
                    m_sceneObserver->ComputeNewScene(Features, m_sceneBounds);

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

        void RayUpdated(int hand, const XrPosef& handPose, XrTime time, RaycastAction raycastAction) override {
            if (!m_sceneVisuals.scene) {
                return;
            }

            std::vector<DistanceSortableIndices> objectsHit;
            for (size_t i = 0; i < m_sceneVisuals.planes.size(); i++) {
                const auto& plane = m_sceneVisuals.planes[i];
                if (!xr::math::Pose::IsPoseValid(m_componentLocations[i].flags)) {
                    continue;
                }
                // Clockwise order
                const float halfWidth = plane.size.width / 2.0f;
                const float halfHeight = plane.size.height / 2.0f;
                auto v0 = XMVectorSet(-halfWidth, -halfHeight, 0, 1);
                auto v1 = XMVectorSet(-halfWidth, halfHeight, 0, 1);
                auto v2 = XMVectorSet(halfWidth, halfHeight, 0, 1);
                auto v3 = XMVectorSet(halfWidth, -halfHeight, 0, 1);
                const auto matrix = xr::math::LoadXrPose(m_componentLocations[i].pose);
                v0 = XMVector4Transform(v0, matrix);
                v1 = XMVector4Transform(v1, matrix);
                v2 = XMVector4Transform(v2, matrix);
                v3 = XMVector4Transform(v3, matrix);

                XMVECTOR rayPosition = xr::math::LoadXrVector3(handPose.position);

                const auto forward = XMVectorSet(0, 0, -1, 0);
                const auto rotation = xr::math::LoadXrQuaternion(handPose.orientation);
                XMVECTOR rayDirection = XMVector3Rotate(forward, rotation);

                XrPosef hitPose{};
                float distance = 0.0f;
                if (RayIntersectQuad(rayPosition, rayDirection, v0, v1, v2, v3, &hitPose, distance)) {
                    objectsHit.push_back({distance, hitPose, i});
                }
            }
            if (!objectsHit.empty()) {
                std::sort(objectsHit.begin(), objectsHit.end());
                const DistanceSortableIndices& objectHit = objectsHit[0];
                const xr::su::ScenePlane& plane = m_sceneVisuals.planes[objectHit.planeIndex];

                m_previewCubes[hand]->Pose() = objectHit.hitPose;
                m_previewCubes[hand]->SetVisible(raycastAction == RaycastAction::Searching);
                if (m_highlightedPlanes[hand] != plane.id) {
                    if (m_highlightedPlanes[hand].has_value()) {
                        UnselectPlane(m_highlightedPlanes[hand].value());
                    }
                    m_highlightedPlanes[hand] = plane.id;
                }

                if (raycastAction == RaycastAction::Activate) {
                    PlacedObject placedObject;
                    XrSpatialAnchorCreateInfoMSFT anchorCreateInfo{XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT};
                    anchorCreateInfo.space = m_context.AppSpace;
                    anchorCreateInfo.pose = objectHit.hitPose;
                    anchorCreateInfo.time = time;
                    XrResult result =
                        m_extensions.xrCreateSpatialAnchorMSFT(m_context.Session.Handle,
                                                               &anchorCreateInfo,
                                                               placedObject.anchor.Put(m_context.Extensions.xrDestroySpatialAnchorMSFT));
                    if (XR_FAILED(result)) {
                        if (result == XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT) {
                            sample::Trace("Anchor cannot be created, likely due to lost tracking. User should try again later");
                            return;
                        } else {
                            CHECK_XRRESULT(result, "xrCreateSpatialAnchorMSFT");
                        }
                    }
                    XrSpatialAnchorSpaceCreateInfoMSFT anchorSpaceCreateInfo{XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT};
                    anchorSpaceCreateInfo.anchor = placedObject.anchor.Get();
                    anchorSpaceCreateInfo.poseInAnchorSpace = xr::math::Pose::Identity();
                    CHECK_XRCMD(m_extensions.xrCreateSpatialAnchorSpaceMSFT(
                        m_context.Session.Handle, &anchorSpaceCreateInfo, placedObject.space.Put()));

                    auto cube = CreatePlacementCube();
                    cube->Pose() = objectHit.hitPose;
                    AddObject(cube);
                    placedObject.visual = cube;

                    m_visiblePlanes.insert(plane.id);
                    m_placedObjects.emplace_back(std::move(placedObject));
                }
            }
        }

        void RayLost(int hand, XrTime time) override {
            m_previewCubes[hand]->SetVisible(false);
            if (m_highlightedPlanes[hand].has_value()) {
                UnselectPlane(m_highlightedPlanes[hand].value());
            }
            m_highlightedPlanes[hand].reset();
        }

    private:
        enum class ScanState { Idle, Waiting, Processing };

        void Enable() {
            m_sceneObserver = std::make_unique<xr::su::SceneObserver>(m_extensions, m_context.Session.Handle);
            m_scanState = ScanState::Idle;

            const auto createPointerRay = [this](const std::shared_ptr<engine::PbrModelObject>& parent, const Pbr::RGBAColor& color) {
                auto aimRay = AddObject(engine::CreateCube(m_context.PbrResources, {1.0f, 1.0f, 1.0f}, color));
                aimRay->SetParent(parent);
                aimRay->Pose().position.z = -5.0f;
                aimRay->Scale() = {0.006f, 0.006f, 10.01f};
            };
            auto leftPointerObject = AddObject(std::make_shared<engine::PbrModelObject>(std::make_shared<Pbr::Model>()));
            auto rightPointerObject = AddObject(std::make_shared<engine::PbrModelObject>(std::make_shared<Pbr::Model>()));
            createPointerRay(leftPointerObject, Pbr::FromSRGB(Colors::HotPink));
            createPointerRay(rightPointerObject, Pbr::FromSRGB(Colors::Cyan));
            m_handRays.SetPointerObjects(std::move(leftPointerObject), std::move(rightPointerObject));
        }

        void Disable() {
            m_sceneVisuals.ForEachEngineObject([this](const auto& object) { RemoveObject(object); });
            m_sceneVisuals = {};

            // Stop the worker thread before clearing sceneObserver because the thread has access to it.
            if (m_future.valid()) {
                m_future.get();
            }
            m_sceneObserver = nullptr;
        }

        std::shared_ptr<engine::PbrModelObject> CreatePlacementCube() {
            return engine::CreateCube(
                m_context.PbrResources, {CubeSideLength, CubeSideLength, CubeSideLength}, Pbr::FromSRGB(Colors::Yellow));
        }

        void UpdatePlacedObjects(XrTime predictedDisplayTime) {
            for (PlacedObject& placedObject : m_placedObjects) {
                XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(placedObject.space.Get(), m_context.AppSpace, predictedDisplayTime, &spaceLocation));
                if (xr::math::Pose::IsPoseValid(spaceLocation)) {
                    placedObject.visual->SetVisible(true);
                    placedObject.visual->Pose() = spaceLocation.pose;
                } else {
                    placedObject.visual->SetVisible(false);
                }
            }
        }

        void UnselectPlane(xr::su::ScenePlane::Id id) {
            auto it = std::find_if(
                m_sceneVisuals.planes.begin(), m_sceneVisuals.planes.end(), [id](const auto& plane) { return plane.id == id; });
            if (it != m_sceneVisuals.planes.end() && m_visiblePlanes.count(id) == 0) {
                const auto index = static_cast<size_t>(std::distance(m_sceneVisuals.planes.begin(), it));
                m_sceneVisuals.visuals[index]->SetVisible(false);
            }
        }

        const xr::ExtensionDispatchTable& m_extensions;
        // The XrSceneObserver needs to be destroyed after SceneVisuals because SceneVisuals contains an XrScene.
        std::unique_ptr<xr::su::SceneObserver> m_sceneObserver;
        std::shared_ptr<Pbr::Material> m_planeMaterial;
        SceneVisuals m_sceneVisuals;
        std::future<SceneVisuals> m_future;
        std::vector<PlacedObject> m_placedObjects;
        std::array<std::shared_ptr<engine::Object>, HandCount> m_previewCubes;
        std::array<std::optional<xr::su::ScenePlane::Id>, HandCount> m_highlightedPlanes;
        // The planes that should remain visible because an object was placed on it
        std::unordered_set<xr::su::ScenePlane::Id> m_visiblePlanes;
        xr::SpaceHandle m_viewSpace;
        XrTime m_lastTimeOfUpdate{};
        xr::SceneBounds m_sceneBounds;
        std::vector<XrSceneComponentLocationMSFT> m_componentLocations;
        TimePoint m_nextUpdate{};
        ScanState m_scanState{ScanState::Idle};
        HandRays m_handRays;
    };

    bool XM_CALLCONV RayIntersectQuad(DirectX::FXMVECTOR rayPosition,
                                      DirectX::FXMVECTOR rayDirection,
                                      DirectX::FXMVECTOR v0,
                                      DirectX::FXMVECTOR v1,
                                      DirectX::FXMVECTOR v2,
                                      DirectX::FXMVECTOR v3,
                                      XrPosef* hitPose,
                                      float& distance) {
        // Not optimal. Should be possible to determine which triangle to test.
        bool hit = TriangleTests::Intersects(rayPosition, rayDirection, v0, v1, v2, distance);
        if (!hit) {
            hit = TriangleTests::Intersects(rayPosition, rayDirection, v3, v2, v0, distance);
        }
        if (hit && hitPose != nullptr) {
            FXMVECTOR hitPosition = XMVectorAdd(rayPosition, XMVectorScale(rayDirection, distance));
            FXMVECTOR plane = XMPlaneFromPoints(v0, v2, v1);

            // p' = p - (n ⋅ p + d) * n
            // Project the ray position onto the plane
            float t = XMVectorGetX(XMVector3Dot(plane, rayPosition)) + XMVectorGetW(plane);
            FXMVECTOR projPoint = XMVectorSubtract(rayPosition, XMVectorMultiply(XMVectorSet(t, t, t, 0), plane));

            // From the projected ray position, look towards the hit position and make the plane's normal "up"
            FXMVECTOR forward = XMVectorSubtract(hitPosition, projPoint);
            XMMATRIX virtualToGazeOrientation = XMMatrixLookToRH(hitPosition, forward, plane);
            xr::math::StoreXrPose(hitPose, XMMatrixInverse(nullptr, virtualToGazeOrientation));
        }
        return hit;
    }

    Pbr::RGBAColor GetColor(XrSceneObjectTypeMSFT type) {
        // The lighting system makes a lot of the colors too bright so multiply them to tone them down
        constexpr auto scaleColor = [](const XMVECTORF32& color, float scale) {
            return Pbr::FromSRGB(color * XMVECTORF32{scale, scale, scale, 1});
        };
        switch (type) {
        case XR_SCENE_OBJECT_TYPE_CEILING_MSFT:
            return Pbr::FromSRGB(Colors::Green);
        case XR_SCENE_OBJECT_TYPE_FLOOR_MSFT:
            return scaleColor(Colors::Blue, 0.5f);
        case XR_SCENE_OBJECT_TYPE_PLATFORM_MSFT:
            return scaleColor(Colors::Orange, 0.6f);
        case XR_SCENE_OBJECT_TYPE_WALL_MSFT:
            return scaleColor(Colors::Tomato, 0.5f);
        case XR_SCENE_OBJECT_TYPE_BACKGROUND_MSFT:
            return scaleColor(Colors::Cyan, 0.8f);
        case XR_SCENE_OBJECT_TYPE_UNCATEGORIZED_MSFT:
            return scaleColor(Colors::Purple, 0.8f);
        case XR_SCENE_OBJECT_TYPE_INFERRED_MSFT:
            return scaleColor(Colors::Yellow, 0.7f);
        default:
            return scaleColor(Colors::White, 0.7f);
        }
    }

    void FillMeshPrimitiveBuilder(const std::vector<XrVector3f>& positions,
                                  const std::vector<uint32_t>& indices,
                                  const Pbr::RGBAColor& color,
                                  Pbr::PrimitiveBuilder& builder) {
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

    std::shared_ptr<engine::PbrModelObject> CreateMeshVisual(const xr::ExtensionDispatchTable& extensions,
                                                             const Pbr::Resources& pbrResources,
                                                             const std::shared_ptr<Pbr::Material>& material,
                                                             XrSceneMSFT scene,
                                                             uint64_t meshBufferId,
                                                             Pbr::PrimitiveBuilder& builder,
                                                             const Pbr::RGBAColor& color) {
        std::vector<XrVector3f> vertexBuffer;
        std::vector<uint32_t> indexBuffer;
        xr::ReadMeshBuffers(scene, extensions, meshBufferId, vertexBuffer, indexBuffer);
        if (indexBuffer.empty() || vertexBuffer.empty()) {
            return nullptr;
        }
        FillMeshPrimitiveBuilder(vertexBuffer, indexBuffer, color, builder);
        auto model = std::make_shared<Pbr::Model>();
        model->AddPrimitive(Pbr::Primitive(pbrResources, builder, material));
        return std::make_shared<engine::PbrModelObject>(std::move(model));
    }

    std::shared_ptr<engine::PbrModelObject> CreatePlaneVisual(const xr::ExtensionDispatchTable& extensions,
                                                              const Pbr::Resources& pbrResources,
                                                              const std::shared_ptr<Pbr::Material>& material,
                                                              const xr::su::ScenePlane& scenePlane,
                                                              const Pbr::RGBAColor& color) {
        constexpr float TextureScale = 5.0f;
        const DirectX::XMFLOAT2 sideLengths{scenePlane.size.width, scenePlane.size.height};
        const DirectX::XMFLOAT2 textureCoord{sideLengths.x * TextureScale, sideLengths.y * TextureScale};
        auto model = std::make_shared<Pbr::Model>();
        model->AddPrimitive(
            Pbr::Primitive(pbrResources, Pbr::PrimitiveBuilder().AddQuad(sideLengths, textureCoord, Pbr::RootNodeIndex, color), material));
        return std::make_shared<engine::PbrModelObject>(std::move(model));
    }

    std::unordered_map<xr::su::SceneObject::Id, XrSceneObjectTypeMSFT> CreateTypeMap(const std::vector<xr::su::SceneObject>& sceneObjects) {
        std::unordered_map<xr::su::SceneObject::Id, XrSceneObjectTypeMSFT> map;
        map.reserve(sceneObjects.size());
        for (const xr::su::SceneObject& sceneObject : sceneObjects) {
            map.emplace(sceneObject.id, sceneObject.type);
        }
        return map;
    }

    SceneVisuals CreateSceneVisuals(const xr::ExtensionDispatchTable& extensions,
                                    const Pbr::Resources& pbrResources,
                                    const std::shared_ptr<Pbr::Material>& material,
                                    std::unique_ptr<xr::su::Scene> scene) {
        static const std::vector<xr::su::SceneObject::Type> typeFilter{XR_SCENE_OBJECT_TYPE_BACKGROUND_MSFT,
                                                                       XR_SCENE_OBJECT_TYPE_WALL_MSFT,
                                                                       XR_SCENE_OBJECT_TYPE_FLOOR_MSFT,
                                                                       XR_SCENE_OBJECT_TYPE_CEILING_MSFT,
                                                                       XR_SCENE_OBJECT_TYPE_PLATFORM_MSFT,
                                                                       XR_SCENE_OBJECT_TYPE_INFERRED_MSFT};
        std::vector<std::shared_ptr<engine::Object>> visuals;
        std::vector<XrUuidMSFT> componentIds;
        Pbr::PrimitiveBuilder builder;

        const std::unordered_map<xr::su::SceneObject::Id, XrSceneObjectTypeMSFT> sceneObjectIdToType =
            CreateTypeMap(scene->GetObjects(typeFilter));

        auto planes = scene->GetPlanes(typeFilter);
        for (const xr::su::ScenePlane& scenePlane : planes) {
            const XrSceneObjectTypeMSFT type = sceneObjectIdToType.at(scenePlane.parentId);
            std::shared_ptr<engine::PbrModelObject> obj = CreatePlaneVisual(extensions, pbrResources, material, scenePlane, GetColor(type));
            if (obj != nullptr) {
                obj->SetVisible(false);
                visuals.push_back(std::move(obj));
                componentIds.push_back(static_cast<XrUuidMSFT>(scenePlane.id));
            }
        }
        return SceneVisuals(std::move(scene), std::move(componentIds), std::move(planes), std::move(visuals));
    }

    std::shared_ptr<Pbr::Material> CreateTextureMaterial(Pbr::Resources& pbr) {
        ID3D11Device* const device = pbr.GetDevice().get();
        auto material = Pbr::Material::CreateFlat(pbr, Pbr::FromSRGB(DirectX::Colors::White), 1.0f, 0.0f);
        const std::vector<uint32_t> rgba = sample::CreateTileTextureBytes(TextureSideLength);
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

} // namespace

std::unique_ptr<engine::Scene> TryCreatePlacementScene(engine::Context& context) {
    return context.Extensions.SupportsSceneUnderstanding ? std::make_unique<PlacementScene>(context) : nullptr;
}
