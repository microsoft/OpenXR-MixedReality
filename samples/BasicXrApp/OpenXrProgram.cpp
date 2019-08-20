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
    struct ImplementOpenXrProgram : xr::sample::IOpenXrProgram {
        ImplementOpenXrProgram(std::string applicationName, std::unique_ptr<xr::sample::IGraphicsPluginD3D11> graphicsPlugin)
            : m_applicationName(std::move(applicationName))
            , m_graphicsPlugin(std::move(graphicsPlugin)) {
        }

        void Run() override {
            CreateInstance();
            CreateActions();

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

            createInfo.applicationInfo = {"", 1, "OpenXR Sample", 1, XR_CURRENT_API_VERSION};
            strcpy_s(createInfo.applicationInfo.applicationName, m_applicationName.c_str());
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
            auto EnableExtentionIfSupported = [&](const char* extensionName) {
                for (uint32_t i = 0; i < extensionCount; i++) {
                    if (strcmp(extensionProperties[i].extensionName, extensionName) == 0) {
                        enabledExtensions.push_back(extensionName);
                        return true;
                    }
                }
                return false;
            };

            // D3D11 extension is required for this sample, so check if it's supported.
            CHECK(EnableExtentionIfSupported(XR_KHR_D3D11_ENABLE_EXTENSION_NAME));

            // Additional optional extensions for enhanced functionality. Track whether enabled in m_optionalExtensions.
            m_optionalExtensions.DepthExtensionSupported = EnableExtentionIfSupported(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
            m_optionalExtensions.UnboundedRefSpaceSupported = EnableExtentionIfSupported(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
            m_optionalExtensions.SpatialAnchorSupported = EnableExtentionIfSupported(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);

            return enabledExtensions;
        }

        void CreateActions() {
            CHECK(m_instance.Get() != XR_NULL_HANDLE);

            // Create an action set.
            {
                XrActionSetCreateInfo actionSetInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
                strcpy_s(actionSetInfo.actionSetName, "place_hologram_action_set");
                strcpy_s(actionSetInfo.localizedActionSetName, "Placement");
                CHECK_XRCMD(xrCreateActionSet(m_instance.Get(), &actionSetInfo, m_actionSet.Put()));
            }

            // Create actions.
            {
                // Enable subaction path filtering for left or right hand.
                m_subactionPaths[LeftSide] = GetXrPath("/user/hand/left");
                m_subactionPaths[RightSide] = GetXrPath("/user/hand/right");

                // Create an input action to place a hologram.
                {
                    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
                    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
                    strcpy_s(actionInfo.actionName, "place_hologram");
                    strcpy_s(actionInfo.localizedActionName, "Place Hologram");
                    actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                    actionInfo.subactionPaths = m_subactionPaths.data();
                    CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_placeAction.Put()));
                }

                // Create an input action getting the left and right hand poses.
                {
                    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
                    actionInfo.actionType = XR_ACTION_TYPE_POSE_INPUT;
                    strcpy_s(actionInfo.actionName, "hand_pose");
                    strcpy_s(actionInfo.localizedActionName, "Hand Pose");
                    actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                    actionInfo.subactionPaths = m_subactionPaths.data();
                    CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_poseAction.Put()));
                }

                // Create an output action for vibrating the left and right controller.
                {
                    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
                    actionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
                    strcpy_s(actionInfo.actionName, "vibrate");
                    strcpy_s(actionInfo.localizedActionName, "Vibrate");
                    actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                    actionInfo.subactionPaths = m_subactionPaths.data();
                    CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_vibrateAction.Put()));
                }

                // Create an input action to exit session
                {
                    XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
                    actionInfo.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
                    strcpy_s(actionInfo.actionName, "exit_session");
                    strcpy_s(actionInfo.localizedActionName, "Exit session");
                    actionInfo.countSubactionPaths = (uint32_t)m_subactionPaths.size();
                    actionInfo.subactionPaths = m_subactionPaths.data();
                    CHECK_XRCMD(xrCreateAction(m_actionSet.Get(), &actionInfo, m_exitAction.Put()));
                }
            }

            // Setup suggest bindings for simple controller.
            {
                std::vector<XrActionSuggestedBinding> bindings;
                bindings.push_back({m_placeAction.Get(), GetXrPath("/user/hand/right/input/select/click")});
                bindings.push_back({m_placeAction.Get(), GetXrPath("/user/hand/left/input/select/click")});
                bindings.push_back({m_poseAction.Get(), GetXrPath("/user/hand/right/input/grip/pose")});
                bindings.push_back({m_poseAction.Get(), GetXrPath("/user/hand/left/input/grip/pose")});
                bindings.push_back({m_vibrateAction.Get(), GetXrPath("/user/hand/right/output/haptic")});
                bindings.push_back({m_vibrateAction.Get(), GetXrPath("/user/hand/left/output/haptic")});
                bindings.push_back({m_exitAction.Get(), GetXrPath("/user/hand/right/input/menu/click")});
                bindings.push_back({m_exitAction.Get(), GetXrPath("/user/hand/left/input/menu/click")});

                XrInteractionProfileSuggestedBinding suggestedBindings{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
                suggestedBindings.interactionProfile = GetXrPath("/interaction_profiles/khr/simple_controller");
                suggestedBindings.suggestedBindings = bindings.data();
                suggestedBindings.countSuggestedBindings = (uint32_t)bindings.size();
                CHECK_XRCMD(xrSuggestInteractionProfileBindings(m_instance.Get(), &suggestedBindings));
            }
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
                    DEBUG_PRINT("No headset detected.  Trying again in one second...");
                    using namespace std::chrono_literals;
                    std::this_thread::sleep_for(1s);
                } else {
                    CHECK_XRRESULT(result, "xrGetSystem");
                }
            };

            // Choose an environment blend mode.
            {
                // Query the list of supported environment blend modes for the current system
                uint32_t count;
                CHECK_XRCMD(xrEnumerateEnvironmentBlendModes(m_instance.Get(), m_systemId, m_primaryViewConfigType, 0, &count, nullptr));
                CHECK(count > 0); // A system must support at least one environment blend mode.

                std::vector<XrEnvironmentBlendMode> environmentBlendModes(count);
                CHECK_XRCMD(xrEnumerateEnvironmentBlendModes(
                    m_instance.Get(), m_systemId, m_primaryViewConfigType, count, &count, environmentBlendModes.data()));

                // This sample supports all modes, pick the system's preferred one.
                m_environmentBlendMode = environmentBlendModes[0];
            }

            // Choose a reasonable depth range can help improve hologram visual quality.
            m_nearFar = {0.1f, 20.f};
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

            XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
            std::vector<XrActionSet> actionSets = {m_actionSet.Get()};
            attachInfo.countActionSets = (uint32_t)actionSets.size();
            attachInfo.actionSets = actionSets.data();
            CHECK_XRCMD(xrAttachSessionActionSets(m_session.Get(), &attachInfo));

            CreateSpaces();
            CreateSwapchains();
        }

        void CreateSpaces() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);

            // Create a space to place a cube in the world.
            {
                if (m_optionalExtensions.UnboundedRefSpaceSupported) {
                    // Unbounded reference space provides the best scene space for world-scale experiences.
                    m_sceneSpaceType = XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT;
                } else {
                    // If running on a platform that does not support world-scale experiences, fall back to local space.
                    m_sceneSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
                }

                XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                spaceCreateInfo.referenceSpaceType = m_sceneSpaceType;
                spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &spaceCreateInfo, m_sceneSpace.Put()));

                // Initialize the placed cube 1 meter in front of user.
                spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Translation({0, 0, -1});
                CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &spaceCreateInfo, m_placedCubeSpace.Put()));
                m_placedCube.Space = m_placedCubeSpace.Get();
                m_placedCube.Scale = {0.1f, 0.1f, 0.1f};
            }

            // Create a space for each hand pointer pose.
            for (uint32_t side : {LeftSide, RightSide}) {
                XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
                createInfo.action = m_poseAction.Get();
                createInfo.poseInActionSpace = xr::math::Pose::Identity();
                createInfo.subactionPath = m_subactionPaths[side];
                CHECK_XRCMD(xrCreateActionSpace(m_session.Get(), &createInfo, m_spacesInHand[side].Put()));
                m_cubesInHand[side].Space = m_spacesInHand[side].Get();
                m_cubesInHand[side].Scale = {0.1f, 0.1f, 0.1f}; // Display a small cube at hand tracking pose.
            }
        }

        struct SwapchainData;
        std::unique_ptr<SwapchainData> CreateSwapchainData(const XrViewConfigurationView& view,
                                                           uint64_t format,
                                                           XrSwapchainUsageFlags usageFlags) {
            XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
            swapchainCreateInfo.arraySize = 1;
            swapchainCreateInfo.format = format;
            swapchainCreateInfo.width = view.recommendedImageRectWidth;
            swapchainCreateInfo.height = view.recommendedImageRectHeight;
            swapchainCreateInfo.mipCount = 1;
            swapchainCreateInfo.faceCount = 1;
            swapchainCreateInfo.sampleCount = view.recommendedSwapchainSampleCount;
            swapchainCreateInfo.usageFlags = usageFlags;

            auto swapchainData = std::make_unique<SwapchainData>();
            swapchainData->width = swapchainCreateInfo.width;
            swapchainData->height = swapchainCreateInfo.height;

            CHECK_XRCMD(xrCreateSwapchain(m_session.Get(), &swapchainCreateInfo, &swapchainData->handle));

            uint32_t chainLength;
            CHECK_XRCMD(xrEnumerateSwapchainImages(swapchainData->handle, 0, &chainLength, nullptr));

            swapchainData->images.resize(chainLength, {XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
            CHECK_XRCMD(xrEnumerateSwapchainImages(swapchainData->handle,
                                                   (uint32_t)swapchainData->images.size(),
                                                   &chainLength,
                                                   reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainData->images.data())));
            return swapchainData;
        }

        void SelectSwapchainPixelFormats() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);

            // Query runtime preferred swapchain formats.
            uint32_t swapchainFormatCount;
            CHECK_XRCMD(xrEnumerateSwapchainFormats(m_session.Get(), 0, &swapchainFormatCount, nullptr));

            std::vector<int64_t> swapchainFormats(swapchainFormatCount);
            CHECK_XRCMD(xrEnumerateSwapchainFormats(
                m_session.Get(), (uint32_t)swapchainFormats.size(), &swapchainFormatCount, swapchainFormats.data()));

            // Choose the first runtime preferred format that this app supports.
            auto SelectPixelFormat = [](const std::vector<int64_t>& runtimePreferredFormats,
                                        const std::vector<DXGI_FORMAT>& applicationSupportedFormats) {
                auto found = std::find_first_of(std::begin(runtimePreferredFormats),
                                                std::end(runtimePreferredFormats),
                                                std::begin(applicationSupportedFormats),
                                                std::end(applicationSupportedFormats));
                if (found == std::end(runtimePreferredFormats)) {
                    THROW("No runtime swapchain format is supported.");
                }
                return (DXGI_FORMAT)*found;
            };

            m_colorSwapchainFormat = SelectPixelFormat(swapchainFormats, m_graphicsPlugin->SupportedColorFormats());
            m_depthSwapchainFormat = SelectPixelFormat(swapchainFormats, m_graphicsPlugin->SupportedDepthFormats());
        }

        void CreateSwapchains() {
            CHECK(m_session.Get() != XR_NULL_HANDLE);
            CHECK(m_renderResources == nullptr);

            m_renderResources = std::make_unique<RenderResources>();

            // Read graphics properties for preferred swapchain length and logging.
            XrSystemProperties systemProperties{XR_TYPE_SYSTEM_PROPERTIES};
            CHECK_XRCMD(xrGetSystemProperties(m_instance.Get(), m_systemId, &systemProperties));

            // Query and cache view configuration views.
            uint32_t viewCount;
            CHECK_XRCMD(xrEnumerateViewConfigurationViews(m_instance.Get(), m_systemId, m_primaryViewConfigType, 0, &viewCount, nullptr));
            CHECK(viewCount > 0); // A system must support at least one view to render into.

            m_renderResources->ConfigViews.resize(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
            CHECK_XRCMD(xrEnumerateViewConfigurationViews(
                m_instance.Get(), m_systemId, m_primaryViewConfigType, viewCount, &viewCount, m_renderResources->ConfigViews.data()));

            // Preallocate view buffers for xrLocateViews later inside frame loop.
            m_renderResources->Views.resize(viewCount, {XR_TYPE_VIEW});

            // Select color and depth swapchain pixel formats
            SelectSwapchainPixelFormats();

            // Create a swapchain for each view and get the images.
            for (uint32_t i = 0; i < viewCount; i++) {
                const XrViewConfigurationView& view = m_renderResources->ConfigViews[i];

                std::unique_ptr<SwapchainData> colorSwapchainData = CreateSwapchainData(
                    view, m_colorSwapchainFormat, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT);
                m_renderResources->ColorSwapchains.push_back(std::move(colorSwapchainData));

                std::unique_ptr<SwapchainData> depthSwapchainData = CreateSwapchainData(
                    view, m_depthSwapchainFormat, XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
                m_renderResources->DepthSwapchains.push_back(std::move(depthSwapchainData));
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
                        sessionBeginInfo.primaryViewConfigurationType = m_primaryViewConfigType;
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

        void PlaceHologramInScene(const XrSpaceLocation& location,
                                  XrTime placementTime,
                                  XrSpace* placementSpace,
                                  XrSpatialAnchorMSFT* placementAnchor) const {
            if (m_optionalExtensions.SpatialAnchorSupported) {
                // Anchors provide the best stability when moving beyond 5 meters, so if the extension is enabled,
                // create an anchor at the hand location and use the resulting anchor space.
                XrSpatialAnchorCreateInfoMSFT createInfo{XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_MSFT};
                createInfo.space = m_sceneSpace.Get();
                createInfo.pose = location.pose;
                createInfo.time = placementTime;

                XrResult r = xrCreateSpatialAnchorMSFT(m_session.Get(), &createInfo, placementAnchor);
                if (r == XR_ERROR_CREATE_SPATIAL_ANCHOR_FAILED_MSFT) {
                    DEBUG_PRINT("Anchor cannot be created, likely due to lost positional tracking.");
                } else if (XR_SUCCEEDED(r)) {
                    XrSpatialAnchorSpaceCreateInfoMSFT createSpaceInfo{XR_TYPE_SPATIAL_ANCHOR_SPACE_CREATE_INFO_MSFT};
                    createSpaceInfo.anchor = *placementAnchor;
                    createSpaceInfo.poseInAnchorSpace = xr::math::Pose::Identity();
                    CHECK_XRCMD(xrCreateSpatialAnchorSpaceMSFT(m_session.Get(), &createSpaceInfo, placementSpace));
                } else {
                    CHECK_XRRESULT(r, "xrCreateSpatialAnchorMSFT");
                }
            } else {
                // If the anchor extension is not available, place it in the scene space.
                XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                createInfo.referenceSpaceType = m_sceneSpaceType;
                createInfo.poseInReferenceSpace = location.pose;
                CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &createInfo, placementSpace));
            }
        }

        void PollActions() {
            if (!IsSessionFocused()) {
                return;
            }

            // Get updated action states.
            std::vector<XrActiveActionSet> activeActionSets = {{m_actionSet.Get(), XR_NULL_PATH}};
            XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
            syncInfo.countActiveActionSets = (uint32_t)activeActionSets.size();
            syncInfo.activeActionSets = activeActionSets.data();
            CHECK_XRCMD(xrSyncActions(m_session.Get(), &syncInfo));

            // Check the state of the actions for left and right hands separately.
            for (uint32_t side : {LeftSide, RightSide}) {
                const XrPath subactionPath = m_subactionPaths[side];

                // Apply a tiny vibration to the corresponding hand to indicate that action is detected.
                auto ApplyVibration = [this, subactionPath] {
                    XrHapticActionInfo actionInfo{XR_TYPE_HAPTIC_ACTION_INFO};
                    actionInfo.action = m_vibrateAction.Get();
                    actionInfo.subactionPath = subactionPath;

                    XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
                    vibration.amplitude = 0.5f;
                    vibration.duration = XR_MIN_HAPTIC_DURATION;
                    vibration.frequency = XR_FREQUENCY_UNSPECIFIED;
                    CHECK_XRCMD(xrApplyHapticFeedback(m_session.Get(), &actionInfo, (XrHapticBaseHeader*)&vibration));
                };

                XrActionStateBoolean placeActionValue{XR_TYPE_ACTION_STATE_BOOLEAN};
                {
                    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                    getInfo.action = m_placeAction.Get();
                    getInfo.subactionPath = subactionPath;
                    CHECK_XRCMD(xrGetActionStateBoolean(m_session.Get(), &getInfo, &placeActionValue));
                }

                // When select button is pressed, place the cube at the location of corresponding hand.
                if (placeActionValue.isActive && placeActionValue.changedSinceLastSync && placeActionValue.currentState) {
                    // Use the poses at the time when action happened to do the placement
                    const XrTime placementTime = placeActionValue.lastChangeTime;

                    // Locate the hand in the scene.
                    XrSpaceLocation handLocation{XR_TYPE_SPACE_LOCATION};
                    {
                        const XrSpace handSpace = m_spacesInHand[side].Get();
                        CHECK_XRCMD(xrLocateSpace(handSpace, m_sceneSpace.Get(), placementTime, &handLocation));
                    }

                    // Ensure we have tracking before placing a cube in the scene, so that it stays reliably at a physical location.
                    if (!xr::math::Pose::IsPoseValid(handLocation)) {
                        DEBUG_PRINT("Cube cannot be placed when positional tracking is lost.");
                    } else {
                        // Place the cube at the given location and time, and remember output placement space and anchor.
                        PlaceHologramInScene(handLocation, placementTime, m_placedCubeSpace.Put(), m_placedCubeAnchor.Put());
                        m_placedCube.Space = m_placedCubeSpace.Get();
                    }

                    ApplyVibration();
                }

                XrActionStateBoolean exitActionValue{XR_TYPE_ACTION_STATE_BOOLEAN};
                {
                    XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                    getInfo.action = m_exitAction.Get();
                    getInfo.subactionPath = subactionPath;
                    CHECK_XRCMD(xrGetActionStateBoolean(m_session.Get(), &getInfo, &exitActionValue));
                }

                // When menu button is released, request to quit the session, and therefore quit the application.
                if (exitActionValue.isActive && exitActionValue.changedSinceLastSync && !exitActionValue.currentState) {
                    CHECK_XRCMD(xrRequestExitSession(m_session.Get()));
                    ApplyVibration();
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

            // Inform the runtime to consider alpha channel during composition
            layer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;

            // Only render when session is visible. otherwise submit zero layers
            if (IsSessionVisible()) {
                if (RenderLayer(frameState.predictedDisplayTime, layer)) {
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

        uint32_t AquireAndWaitForSwapchainImage(XrSwapchain handle) {
            uint32_t swapchainImageIndex;
            XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
            CHECK_XRCMD(xrAcquireSwapchainImage(handle, &acquireInfo, &swapchainImageIndex));

            XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
            waitInfo.timeout = XR_INFINITE_DURATION;
            CHECK_XRCMD(xrWaitSwapchainImage(handle, &waitInfo));

            return swapchainImageIndex;
        }

        bool RenderLayer(XrTime predictedDisplayTime, XrCompositionLayerProjection& layer) {
            XrViewState viewState{XR_TYPE_VIEW_STATE};
            std::vector<XrView>& views = m_renderResources->Views;
            uint32_t viewCapacityInput = (uint32_t)views.size();
            uint32_t viewCountOutput;

            XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
            viewLocateInfo.viewConfigurationType = m_primaryViewConfigType;
            viewLocateInfo.displayTime = predictedDisplayTime;
            viewLocateInfo.space = m_sceneSpace.Get();
            CHECK_XRCMD(xrLocateViews(m_session.Get(), &viewLocateInfo, &viewState, viewCapacityInput, &viewCountOutput, views.data()));
            CHECK(viewCountOutput == viewCapacityInput);
            CHECK(viewCountOutput == m_renderResources->ConfigViews.size());
            CHECK(viewCountOutput == m_renderResources->ColorSwapchains.size());
            CHECK(viewCountOutput == m_renderResources->DepthSwapchains.size());

            if (!xr::math::Pose::IsPoseValid(viewState)) {
                DEBUG_PRINT("xrLocateViews returned an invalid pose.");
                return false;
            }

            std::vector<xr::sample::Cube> visibleCubes;

            // Update cubes location with latest space relation
            for (auto cube : {m_placedCube, m_cubesInHand[LeftSide], m_cubesInHand[RightSide]}) {
                if (cube.Space != XR_NULL_HANDLE) {
                    XrSpaceLocation spaceLocation{XR_TYPE_SPACE_LOCATION};
                    CHECK_XRCMD(xrLocateSpace(cube.Space, m_sceneSpace.Get(), predictedDisplayTime, &spaceLocation));

                    if (xr::math::Pose::IsPoseValid(spaceLocation)) {
                        cube.Pose = spaceLocation.pose;
                        visibleCubes.push_back(cube);
                    }
                }
            }

            m_renderResources->ProjectionLayerViews.resize(viewCountOutput);
            if (m_optionalExtensions.DepthExtensionSupported) {
                m_renderResources->DepthInfoViews.resize(viewCountOutput);
            }

            // Render view to the appropriate part of the swapchain image.
            for (uint32_t i = 0; i < viewCountOutput; i++) {
                // Each view has a separate swapchain which is acquired, rendered to, and released.
                const SwapchainData& colorSwapchain = *m_renderResources->ColorSwapchains[i];
                const SwapchainData& depthSwapchain = *m_renderResources->DepthSwapchains[i];

                const uint32_t colorSwapchainImageIndex = AquireAndWaitForSwapchainImage(colorSwapchain.handle);
                const uint32_t depthSwapchainImageIndex = AquireAndWaitForSwapchainImage(depthSwapchain.handle);

                m_renderResources->ProjectionLayerViews[i] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
                m_renderResources->ProjectionLayerViews[i].pose = m_renderResources->Views[i].pose;
                m_renderResources->ProjectionLayerViews[i].fov = m_renderResources->Views[i].fov;
                m_renderResources->ProjectionLayerViews[i].subImage.swapchain = colorSwapchain.handle;
                m_renderResources->ProjectionLayerViews[i].subImage.imageRect.offset = {0, 0};
                m_renderResources->ProjectionLayerViews[i].subImage.imageRect.extent = {colorSwapchain.width, colorSwapchain.height};
                m_renderResources->ProjectionLayerViews[i].subImage.imageArrayIndex = 0;

                if (m_optionalExtensions.DepthExtensionSupported) {
                    m_renderResources->DepthInfoViews[i] = {XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR};
                    m_renderResources->DepthInfoViews[i].minDepth = 0;
                    m_renderResources->DepthInfoViews[i].maxDepth = 1;
                    m_renderResources->DepthInfoViews[i].nearZ = m_nearFar.Near;
                    m_renderResources->DepthInfoViews[i].farZ = m_nearFar.Far;
                    m_renderResources->DepthInfoViews[i].subImage.swapchain = depthSwapchain.handle;
                    m_renderResources->DepthInfoViews[i].subImage.imageRect.offset = {0, 0};
                    m_renderResources->DepthInfoViews[i].subImage.imageRect.extent = {depthSwapchain.width, depthSwapchain.height};
                    m_renderResources->DepthInfoViews[i].subImage.imageArrayIndex = 0;

                    // Chain depth info struct to the corresponding projection layer views's next
                    m_renderResources->ProjectionLayerViews[i].next = &m_renderResources->DepthInfoViews[i];
                }

                m_graphicsPlugin->RenderView(m_renderResources->ProjectionLayerViews[i],
                                             m_colorSwapchainFormat,
                                             colorSwapchain.images[colorSwapchainImageIndex],
                                             m_depthSwapchainFormat,
                                             depthSwapchain.images[depthSwapchainImageIndex],
                                             m_environmentBlendMode,
                                             m_nearFar,
                                             visibleCubes);

                XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
                CHECK_XRCMD(xrReleaseSwapchainImage(colorSwapchain.handle, &releaseInfo));
                CHECK_XRCMD(xrReleaseSwapchainImage(depthSwapchain.handle, &releaseInfo));
            }

            layer.space = m_sceneSpace.Get();
            layer.viewCount = (uint32_t)m_renderResources->ProjectionLayerViews.size();
            layer.views = m_renderResources->ProjectionLayerViews.data();
            return true;
        }

        void PrepareSessionRestart() {
            m_renderResources.reset();
            m_session.Reset();
            m_systemId = XR_NULL_SYSTEM_ID;
        }

        constexpr bool IsSessionVisible() const {
            return m_sessionState == XR_SESSION_STATE_VISIBLE || m_sessionState == XR_SESSION_STATE_FOCUSED;
        }

        constexpr bool IsSessionFocused() const {
            return m_sessionState == XR_SESSION_STATE_FOCUSED;
        }

        XrPath GetXrPath(const char* string) const {
            return xr::StringToPath(m_instance.Get(), string);
        }

    private:
        const XrFormFactor m_formFactor{XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY};
        const XrViewConfigurationType m_primaryViewConfigType{XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO};

        const std::string m_applicationName;
        const std::unique_ptr<xr::sample::IGraphicsPluginD3D11> m_graphicsPlugin;

        xr::InstanceHandle m_instance;
        xr::SessionHandle m_session;
        uint64_t m_systemId{XR_NULL_SYSTEM_ID};

        struct {
            bool DepthExtensionSupported{false};
            bool UnboundedRefSpaceSupported{false};
            bool SpatialAnchorSupported{false};
        } m_optionalExtensions;

        xr::SpaceHandle m_sceneSpace;
        XrReferenceSpaceType m_sceneSpaceType{};

        xr::SpatialAnchorHandle m_placedCubeAnchor;
        xr::SpaceHandle m_placedCubeSpace;
        xr::sample::Cube m_placedCube; // Placed in local or anchor space.

        constexpr static uint32_t LeftSide = 0;
        constexpr static uint32_t RightSide = 1;
        std::array<XrPath, 2> m_subactionPaths{};
        std::array<xr::SpaceHandle, 2> m_spacesInHand{};
        std::array<xr::sample::Cube, 2> m_cubesInHand{};

        xr::ActionSetHandle m_actionSet;
        xr::ActionHandle m_placeAction;
        xr::ActionHandle m_exitAction;
        xr::ActionHandle m_poseAction;
        xr::ActionHandle m_vibrateAction;

        struct SwapchainData {
            XrSwapchain handle = {XR_NULL_HANDLE};
            int32_t width{0};
            int32_t height{0};
            std::vector<XrSwapchainImageD3D11KHR> images;
        };

        DXGI_FORMAT m_colorSwapchainFormat{};
        DXGI_FORMAT m_depthSwapchainFormat{};
        XrEnvironmentBlendMode m_environmentBlendMode{};
        xr::math::NearFarDistance m_nearFar{};

        struct RenderResources {
            std::vector<XrView> Views;
            std::vector<XrViewConfigurationView> ConfigViews;
            std::vector<std::unique_ptr<SwapchainData>> ColorSwapchains;
            std::vector<std::unique_ptr<SwapchainData>> DepthSwapchains;
            std::vector<XrCompositionLayerProjectionView> ProjectionLayerViews;
            std::vector<XrCompositionLayerDepthInfoKHR> DepthInfoViews;
        };

        std::unique_ptr<RenderResources> m_renderResources{};

        bool m_sessionRunning{false};
        XrSessionState m_sessionState{XR_SESSION_STATE_UNKNOWN};
    };
} // namespace

namespace xr::sample {
    std::unique_ptr<xr::sample::IOpenXrProgram> CreateOpenXrProgram(std::string applicationName,
                                                                    std::unique_ptr<xr::sample::IGraphicsPluginD3D11> graphicsPlugin) {
        return std::make_unique<ImplementOpenXrProgram>(std::move(applicationName), std::move(graphicsPlugin));
    }
} // namespace xr::sample
