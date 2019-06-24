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
#include "App.h"

namespace {
    using namespace xr;
    using namespace xr::math;

    struct OpenXrProgram : IOpenXrProgram {
        OpenXrProgram(std::string applicationName, std::unique_ptr<IGraphicsPlugin> graphicsPlugin)
            : m_applicationName(std::move(applicationName))
            , m_graphicsPlugin(std::move(graphicsPlugin)) {
        }

        void Run() override {
            CreateInstance();

            bool requestRestart = false;
            do {
                InitializeSystem();
                InitializeSession();

                while (true) {
                    bool exitRenderLoop = false;
                    ProcessEvents(&exitRenderLoop, &requestRestart);
                    if (exitRenderLoop) {
                        break;
                    }

                    if (m_sessionRunning) {
                        PollActions();
                        RenderFrame();
                    } else {
                        // Throttle loop since xrWaitFrame won't be called.
                        using namespace std::chrono_literals;
                        std::this_thread::sleep_for(250ms);
                    }
                }

                if (requestRestart) {
                    PrepareSessionRestart();
                }
            } while (requestRestart);
        }

    private:
        void CreateInstance() {
            CHECK(m_instance.Get() == XR_NULL_HANDLE);

            // Build out the extensions to enable. Some extensions are required and some are optional.
            const std::vector<const char*> enabledExtensions = SelectExtensions();

            // Create the instance with desired extensions.
            XrInstanceCreateInfo createInfo{XR_TYPE_INSTANCE_CREATE_INFO};
            createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
            createInfo.enabledExtensionNames = enabledExtensions.data();

            strcpy_s(createInfo.applicationInfo.applicationName, m_applicationName.c_str());
            createInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
            createInfo.applicationInfo.applicationVersion = 1;
            CHECK_XRCMD(xrCreateInstance(&createInfo, m_instance.Put()));
        }

