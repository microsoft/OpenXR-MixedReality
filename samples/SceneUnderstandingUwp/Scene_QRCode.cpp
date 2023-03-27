#include "pch.h"
#include <unordered_set>
#include <DirectXCollision.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <XrUtility/XrString.h>
#include <pbr/GltfLoader.h>
#include <SampleShared/FileUtility.h>
#include <SampleShared/TextureUtility.h>
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>
#include <XrSceneLib/SpaceObject.h>
#include <XrSceneLib/TextTexture.h>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace DirectX;
using TimePoint = engine::FrameTime::clock::time_point;

namespace {
    enum class FilteringType { None = 0, Frustum, Sphere, Box, Count };

    constexpr auto UpdateInterval = 5s; // Time to wait between SU requests
    constexpr auto MaxMarkerAge = 600s; // Hide markers not detected for 10 minutes

    // Filtering objects constants
    constexpr float FrustumDistance = 2.0f;
    constexpr DirectX::XMFLOAT2 FrustumSpan = {1.0f, .5f};

    constexpr float SphereDistance = 1.0f;
    constexpr float SphereRadius = 0.3f;

    constexpr float BoxWidth = 1.0f;
    constexpr float BoxHeight = .6f;
    constexpr float BoxDepth = .4f;
    constexpr float BoxDistance = 1.0f;

    constexpr auto BoundsColor = Pbr::RGBAColor{0.0f, 1.0f, 0.0f, .3f};

    struct QRCodeScene : public engine::Scene {
        explicit QRCodeScene(engine::Context& context)
            : Scene(context)
            , m_filteringType(FilteringType::None)
            , m_nextUpdate{engine::FrameTime::clock::now() + UpdateInterval}
            , m_frustumBoundsTarget{engine::CreateQuad(
                  m_context.PbrResources, FrustumSpan, Pbr::Material::CreateFlat(m_context.PbrResources, BoundsColor))}
            , m_sphereBoundsTarget{engine::CreateSphere(m_context.PbrResources, 2 * SphereRadius, 32, BoundsColor)}
            , m_boxBoundsTarget{engine::CreateCube(m_context.PbrResources, XMFLOAT3{BoxWidth, BoxHeight, BoxDepth}, BoundsColor)} {
            // Our filtering bounds will be based on view space
            XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            createInfo.poseInReferenceSpace = xr::math::Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(context.Session.Handle, &createInfo, &m_viewSpace));

            m_frustumBoundsTarget->SetVisible(false);
            m_sphereBoundsTarget->SetVisible(false);
            m_boxBoundsTarget->SetVisible(false);
            AddObject(m_frustumBoundsTarget);
            AddObject(m_sphereBoundsTarget);
            AddObject(m_boxBoundsTarget);

            // Setup action to change filter
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("qrcodes_scene_actions", "QRCodes Scene Actions");

            const std::vector<std::string> subactionPathBothHands = {"/user/hand/right", "/user/hand/left"};