        std::vector<const char*> SelectExtensions() {
            // Fetch the list of extensions supported by the runtime.
            uint32_t extensionCount;
            CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr));
            std::vector<XrExtensionProperties> extensionProperties(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
            CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()));

            std::vector<const char*> enabledExtensions;

            // Add a specific extension to the list of extensions to be enabled, if it is supported.
            auto AddExtIfSupported = [&](const char* extensionName) {
                for (uint32_t i = 0; i < extensionCount; i++) {
                    if (strcmp(extensionProperties[i].extensionName, extensionName) == 0) {
                        enabledExtensions.push_back(extensionName);
                        return true;
                    }
                }
                return false;
            };

            // D3D11 extension is required so check that it was added.
            CHECK(AddExtIfSupported(XR_KHR_D3D11_ENABLE_EXTENSION_NAME));

            // Additional optional extensions for enhanced functionality. Track whether enabled in m_optionalExtensions.
            m_optionalExtensions.UnboundedRefSpaceSupported = AddExtIfSupported(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
            m_optionalExtensions.SpatialAnchorSupported = AddExtIfSupported(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);

            return enabledExtensions;
        }

        void InitializeSystem() {
            CHECK(m_instance.Get() != XR_NULL_HANDLE);
            CHECK(m_systemId == XR_NULL_SYSTEM_ID);

            XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
            systemInfo.formFactor = m_formFactor;
            while (true) {
                XrResult result = xrGetSystem(m_instance.Get(), &systemInfo, &m_systemId);
                if (SUCCEEDED(result)) {
                    break;
                } else if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE) {
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(250ms);
                } else {
                    CHECK_XRRESULT(result, "xrGetSystem");
                }
            };

            m_environmentBlendMode = SelectEnvironmentBlendMode();
        }

        XrEnvironmentBlendMode SelectEnvironmentBlendMode() {
            CHECK(m_instance.Get() != XR_NULL_HANDLE);
            CHECK(m_systemId != XR_NULL_SYSTEM_ID);

            // Fetch the list of supported environment blend mode of give system
            uint32_t count;
            CHECK_XRCMD(xrEnumerateEnvironmentBlendModes(m_instance.Get(), m_systemId, 0, &count, nullptr));
            std::vector<XrEnvironmentBlendMode> environmentBlendModes(count);
            CHECK_XRCMD(xrEnumerateEnvironmentBlendModes(m_instance.Get(), m_systemId, count, &count, environmentBlendModes.data()));

            auto it = std::find_if(environmentBlendModes.begin(), environmentBlendModes.end(), [](auto&& blendMode) {
                return blendMode == XR_ENVIRONMENT_BLEND_MODE_ADDITIVE || blendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
            });

            CHECK(it != environmentBlendModes.end()); // This system does not have a supported blend mode.
            return *it;
        }

        void InitializeSession() {
            CHECK(m_instance.Get() != XR_NULL_HANDLE);
            CHECK(m_systemId != XR_NULL_SYSTEM_ID);
            CHECK(m_session.Get() == XR_NULL_HANDLE);

            m_graphicsPlugin->InitializeDevice(m_instance.Get(), m_systemId);

            XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
            createInfo.next = m_graphicsPlugin->GetGraphicsBinding();
            createInfo.systemId = m_systemId;
            CHECK_XRCMD(xrCreateSession(m_instance.Get(), &createInfo, m_session.Put()));

            CreateActions();
            CreateSpaces();
            CreateSwapchains();
        }

        void CreateActions() {
            CHECK(m_instance.Get() != XR_NULL_HANDLE);
            CHECK(m_session.Get() != XR_NULL_HANDLE);

            // Create an action set.
            {
                XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
                strcpy_s(actionSetInfo.actionSetName, "place_hologram_action_set");
                strcpy_s(actionSetInfo.localizedActionSetName, "Placement");
                CHECK_XRCMD(xrCreateActionSet(m_session.Get(), &actionSetInfo, m_actionSet.Put()));
            }

            // Create actions.
            {
                // Enable subaction path filtering for left or right hand.
                m_subactionPaths[LeftSide] = GetXrPath("/user/hand/left");
                m_subactionPaths[RightSide] = GetXrPath("/user/hand/right");

                // Create an input action to place a hologram.
                XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
                actionInfo.actionType = XR_INPUT_ACTION_TYPE_BOOLEAN;
                strcpy_s(actionInfo.actionName, "place_hologram");
                strcpy_s(actionInfo.localizedActionName, "Place Hologram");
                actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                actionInfo.subactionPaths = m_subactionPaths.data();
                CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_placeAction.Put()));

                // Create an input action getting the left and right hand poses.
                actionInfo.actionType = XR_INPUT_ACTION_TYPE_POSE;
                strcpy_s(actionInfo.actionName, "hand_pose");
                strcpy_s(actionInfo.localizedActionName, "Hand Pose");
                actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                actionInfo.subactionPaths = m_subactionPaths.data();
                CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_poseAction.Put()));

                // Create output actions for vibrating the left and right controller.
                actionInfo.actionType = XR_OUTPUT_ACTION_TYPE_VIBRATION;
                strcpy_s(actionInfo.actionName, "vibrate");
                strcpy_s(actionInfo.localizedActionName, "Vibrate");
                actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                actionInfo.subactionPaths = m_subactionPaths.data();
                CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_vibrateAction.Put()));
            }

            // Setup suggest bindings for simple controller.
            {
                std::vector<XrActionSuggestedBinding> bindings;
                bindings.push_back({m_placeAction.Get(), GetXrPath("/user/hand/right/input/select/click")});
                bindings.push_back({m_placeAction.Get(), GetXrPath("/user/hand/left/input/select/click")});
                bindings.push_back({m_poseAction.Get(), GetXrPath("/user/hand/right/input/palm/pose")});
                bindings.push_back({m_poseAction.Get(), GetXrPath("/user/hand/left/input/palm/pose")});
                bindings.push_back({m_vibrateAction.Get(), GetXrPath("/user/hand/right/output/haptic")});
                bindings.push_back({m_vibrateAction.Get(), GetXrPath("/user/hand/left/output/haptic")});

                XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
                suggestedBindings.interactionProfile = GetXrPath("/interaction_profiles/khr/simple_controller");
                suggestedBindings.suggestedBindings = bindings.data();
                suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
                CHECK_XRCMD(xrSetInteractionProfileSuggestedBindings(m_session.Get(), &suggestedBindings));
            }
        }

        void CreateSpaces() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);

            // Create a space to place a cube in the world.
            {
                XrReferenceSpaceType referenceSpaceType;

                if (m_optionalExtensions.UnboundedRefSpaceSupported) {
                    // Unbounded reference space provides the best scene space for world-scale experiences.
                    referenceSpaceType = XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT;
                } else {
                    // If running on a platform that does not support world-scale experiences, fall back to local space.
                    referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
                }

                XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                spaceCreateInfo.referenceSpaceType = referenceSpaceType;
                spaceCreateInfo.poseInReferenceSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &spaceCreateInfo, m_sceneSpace.Put()));

                // Initialize the placed cube 1 meter in front of user.
                spaceCreateInfo.poseInReferenceSpace = Pose::Translation({0, 0, -1});
                CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &spaceCreateInfo, m_placedCubeSpace.Put()));
                m_placedCube.Space = m_placedCubeSpace.Get();
                m_placedCube.Scale = {0.1f, 0.1f, 0.1f};
            }

            // Create a space for each hand pointer pose.
            for (uint32_t side : {LeftSide, RightSide}) {
                XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
                createInfo.poseInActionSpace = Pose::Identity();
                createInfo.subactionPath = m_subactionPaths[side];
                CHECK_XRCMD(xrCreateActionSpace(m_poseAction.Get(), &createInfo, m_spacesInHand[side].Put()));
                m_cubesInHand[side].Space = m_spacesInHand[side].Get();
                m_cubesInHand[side].Scale = {0.1f, 0.1f, 0.1f}; // Display a small cube at hand tracking pose.
            }
        }

        void CreateSwapchains() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);
            CHECK(m_swapchains.size() == 0);
            CHECK(m_configViews.empty());

            // Read graphics properties for preferred swapchain length and logging.
            XrSystemProperties systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
            CHECK_XRCMD(xrGetSystemProperties(m_instance.Get(), m_systemId, &systemProperties));

            // Only supports stereo at the moment
            CHECK_MSG(m_primaryViewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, "Unsupported view configuration type");

            // Query and cache view configuration views.
            uint32_t viewCount;
            CHECK_XRCMD(
                xrEnumerateViewConfigurationViews(m_instance.Get(), m_systemId, m_primaryViewConfigurationType, 0, &viewCount, nullptr));
            m_configViews.resize(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
            CHECK_XRCMD(xrEnumerateViewConfigurationViews(
                m_instance.Get(), m_systemId, m_primaryViewConfigurationType, viewCount, &viewCount, m_configViews.data()));

            // Create and cache view buffer for xrLocateViews later.
            m_views.resize(viewCount, {XR_TYPE_VIEW});

            // Create the swapchain and get the images.
            if (viewCount > 0) {
                // Select a swapchain format.
                uint32_t swapchainFormatCount;
                CHECK_XRCMD(xrEnumerateSwapchainFormats(m_session.Get(), 0, &swapchainFormatCount, nullptr));
                std::vector<int64_t> swapchainFormats(swapchainFormatCount);
                CHECK_XRCMD(xrEnumerateSwapchainFormats(
                    m_session.Get(), (uint32_t)swapchainFormats.size(), &swapchainFormatCount, swapchainFormats.data()));
                CHECK(swapchainFormatCount == swapchainFormats.size());
                m_colorSwapchainFormat = m_graphicsPlugin->SelectColorSwapchainFormat(swapchainFormats);

                // Create a swapchain for each view.
                for (uint32_t i = 0; i < viewCount; i++) {
                    const XrViewConfigurationView& vp = m_configViews[i];

                    // Create the swapchain.
                    XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
                    swapchainCreateInfo.arraySize = 1;
                    swapchainCreateInfo.format = m_colorSwapchainFormat;
                    swapchainCreateInfo.width = vp.recommendedImageRectWidth;
                    swapchainCreateInfo.height = vp.recommendedImageRectHeight;
                    swapchainCreateInfo.mipCount = 1;
                    swapchainCreateInfo.faceCount = 1;
                    swapchainCreateInfo.sampleCount = vp.recommendedSwapchainSampleCount;
                    swapchainCreateInfo.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
                    Swapchain swapchain;
                    swapchain.width = swapchainCreateInfo.width;
                    swapchain.height = swapchainCreateInfo.height;
                    CHECK_XRCMD(xrCreateSwapchain(m_session.Get(), &swapchainCreateInfo, &swapchain.handle));

                    m_swapchains.push_back(swapchain);

                    uint32_t imageCount;
                    CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain.handle, 0, &imageCount, nullptr));

                    std::vector<XrSwapchainImageBaseHeader*> swapchainImages =
                        m_graphicsPlugin->AllocateSwapchainImageStructs(imageCount, swapchainCreateInfo);
                    CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain.handle, imageCount, &imageCount, swapchainImages[0]));

                    m_swapchainImages.insert(std::make_pair(swapchain.handle, std::move(swapchainImages)));
                }
            }
        }

        // Return true if an event is available, otherwise return false.
        bool TryReadNextEvent(XrEventDataBuffer* buffer) const {
            // Reset buffer header for every xrPollEvent function call.
            *buffer = {XR_TYPE_EVENT_DATA_BUFFER};
            const XrResult xr = CHECK_XRCMD(xrPollEvent(m_instance.Get(), buffer));
            if (xr == XR_EVENT_UNAVAILABLE) {
                return false;
            } else {
                return true;
            }
        }

        void ProcessEvents(bool* exitRenderLoop, bool* requestRestart) {
            *exitRenderLoop = *requestRestart = false;

            XrEventDataBuffer buffer{XR_TYPE_EVENT_DATA_BUFFER};
            XrEventDataBaseHeader* header = reinterpret_cast<XrEventDataBaseHeader*>(&buffer);

            // Process all pending messages.
            while (TryReadNextEvent(&buffer)) {
                switch (header->type) {
                case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
                    *exitRenderLoop = true;
                    *requestRestart = false;
                    return;
                }
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
                    const auto stateEvent = *reinterpret_cast<const XrEventDataSessionStateChanged*>(header);
                    CHECK(m_session.Get() != XR_NULL_HANDLE && m_session.Get() == stateEvent.session);
                    m_sessionState = stateEvent.state;
                    switch (m_sessionState) {
                    case XR_SESSION_STATE_READY: {
                        CHECK(m_session.Get() != XR_NULL_HANDLE);
                        XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                        sessionBeginInfo.primaryViewConfigurationType = m_primaryViewConfigurationType;
                        CHECK_XRCMD(xrBeginSession(m_session.Get(), &sessionBeginInfo));
                        m_sessionRunning = true;
                        break;
                    }
                    case XR_SESSION_STATE_STOPPING: {
                        m_sessionRunning = false;
                        CHECK_XRCMD(xrEndSession(m_session.Get()))
                        break;
                    }
                    case XR_SESSION_STATE_EXITING: {
                        // Do not attempt to restart because user closed this session.
                        *exitRenderLoop = true;
                        *requestRestart = false;
                        break;
                    }
                    case XR_SESSION_STATE_LOSS_PENDING: {
                        // Poll for a new systemId
                        *exitRenderLoop = true;
                        *requestRestart = true;
                        break;
                    }
                    }
                    break;
                }
                case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
                case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
                default: {
                    DEBUG_PRINT("Ignoring event type %d", header->type);
                    break;
                }
                }
            }
        }

        void PollActions() {
            if (!IsSessionFocused()) {
                return;
            }

            // Get updated action states.
            XrActiveActionSet activeActionSet{XR_TYPE_ACTIVE_ACTION_SET};
            activeActionSet.actionSet = m_actionSet.Get();
            CHECK_XRCMD(xrSyncActionData(m_session.Get(), 1, &activeActionSet));

            // Check the state of the actions for left and right hands separately.
            for (uint32_t side : {LeftSide, RightSide}) {
                const XrPath subactionPath = m_subactionPaths[side];

                XrActionStateBoolean placeActionValue{XR_TYPE_ACTION_STATE_BOOLEAN};
                CHECK_XRCMD(xrGetActionStateBoolean(m_placeAction.Get(), 1, &subactionPath, &placeActionValue));

                if (placeActionValue.changedSinceLastSync && placeActionValue.currentState) {
                    // Apply a tiny vibration to controller to indicate that action is detected.
                    {
                        XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
                        vibration.amplitude = 0.5f;
                        vibration.duration = XR_MIN_HAPTIC_DURATION;
                        vibration.frequency = XR_FREQUENCY_UNSPECIFIED;
                        CHECK_XRCMD(xrApplyHapticFeedback(m_vibrateAction.Get(), 1, &subactionPath, (XrHapticBaseHeader*)&vibration));
                    }

                    // Locate the hand in the scene.
                    const XrSpace handSpace = m_spacesInHand[side].Get();
                    XrSpaceRelation spaceRelation{XR_TYPE_SPACE_RELATION};
                    CHECK_XRCMD(xrLocateSpace(handSpace, m_sceneSpace.Get(), placeActionValue.lastChangeTime, &spaceRelation));

                    // Ensure we have tracking before placing a cube in the scene, so that it stays reliably at a physical location.
                    constexpr XrSpaceRelationFlags PoseValidFlags =
                        XR_SPACE_RELATION_POSITION_TRACKED_BIT | XR_SPACE_RELATION_ORIENTATION_TRACKED_BIT;
                    if ((spaceRelation.relationFlags & PoseValidFlags) != PoseValidFlags) {
                        DEBUG_PRINT("Cube cannot be placed when positional tracking is lost.");
                    } else {
                        if (m_optionalExtensions.SpatialAnchorSupported) {
                            // Anchors provide the best stability when moving beyond 5 meters, so if the extension is enabled,
                            // create an anchor at the hand location and use the resulting anchor space.
                            XrSpatialAnchorCreateInfoMSFT createInfo{XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT};
                            createInfo.space = m_sceneSpace.Get();
                            createInfo.pose = spaceRelation.pose;
                            createInfo.time = placeActionValue.lastChangeTime;
                            XrSpatialAnchorHandle spatialAnchor;
                            XrResult r = xrCreateSpatialAnchorMSFT(m_session.Get(), &createInfo, spatialAnchor.Put());
                            if (XR_SUCCEEDED(r)) {
                                CHECK_XRCMD(xrCreateSpatialAnchorSpaceMSFT(m_session.Get(), spatialAnchor.Get(), m_placedCubeSpace.Put()));
                                m_spatialAnchor = std::move(spatialAnchor);
                            } else if (r != XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT) {
                                CHECK_XRRESULT(r, "xrCreateSpatialAnchorSpaceMSFT");
                            }
                        } else {
                            // If the anchor extension is not available, create a local space with an origin at the hand location.
                            XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
                            createInfo.poseInReferenceSpace = spaceRelation.pose;
                            CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &createInfo, m_placedCubeSpace.Put()));
                        }

                        // Place the cube at new space.
                        m_placedCube.Space = m_placedCubeSpace.Get();
                    }
                }
            }
        }

        void RenderFrame() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);

            XrFrameWaitInfo frameWaitInfo{XR_TYPE_FRAME_WAIT_INFO};
            XrFrameState frameState{XR_TYPE_FRAME_STATE};
            CHECK_XRCMD(xrWaitFrame(m_session.Get(), &frameWaitInfo, &frameState));

            XrFrameBeginInfo frameBeginInfo{XR_TYPE_FRAME_BEGIN_INFO};
            CHECK_XRCMD(xrBeginFrame(m_session.Get(), &frameBeginInfo));

            // EndFrame can submit mutiple layers
            std::vector<XrCompositionLayerBaseHeader*> layers;

            // The projection layer consists of projection layer views.
            XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
            std::vector<XrCompositionLayerProjectionView> projectionLayerViews;

            // Only render when session is visible. otherwise submit zero layers
            if (IsSessionVisible()) {
                if (RenderLayer(frameState.predictedDisplayTime, layer, projectionLayerViews)) {
                    layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&layer));
                }
            }

            // Submit the composition layers for the predicted display time.
            XrFrameEndInfo frameEndInfo{XR_TYPE_FRAME_END_INFO};
            frameEndInfo.displayTime = frameState.predictedDisplayTime;
            frameEndInfo.environmentBlendMode = m_environmentBlendMode;
            frameEndInfo.layerCount = (uint32_t)layers.size();
            frameEndInfo.layers = layers.data();
            CHECK_XRCMD(xrEndFrame(m_session.Get(), &frameEndInfo));
        }

        bool RenderLayer(XrTime predictedDisplayTime,
                         XrCompositionLayerProjection& layer,
                         std::vector<XrCompositionLayerProjectionView>& projectionLayerViews) {
            XrViewState viewState{XR_TYPE_VIEW_STATE};
            uint32_t viewCapacityInput = (uint32_t)m_views.size();
            uint32_t viewCountOutput;

            XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
            viewLocateInfo.displayTime = predictedDisplayTime;
            viewLocateInfo.space = m_sceneSpace.Get();
            CHECK_XRCMD(xrLocateViews(m_session.Get(), &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, m_views.data()));
            CHECK(viewCountOutput == viewCapacityInput);
            CHECK(viewCountOutput == m_configViews.size());
            CHECK(viewCountOutput == m_swapchains.size());

            constexpr XrViewStateFlags viewPoseValidFlags = XR_VIEW_STATE_POSITION_VALID_BIT | XR_VIEW_STATE_ORIENTATION_VALID_BIT;
            if ((viewState.viewStateFlags & viewPoseValidFlags) != viewPoseValidFlags) {
                DEBUG_PRINT("xrLocateViews returned an invalid pose.");
                return false;
            }

            std::vector<Cube> visibleCubes;

            // Update cubes location with latest space relation
            for (auto cube : {m_placedCube, m_cubesInHand[LeftSide], m_cubesInHand[RightSide]}) {
                if (cube.Space != XR_NULL_HANDLE) {
                    XrSpaceRelation spaceRelation{XR_TYPE_SPACE_RELATION};
                    CHECK_XRCMD(xrLocateSpace(cube.Space, m_sceneSpace.Get(), predictedDisplayTime, &spaceRelation));

                    constexpr XrViewStateFlags poseValidFlags =
                        XR_SPACE_RELATION_POSITION_VALID_BIT | XR_SPACE_RELATION_ORIENTATION_VALID_BIT;
                    if ((spaceRelation.relationFlags & poseValidFlags) == poseValidFlags) {
                        cube.Pose = spaceRelation.pose;
                        visibleCubes.push_back(cube);
                    }
                }
            }

            projectionLayerViews.resize(viewCountOutput);

            // Render view to the appropriate part of the swapchain image.
            for (uint32_t i = 0; i < viewCountOutput; i++) {
                // Each view has a separate swapchain which is acquired, rendered to, and released.
                const Swapchain viewSwapchain = m_swapchains[i];

                uint32_t swapchainImageIndex;
                XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
                CHECK_XRCMD(xrAcquireSwapchainImage(viewSwapchain.handle, &acquireInfo, &swapchainImageIndex));

                XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
                waitInfo.timeout = XR_INFINITE_DURATION;
                CHECK_XRCMD(xrWaitSwapchainImage(viewSwapchain.handle, &waitInfo));

                projectionLayerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
                projectionLayerViews[i].pose = m_views[i].pose;
                projectionLayerViews[i].fov = m_views[i].fov;
                projectionLayerViews[i].subImage.swapchain = viewSwapchain.handle;
                projectionLayerViews[i].subImage.imageRect.offset = {0, 0};
                projectionLayerViews[i].subImage.imageRect.extent = {viewSwapchain.width, viewSwapchain.height};

                const XrSwapchainImageBaseHeader* const swapchainImage = m_swapchainImages[viewSwapchain.handle][swapchainImageIndex];
                m_graphicsPlugin->RenderView(
                    projectionLayerViews[i], swapchainImage, m_environmentBlendMode, m_colorSwapchainFormat, visibleCubes);

                XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
                CHECK_XRCMD(xrReleaseSwapchainImage(viewSwapchain.handle, &releaseInfo));
            }

            layer.space = m_sceneSpace.Get();
            layer.viewCount = (uint32_t)projectionLayerViews.size();
            layer.views = projectionLayerViews.data();
            return true;
        }

        void PrepareSessionRestart() {
            m_configViews.clear();
            m_swapchains.clear();
            m_session.Reset();
            m_systemId = XR_NULL_SYSTEM_ID;
        }

        constexpr bool IsSessionVisible() const {
            return m_sessionState == XR_SESSION_STATE_VISIBLE || m_sessionState == XR_SESSION_STATE_FOCUSED;
        }

        constexpr bool IsSessionFocused() const {
            return m_sessionState == XR_SESSION_STATE_FOCUSED;
        }

        XrPath GetXrPath(std::string_view string) const {
            XrPath path;
            CHECK_XRCMD(xrStringToPath(m_instance.Get(), string.data(), &path));
            return path;
        }

    private:
        const XrFormFactor m_formFactor{XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY};
        const XrViewConfigurationType m_primaryViewConfigurationType{XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO};

        const std::string m_applicationName;
        const std::unique_ptr<IGraphicsPlugin> m_graphicsPlugin;

        XrInstanceHandle m_instance;
        XrSessionHandle m_session;
        uint64_t m_systemId{XR_NULL_SYSTEM_ID};

        struct {
            bool UnboundedRefSpaceSupported{false};
            bool SpatialAnchorSupported{false};
        } m_optionalExtensions;

        XrSpaceHandle m_sceneSpace;

        XrSpatialAnchorHandle m_spatialAnchor;
        XrSpaceHandle m_placedCubeSpace;
        Cube m_placedCube; // Placed in local or anchor space.

        constexpr static uint32_t LeftSide = 0;
        constexpr static uint32_t RightSide = 1;
        std::array<XrPath, 2> m_subactionPaths{};
        std::array<XrSpaceHandle, 2> m_spacesInHand{};
        std::array<Cube, 2> m_cubesInHand{};

        XrActionSetHandle m_actionSet;
        XrActionHandle m_placeAction;
        XrActionHandle m_poseAction;
        XrActionHandle m_vibrateAction;

        struct Swapchain {
            XrSwapchain handle;
            int32_t width;
            int32_t height;
        };

        std::vector<XrViewConfigurationView> m_configViews;
        std::vector<Swapchain> m_swapchains;
        std::unordered_map<XrSwapchain, std::vector<XrSwapchainImageBaseHeader*>> m_swapchainImages;
        std::vector<XrView> m_views;
        int64_t m_colorSwapchainFormat{-1};

        bool m_sessionRunning{false};
        XrSessionState m_sessionState{XR_SESSION_STATE_UNKNOWN};
        XrEnvironmentBlendMode m_environmentBlendMode{};
    };
} // namespace

namespace xr {
    std::unique_ptr<IOpenXrProgram> CreateOpenXrProgram(std::string applicationName, std::unique_ptr<IGraphicsPlugin> graphicsPlugin) {
        return std::make_unique<OpenXrProgram>(std::move(applicationName), std::move(graphicsPlugin));
    }
} // namespace xr