            m_switchFilterAction = actionSet.CreateAction(
                "switch_filter_action", "Switch Filter Action", XR_ACTION_TYPE_BOOLEAN_INPUT, subactionPathBothHands);

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                                                              {
                                                                  {m_switchFilterAction, "/user/hand/right/input/select/click"},
                                                                  {m_switchFilterAction, "/user/hand/left/input/select/click"},
                                                              });
            if (context.Extensions.SupportsHandInteractionMSFT) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                  {
                                                                      {m_switchFilterAction, "/user/hand/left/input/select/value"},
                                                                      {m_switchFilterAction, "/user/hand/right/input/select/value"},
                                                                  });
            }
        }

        bool TryGetFrustumQuadPose(XrTime time, XrPosef* frustumInAppSpace, XrPosef* quadInAppSpace) {
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_viewSpace, m_context.AppSpace, time, &location));
            if (!xr::math::Pose::IsPoseValid(location.locationFlags)) {
                return false;
            }
            // Frustum's pose is exactly View Pose at this precise moment
            *frustumInAppSpace = location.pose;

            // Quad should be displayed at the end of the frustum but is looking towards the view
            XrPosef quadInFrustum = xr::math::Pose::Translation({0.0f, 0.0f, -FrustumDistance});
            *quadInAppSpace = xr::math::Pose::Multiply(quadInFrustum, *frustumInAppSpace);
            return true;
        }

        bool TryGetSpherePose(XrTime time, XrPosef* sphereInAppSpace) {
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_viewSpace, m_context.AppSpace, time, &location));
            if (!xr::math::Pose::IsPoseValid(location.locationFlags)) {
                return false;
            }
            // Sphere is located in frot of the view
            XrPosef sphereInViewSpace = xr::math::Pose::Translation(XrVector3f{0.0f, 0.0f, -SphereDistance});
            *sphereInAppSpace = xr::math::Pose::Multiply(sphereInViewSpace, location.pose);
            return true;
        }

        bool TryGetBoxPose(XrTime time, XrPosef* boxInAppSpace) {
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_viewSpace, m_context.AppSpace, time, &location));
            if (!xr::math::Pose::IsPoseValid(location.locationFlags)) {
                return false;
            }
            // Sphere is located in frot of the view
            XrPosef boxInViewSpace = xr::math::Pose::Translation(XrVector3f{0.0f, 0.0f, -BoxDistance});
            *boxInAppSpace = xr::math::Pose::Multiply(boxInViewSpace, location.pose);
            return true;
        }

        bool ShouldSwitchFilter() {
            XrActionStateBoolean switchFilterState{XR_TYPE_ACTION_STATE_BOOLEAN};
            XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            getInfo.action = m_switchFilterAction;

            getInfo.subactionPath = m_context.Instance.RightHandPath;
            CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &switchFilterState));
            if (switchFilterState.isActive && switchFilterState.changedSinceLastSync && switchFilterState.currentState) {
                return true;
            }

            getInfo.subactionPath = m_context.Instance.LeftHandPath;
            CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &switchFilterState));
            if (switchFilterState.isActive && switchFilterState.changedSinceLastSync && switchFilterState.currentState) {
                return true;
            }
            return false;
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            if (!m_sceneObserver) {
                Enable();
            }
            m_lastTimeOfUpdate = frameTime.PredictedDisplayTime;

            // Check actions
            bool shouldSwitchFilter = ShouldSwitchFilter();
            if (shouldSwitchFilter) {
                // Reset all filter data and switch filter type
                m_sceneComputeInfo.bounds.boxCount = 0;
                m_sceneComputeInfo.bounds.sphereCount = 0;
                m_sceneComputeInfo.bounds.frustumCount = 0;
                m_boundsLocated = false;
                m_frustumBoundsTarget->SetVisible(false);
                m_sphereBoundsTarget->SetVisible(false);
                m_boxBoundsTarget->SetVisible(false);
                m_filteringType =
                    static_cast<FilteringType>((static_cast<size_t>(m_filteringType) + 1) % static_cast<size_t>(FilteringType::Count));
            }

            // Update the location of all scene objects
            if (m_markerIds.size() > 0) {
                XrSceneComponentsLocateInfoMSFT locateInfo{XR_TYPE_SCENE_COMPONENTS_LOCATE_INFO_MSFT};
                locateInfo.baseSpace = m_context.AppSpace;
                locateInfo.time = frameTime.PredictedDisplayTime;
                locateInfo.componentIdCount = static_cast<uint32_t>(m_markerIds.size());
                locateInfo.componentIds = m_markerIds.data();

                XrSceneComponentLocationsMSFT componentLocations{XR_TYPE_SCENE_COMPONENT_LOCATIONS_MSFT};
                componentLocations.locationCount = static_cast<uint32_t>(m_componentLocations.size());
                componentLocations.locations = m_componentLocations.data();

                CHECK_XRCMD(xrLocateSceneComponentsMSFT(m_scene.Get(), &locateInfo, &componentLocations));

                for (size_t i = 0; i < m_markerIds.size(); ++i) {
                    const XrSceneComponentLocationMSFT& location = m_componentLocations[i];
                    auto& visualInfo = m_markerVisuals[m_markerIds[i]];
                    const std::shared_ptr<engine::Object>& object = visualInfo.second;
                    const auto centerToPose = visualInfo.first;
                    if (xr::math::Pose::IsPoseValid(location.flags)) {
                        object->Pose() = xr::math::Pose::Multiply(centerToPose, location.pose);
                    } else {
                        object->SetVisible(false);
                    }
                }
            }

            if (m_scanState == ScanState::Waiting) {
                // Check if the results are available
                // xr_msft_scene_marker.h
                // 4. Poll xrGetSceneComputeStateMSFT, waiting for compute complete
                XrSceneComputeStateMSFT state{XR_SCENE_COMPUTE_STATE_NONE_MSFT};
                CHECK_XRCMD(xrGetSceneComputeStateMSFT(m_sceneObserver.Get(), &state));
                if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_MSFT) {
                    UpdateQRCodeVisuals(frameTime.PredictedDisplayTime);
                    m_scanState = ScanState::Idle;
                } else if (state == XR_SCENE_COMPUTE_STATE_COMPLETED_WITH_ERROR_MSFT) {
                    sample::Trace("Compute completed with error");
                    m_scanState = ScanState::Idle;
                }
            } else if (m_scanState == ScanState::Idle) {
                // Try to locate filtering bounds if needed
                if (!m_boundsLocated) {
                    switch (m_filteringType) {
                    case FilteringType::Frustum:
                        // Declare a Frustum in app_space, in the view direction, with a PI/2 angular width, PI angular height
                        // And starting in the View origin
                        {
                            XrPosef frustumPose{};
                            XrPosef quadPose{};
                            m_boundsLocated = TryGetFrustumQuadPose(m_lastTimeOfUpdate, &frustumPose, &quadPose);
                            if (m_boundsLocated) {
                                // View space localized, we can
                                //  (1) give a pose to the quad helping to understand the frustum
                                //  (2) give the frustum description in AppSpace to enable QRCode filtering
                                m_frustumBounds.pose = frustumPose;
                                // Let's target a 1m x 1m at "far distance"
                                m_frustumBounds.fov.angleUp = atan2f(FrustumSpan.y / 2.0f, FrustumDistance);
                                m_frustumBounds.fov.angleRight = atan2f(FrustumSpan.x / 2.0f, FrustumDistance);
                                m_frustumBounds.fov.angleDown = -m_frustumBounds.fov.angleUp;
                                m_frustumBounds.fov.angleLeft = -m_frustumBounds.fov.angleRight;
                                m_frustumBounds.farDistance = FrustumDistance;
                                m_sceneComputeInfo.bounds.frustumCount = 1;
                                m_sceneComputeInfo.bounds.frustums = &m_frustumBounds;

                                m_frustumBoundsTarget->Pose() = quadPose;
                            }
                        }
                        m_frustumBoundsTarget->SetVisible(m_boundsLocated);
                        break;

                    case FilteringType::Sphere:
                        // Declare a Sphare in app_space, in the view direction at known distance
                        {
                            XrPosef spherePose{};
                            m_boundsLocated = TryGetSpherePose(m_lastTimeOfUpdate, &spherePose);
                            if (m_boundsLocated) {
                                m_sphereBounds.center = spherePose.position;
                                m_sphereBounds.radius = SphereRadius;
                                m_sceneComputeInfo.bounds.sphereCount = 1;
                                m_sceneComputeInfo.bounds.spheres = &m_sphereBounds;

                                m_sphereBoundsTarget->Pose() = spherePose;
                            }
                        }
                        m_sphereBoundsTarget->SetVisible(m_boundsLocated);

                        break;

                    case FilteringType::Box:
                        // Declare a Sphare in app_space, in the view direction at known distance
                        {
                            XrPosef boxPose{};
                            m_boundsLocated = TryGetBoxPose(m_lastTimeOfUpdate, &boxPose);
                            if (m_boundsLocated) {
                                m_boxBounds.pose = boxPose;
                                m_boxBounds.extents = XrVector3f{BoxWidth, BoxHeight, BoxDepth};
                                m_sceneComputeInfo.bounds.boxCount = 1;
                                m_sceneComputeInfo.bounds.boxes = &m_boxBounds;

                                m_boxBoundsTarget->Pose() = boxPose;
                            }
                        }
                        m_boxBoundsTarget->SetVisible(m_boundsLocated);
                        break;
                    }
                }

                // no active query, start one if enough time has passed
                if (frameTime.Now > m_nextUpdate) {
                    // Ready to compute scene
                    bool computeScene = (m_filteringType == FilteringType::None) || m_boundsLocated;
                    if (computeScene) {
                        // Start the async query
                        // 3. If supported, call xrComputeNewSceneMSFT including XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT,
                        //    XrSceneBoundsMSFT will be ignored
                        const XrSceneComputeFeatureMSFT requestedFeature{XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT};
                        m_sceneComputeInfo.requestedFeatureCount = 1;
                        m_sceneComputeInfo.requestedFeatures = &requestedFeature;
                        m_sceneComputeInfo.bounds.space = m_context.AppSpace;
                        m_sceneComputeInfo.bounds.time = m_lastTimeOfUpdate;
                        // m_sceneComputeInfo.bounds's sphere, frustum and box are already set
                        m_sceneComputeInfo.consistency = XR_SCENE_COMPUTE_CONSISTENCY_SNAPSHOT_COMPLETE_MSFT;
                        CHECK_XRCMD(xrComputeNewSceneMSFT(m_sceneObserver.Get(), &m_sceneComputeInfo));

                        m_nextUpdate = frameTime.Now + UpdateInterval;
                        m_scanState = ScanState::Waiting;
                    }
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
            // xr_msft_scene_marker.h
            // 0. Call xrEnumerateSceneComputeFeaturesMSFT to check if observer supports XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT
            {
                uint32_t count = 0;
                CHECK_XRCMD(xrEnumerateSceneComputeFeaturesMSFT(m_context.Instance.Handle, m_context.System.Id, count, &count, nullptr));
                std::vector<XrSceneComputeFeatureMSFT> supportedFeatures(count);
                CHECK_XRCMD(xrEnumerateSceneComputeFeaturesMSFT(
                    m_context.Instance.Handle, m_context.System.Id, count, &count, supportedFeatures.data()));
                supportedFeatures.resize(count);
                if (std::find(supportedFeatures.begin(), supportedFeatures.end(), XR_SCENE_COMPUTE_FEATURE_MARKER_MSFT) ==
                    supportedFeatures.end()) {
                    throw std::logic_error("Markers are not supported");
                }
            }

            // xr_msft_scene_marker.h
            // 1. Call xrCreateSceneObserverMSFT
            XrSceneObserverCreateInfoMSFT createInfo{XR_TYPE_SCENE_OBSERVER_CREATE_INFO_MSFT};
            CHECK_XRCMD(xrCreateSceneObserverMSFT(m_context.Session.Handle, &createInfo, m_sceneObserver.Put(xrDestroySceneObserverMSFT)));
            m_scanState = ScanState::Idle;
        }

        void Disable() {
            for (auto& [id, visual] : m_markerVisuals) {
                RemoveObject(visual.second);
            }
            m_markerVisuals.clear();
            m_markerIds.clear();
        }

        // Other results from SU might need background processing, but QR Codes should generally be fine
        void UpdateQRCodeVisuals(const XrTime predictedDisplayTime) {
            // xr_msft_scene_marker.h
            // 5. Call xrCreateSceneMSFT to get compute result
            XrSceneCreateInfoMSFT createInfo{XR_TYPE_SCENE_CREATE_INFO_MSFT};
            CHECK_XRCMD(xrCreateSceneMSFT(m_sceneObserver.Get(), &createInfo, m_scene.Put(xrDestroySceneMSFT)));

            XrSceneComponentsGetInfoMSFT getInfo{XR_TYPE_SCENE_COMPONENTS_GET_INFO_MSFT};
            getInfo.componentType = XR_SCENE_COMPONENT_TYPE_MARKER_MSFT;

            std::vector<XrSceneMarkerTypeMSFT> markerTypes{XrSceneMarkerTypeMSFT::XR_SCENE_MARKER_TYPE_QR_CODE_MSFT};
            XrSceneMarkerTypeFilterMSFT typesFilter{XR_TYPE_SCENE_MARKER_TYPE_FILTER_MSFT};
            typesFilter.markerTypeCount = static_cast<uint32_t>(markerTypes.size());
            typesFilter.markerTypes = markerTypes.data();
            xr::InsertExtensionStruct(getInfo, typesFilter);
            XrSceneComponentsMSFT sceneComponents{XR_TYPE_SCENE_COMPONENTS_MSFT};

            // Get the number of markers
            CHECK_XRCMD(xrGetSceneComponentsMSFT(m_scene.Get(), &getInfo, &sceneComponents));
            const uint32_t count = sceneComponents.componentCountOutput;

            std::vector<XrSceneComponentMSFT> components(count);
            sceneComponents.componentCapacityInput = count;
            sceneComponents.components = components.data();

            std::vector<XrSceneMarkerMSFT> markers(count);
            XrSceneMarkersMSFT sceneMarkers{XR_TYPE_SCENE_MARKERS_MSFT};
            sceneMarkers.sceneMarkerCount = count;
            sceneMarkers.sceneMarkers = markers.data();
            xr::InsertExtensionStruct(sceneComponents, sceneMarkers);

            std::vector<XrSceneMarkerQRCodeMSFT> qrcodes(count);
            XrSceneMarkerQRCodesMSFT sceneQRCodes{XR_TYPE_SCENE_MARKER_QR_CODES_MSFT};
            sceneQRCodes.qrCodeCount = count;
            sceneQRCodes.qrCodes = qrcodes.data();
            xr::InsertExtensionStruct(sceneComponents, sceneQRCodes);


            CHECK_XRCMD(xrGetSceneComponentsMSFT(m_scene.Get(), &getInfo, &sceneComponents));

            // Get or create visuals for all QR Codes not too obsolete
            decltype(m_markerVisuals) markerVisuals;
            for (uint32_t i = 0; i < count; ++i) {
                const XrDuration markerAgeNanos = predictedDisplayTime - markers[i].lastSeenTime;
                std::chrono::nanoseconds markerAge = duration<XrDuration, std::nano>(markerAgeNanos);
                if (markerAge < MaxMarkerAge) {
                    auto& markerId = components[i].id;

                    // Always re-create to enable color change and dimension change
                    auto visual = CreateMarkerVisual(
                        markerId,
                        markers[i],
                        qrcodes[i]);
                    XrQuaternionf rotation = xr::math::Quaternion::RotationAxisAngle(XrVector3f{1.0f, 0.0f, 0.0f}, XM_PI);
                    XrVector3f centerOffset{markers[i].center.x, markers[i].center.y, 0.0f};
                    auto centerToPose = xr::math::Pose::MakePose(rotation, centerOffset);
                    markerVisuals[markerId] = std::make_pair(centerToPose, visual);
                    AddObject(visual);
                }
            }
            // Now, remove obsolete visuals
            for (auto& [id, visual] : m_markerVisuals) {
                RemoveObject(visual.second);
            }
            m_markerVisuals = std::move(markerVisuals);

            // Finally, update the parallel vectors
            m_markerIds.clear();
            for (auto& [id, visual] : m_markerVisuals) {
                m_markerIds.push_back(id);
            }
            m_componentLocations.resize(m_markerIds.size());
        }

        // Create a Quad which covers the marker and displays the marker's string as text
        std::shared_ptr<engine::PbrModelObject> CreateMarkerVisual(const XrUuidMSFT& markerId,
                                                                   const XrSceneMarkerMSFT& marker,
                                                                   const XrSceneMarkerQRCodeMSFT& qrcode) {
            uint32_t stringLength{0};
            std::vector<char> markerString;
            CHECK_XRCMD(xrGetSceneMarkerDecodedStringMSFT(m_scene.Get(), &markerId, 0, &stringLength, nullptr));
            markerString.resize(stringLength);
            CHECK_XRCMD(xrGetSceneMarkerDecodedStringMSFT(m_scene.Get(), &markerId, stringLength, &stringLength, markerString.data()));
            const DirectX::XMFLOAT2 size{marker.size.width, marker.size.height};
            const bool isMicroQR =
                (qrcode.symbolType == XrSceneMarkerQRCodeSymbolTypeMSFT::XR_SCENE_MARKER_QR_CODE_SYMBOL_TYPE_MICRO_QR_CODE_MSFT);
            const auto& pbrResources = m_context.PbrResources;

            engine::TextTextureInfo textInfo{256, 256};
            textInfo.Foreground = Pbr::RGBA::White;
            textInfo.Background = Pbr::FromSRGB(Colors::Blue);
            textInfo.Margin = 5;
            textInfo.TextAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
            textInfo.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
            textInfo.FontSize = 32.0f;

            auto textTexture = std::make_unique<engine::TextTexture>(m_context, textInfo);
            textTexture->Draw(markerString.data());
            const auto& material = textTexture->CreatePbrMaterial(m_context.PbrResources);
            return engine::CreateQuad(m_context.PbrResources, size, material);
        }

        // The XrSceneObserver needs to be destroyed after the XrScene.
        xr::UniqueXrHandle<XrSceneObserverMSFT> m_sceneObserver;
        xr::UniqueXrHandle<XrSceneMSFT> m_scene;
        std::vector<XrUuidMSFT> m_markerIds;
        std::vector<XrSceneComponentLocationMSFT> m_componentLocations;
        std::unordered_map<XrUuidMSFT, std::pair<XrPosef, std::shared_ptr<engine::Object>>> m_markerVisuals;
        XrTime m_lastTimeOfUpdate{};
        XrNewSceneComputeInfoMSFT m_sceneComputeInfo{XR_TYPE_NEW_SCENE_COMPUTE_INFO_MSFT};
        XrSceneSphereBoundMSFT m_sphereBounds{0.0f};
        XrSceneFrustumBoundMSFT m_frustumBounds{0.0f};
        XrSceneOrientedBoxBoundMSFT m_boxBounds{0.0f};

        TimePoint m_nextUpdate{};
        ScanState m_scanState{ScanState::Idle};

        // Bounds filtering
        FilteringType m_filteringType;
        bool m_boundsLocated{false};
        XrSpace m_viewSpace;
        std::shared_ptr<engine::Object> m_frustumBoundsTarget;
        std::shared_ptr<engine::Object> m_sphereBoundsTarget;
        std::shared_ptr<engine::Object> m_boxBoundsTarget;

        XrAction m_switchFilterAction{XR_NULL_HANDLE};
    };

} // namespace

std::unique_ptr<engine::Scene> TryCreateQRCodeScene(engine::Context& context) {
    return context.Extensions.SupportsSceneMarker ? std::make_unique<QRCodeScene>(context) : nullptr;
}

