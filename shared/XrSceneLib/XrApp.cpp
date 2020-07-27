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

#include <XrUtility/XrEnumerate.h>
#include <XrUtility/XrToString.h>
#include <XrUtility/XrViewConfiguration.h>

#include <SampleShared/FileUtility.h>
#include <SampleShared/DxUtility.h>
#include <SampleShared/BgfxUtility.h>
#include <SampleShared/Trace.h>

#include "XrApp.h"
#include "CompositionLayers.h"
#include "SceneContext.h"
#include <bgfx/bgfx.h>


using namespace DirectX;
using namespace std::chrono_literals;

namespace {
    const std::vector<DXGI_FORMAT> SupportedColorSwapchainFormats = {
        DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
    };

    const std::vector<DXGI_FORMAT> SupportedDepthSwapchainFormats = {
        DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_D16_UNORM,
    };

    const XrViewConfigurationType PrimaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

    const std::vector<XrViewConfigurationType> SupportedViewConfigurationTypes = {
        XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
        XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT,
    };

    const std::vector<XrEnvironmentBlendMode> SupportedEnvironmentBlendModes = {
        XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
        XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
        XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
    };

    const std::vector<D3D_FEATURE_LEVEL> SupportedFeatureLevels = {
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    std::vector<std::string> CombineSceneLibRequestedExtensions(const std::vector<std::string>& extensions) {
        const std::vector<std::string> libraryRequestedExtensions = {
            XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
            XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME,
            XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME,
            XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME,
            XR_MSFT_FIRST_PERSON_OBSERVER_EXTENSION_NAME,
#if UWP
            XR_MSFT_HOLOGRAPHIC_WINDOW_ATTACHMENT_PREVIEW_EXTENSION_NAME,
            XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME,
#endif
        };

        std::vector<std::string> combinedExtensions = extensions;
        for (auto extension : libraryRequestedExtensions) {
            if (std::find(combinedExtensions.begin(), combinedExtensions.end(), extension) == combinedExtensions.end()) {
                combinedExtensions.push_back(extension);
            }
        }
        return combinedExtensions;
    }

    class ImplementXrApp : public XrApp {
    public:
        ImplementXrApp(XrAppConfiguration appConfiguration);
        ~ImplementXrApp();

        void AddScene(std::unique_ptr<Scene> scene) override;
        const std::vector<std::unique_ptr<Scene>>& Scenes() const override;

        void Run() override;
        void Stop() override;
        bool Step() override;

        ::SceneContext& SceneContext() const override {
            return *m_sceneContext;
        }

        ::ProjectionLayers& ProjectionLayers() override {
            return m_projectionLayers;
        }

    private:

        const XrAppConfiguration m_appConfiguration;

        std::unique_ptr<::SceneContext> m_sceneContext;
        xr::SpaceHandle m_viewSpace;
        xr::SpaceHandle m_sceneSpace;

        ::ProjectionLayers m_projectionLayers;
        std::unordered_map<XrViewConfigurationType, xr::ViewConfigurationState> m_viewConfigStates;

        std::mutex m_secondaryViewConfigActiveMutex;
        std::vector<XrSecondaryViewConfigurationStateMSFT> m_secondaryViewConfigurationsState;

        std::mutex m_sceneMutex;
        std::vector<std::unique_ptr<Scene>> m_scenes;

        std::atomic<bool> m_sessionRunning{false};
        std::atomic<bool> m_abortFrameLoop{false};
        bool m_actionBindingsFinalized{false};

        std::thread m_renderThread;
        std::atomic<bool> m_renderThreadRunning{false};

        std::mutex m_frameReadyToRenderMutex;
        std::condition_variable m_frameReadyToRenderNotify;
        bool m_frameReadyToRender{false};
        FrameTime m_currentFrameTime;

    private:
        bool ProcessEvents();
        void StartRenderThreadIfNotRunning();
        void StopRenderThreadIfRunning();
        void UpdateFrame();
        void RenderFrame();
        void NotifyFrameRenderThread();
        void RenderViewConfiguration(const std::scoped_lock<std::mutex>& proofOfSceneLock,
                                     XrViewConfigurationType viewConfigurationType,
                                     CompositionLayers& layers);
        void SetSecondaryViewConfigurationActive(xr::ViewConfigurationState& secondaryViewConfigState, bool active);

        void FinalizeActionBindings();
        void SyncActions(const std::scoped_lock<std::mutex>& proofOfSceneLock);

        void BeginSession();
        void EndSession();
    };

    ImplementXrApp::ImplementXrApp(XrAppConfiguration appConfiguration)
        : m_appConfiguration(std::move(appConfiguration)) {

        // Create an instance using combined extensions of XrSceneLib and the application.
        // The extension context record those supported by the runtime and enabled by the instance.
        std::vector<std::string> requestedExtensions = CombineSceneLibRequestedExtensions(m_appConfiguration.RequestedExtensions);
        std::vector<const char*> requestedExtensionsCStr;
        for (const std::string& ext : requestedExtensions) {
            requestedExtensionsCStr.push_back(ext.c_str());
        }

        xr::ExtensionContext extensions = xr::CreateExtensionContext(requestedExtensionsCStr);
        xr::InstanceContext instance =
            xr::CreateInstanceContext(m_appConfiguration.AppInfo, {"XrSceneLib", 1}, extensions.EnabledExtensions);
        extensions.PopulateDispatchTable(instance.Handle);

        // For example, this sample currently requires D3D11 extension to be supported.
        if (!extensions.SupportsD3D11) {
            throw std::logic_error("This sample currently only supports D3D11.");
        }

#if UWP
        if (!extensions.SupportsAppContainer) {
            throw std::logic_error("The UWP version of this sample requires XR_EXT_win32_appcontainer_compatible extension.");
        }
#endif

        // Then get the active system with required form factor.
        // If no system is plugged in, wait until the device is plugged in.
        xr::SystemContext system = [&instance, &extensions] {
            std::optional<xr::SystemContext> systemOpt;
            while (!(systemOpt = xr::CreateSystemContext(instance.Handle,
                                                         extensions,
                                                         XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
                                                         SupportedViewConfigurationTypes,
                                                         SupportedEnvironmentBlendModes))) {
                sample::Trace("Waiting for system plugin ...");
                std::this_thread::sleep_for(500ms);
            }
            return systemOpt.value();
        }();

        if (!xr::Contains(system.SupportedPrimaryViewConfigurationTypes, PrimaryViewConfigurationType)) {
            throw std::logic_error("The system doesn't support required primary view configuration.");
        }
        auto [d3d11Binding, device, deviceContext] = sample::bg::CreateD3D11Binding(
            instance.Handle, system.Id, extensions, m_appConfiguration.SingleThreadedD3D11Device, SupportedFeatureLevels);

        xr::SessionHandle sessionHandle;
        XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO, nullptr, 0, system.Id};

        xr::InsertExtensionStruct(sessionCreateInfo, d3d11Binding);

        XrHolographicWindowAttachmentMSFT holographicWindowAttachment;
        if (m_appConfiguration.HolographicWindowAttachment.has_value() && extensions.SupportsHolographicWindowAttachment) {
            holographicWindowAttachment = m_appConfiguration.HolographicWindowAttachment.value();
            xr::InsertExtensionStruct(sessionCreateInfo, holographicWindowAttachment);
        }

        CHECK_XRCMD(xrCreateSession(instance.Handle, &sessionCreateInfo, sessionHandle.Put()));

        xr::SessionContext session(std::move(sessionHandle),
                                   system,
                                   extensions,
                                   PrimaryViewConfigurationType,
                                   SupportedViewConfigurationTypes, // enable all supported secondary view config
                                   SupportedColorSwapchainFormats,
                                   SupportedDepthSwapchainFormats);

        // Initialize XrViewConfigurationView and XrView buffers
        for (const auto& viewConfigurationType : xr::GetAllViewConfigurationTypes(session)) {
            m_viewConfigStates.emplace(viewConfigurationType,
                                       xr::CreateViewConfigurationState(viewConfigurationType, instance.Handle, system.Id));
        }

        // Create view app space
        XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
        CHECK_XRCMD(xrCreateReferenceSpace(session.Handle, &spaceCreateInfo, m_viewSpace.Put()));

        // Create main app space
        spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        CHECK_XRCMD(xrCreateReferenceSpace(session.Handle, &spaceCreateInfo, m_sceneSpace.Put()));

        Pbr::Resources pbrResources = sample::InitializePbrResources(device.get());

        m_sceneContext = std::make_unique<::SceneContext>(std::move(instance),
                                                          std::move(extensions),
                                                          std::move(system),
                                                          std::move(session),
                                                          m_sceneSpace.Get(),
                                                          std::move(pbrResources),
                                                          device,
                                                          deviceContext);

        m_projectionLayers.Resize(1, SceneContext(), true /*forceReset*/);
    }

    ImplementXrApp::~ImplementXrApp() {
        StopRenderThreadIfRunning();

        {
            std::scoped_lock lock(m_sceneMutex);
            m_scenes.clear();
        }
    }

    void ImplementXrApp::AddScene(std::unique_ptr<Scene> scene) {
        if (!scene) {
            return; // Some scenes might skip creation due to extension unavailability.
        }

        std::scoped_lock lock(m_sceneMutex);
        m_scenes.push_back(std::move(scene));
    }

    const std::vector<std::unique_ptr<Scene>>& ImplementXrApp::Scenes() const {
        return m_scenes;
    }
     // initialize bgfx here
     //sample::bgfx {
     //   void* InitializeDevice(sample::RendererType rendererType, LUID adapterLuid) override {
    void ImplementXrApp::Run() {
        ::SetThreadDescription(::GetCurrentThread(), L"App Thread");
        
        m_abortFrameLoop = false;
        while (Step()) {
        }
    }

    bool ImplementXrApp::Step() {
        if (m_abortFrameLoop || !ProcessEvents()) {
            return false; // quit frame loop
        }

        // Defer action bindings until the first game step.
        if (!m_actionBindingsFinalized) {
            FinalizeActionBindings();
            m_actionBindingsFinalized = true;
        }

        if (m_sessionRunning) {
            if (m_appConfiguration.RenderSynchronously) {
                UpdateFrame();
                RenderFrame();
            } else {
                StartRenderThreadIfNotRunning();
                UpdateFrame();
                NotifyFrameRenderThread();
            }
        } else {
            std::this_thread::sleep_for(0.1s);
        }

        return true; // continue frame loop
    }

    void ImplementXrApp::NotifyFrameRenderThread() {
        {
            std::unique_lock lock(m_frameReadyToRenderMutex);
            m_frameReadyToRender = true;
        }
        m_frameReadyToRenderNotify.notify_all();
    }

    void ImplementXrApp::FinalizeActionBindings() {
        std::scoped_lock sceneLock(m_sceneMutex);

        std::vector<const xr::ActionContext*> actionContexts;
        for (const auto& scene : m_scenes) {
            actionContexts.push_back(&scene->ActionContext());
        }

        xr::AttachActionsToSession(SceneContext().Instance.Handle, SceneContext().Session.Handle, actionContexts);
    }

    void ImplementXrApp::SyncActions(const std::scoped_lock<std::mutex>& proofOfSceneLock) {
        std::vector<const xr::ActionContext*> actionContexts;
        for (const auto& scene : m_scenes) {
            if (scene->IsActive()) {
                actionContexts.push_back(&scene->ActionContext());
            }
        }
        xr::SyncActions(SceneContext().Session.Handle, actionContexts);
    }

    void ImplementXrApp::StartRenderThreadIfNotRunning() {
        bool alreadyRunning = false;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, true)) {
            m_frameReadyToRender = false; // Always wait for xrWaitFrame before begin rendering frames.
            m_renderThread = std::thread([this]() {
                try {
                    ::SetThreadDescription(::GetCurrentThread(), L"Render Thread");

                    while (m_renderThreadRunning && m_sessionRunning) {
                        {
                            std::unique_lock lock(m_frameReadyToRenderMutex);
                            m_frameReadyToRenderNotify.wait(lock, [this] { return m_frameReadyToRender; });
                            m_frameReadyToRender = false;
                        }

                        if (!m_renderThreadRunning || !m_sessionRunning) {
                            break; // check again after waiting
                        }

                        RenderFrame();
                    }
                } catch (const std::exception& ex) {
                    sample::Trace("Render thread exception: {}", ex.what());
                    m_abortFrameLoop = true;
                } catch (...) {
                    sample::Trace(L"Render thread exception");
                    m_abortFrameLoop = true;
                }
            });
        }
    }

    void ImplementXrApp::StopRenderThreadIfRunning() {
        bool alreadyRunning = true;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, false)) {
            {
                // Notify "frameReadyToRender" with "renderThreadRunning = false" to exit render thread.
                std::unique_lock lock(m_frameReadyToRenderMutex);
                m_frameReadyToRender = true;
            }
            m_frameReadyToRenderNotify.notify_all();
            if (m_renderThread.joinable()) {
                m_renderThread.join();
            }
        }
    }

    void ImplementXrApp::Stop() {
        if (m_sessionRunning) {
            CHECK_XRCMD(xrRequestExitSession(SceneContext().Session.Handle));
        } else {
            m_abortFrameLoop = true; // quit app frame loop
        }
    }

    bool ImplementXrApp::ProcessEvents() {
        XrEventDataBuffer eventData;
        while (true) {
            eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
            eventData.next = nullptr;
            XrResult res = xrPollEvent(SceneContext().Instance.Handle, &eventData);
            CHECK_XRCMD(res);
            if (res == XR_EVENT_UNAVAILABLE) {
                return true;
            }

            if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                auto* sessionStateChanged = xr::event_cast<XrEventDataSessionStateChanged>(&eventData);
                if (sessionStateChanged->session == SceneContext().Session.Handle) {
                    SceneContext().SessionState = sessionStateChanged->state;
                    switch (SceneContext().SessionState) {
                    case XR_SESSION_STATE_EXITING:
                        return false; // User's intended to quit
                    case XR_SESSION_STATE_LOSS_PENDING:
                        return false; // Runtime's intend to quit
                    case XR_SESSION_STATE_READY:
                        BeginSession();
                        break;
                    case XR_SESSION_STATE_STOPPING:
                        EndSession();
                        break;
                    }
                }
            }

            {
                std::scoped_lock lock(m_sceneMutex);
                for (const std::unique_ptr<Scene>& scene : m_scenes) {
                    scene->NotifyEvent(eventData);
                }
            }
        }
    }

    void ImplementXrApp::BeginSession() {
        XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
        sessionBeginInfo.primaryViewConfigurationType = PrimaryViewConfigurationType;

        XrSecondaryViewConfigurationSessionBeginInfoMSFT secondaryViewConfigInfo{
            XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SESSION_BEGIN_INFO_MSFT};
        if (SceneContext().Extensions.SupportsSecondaryViewConfiguration &&
            SceneContext().Session.EnabledSecondaryViewConfigurationTypes.size() > 0) {
            secondaryViewConfigInfo.viewConfigurationCount = (uint32_t)SceneContext().Session.EnabledSecondaryViewConfigurationTypes.size();
            secondaryViewConfigInfo.enabledViewConfigurationTypes = SceneContext().Session.EnabledSecondaryViewConfigurationTypes.data();
            xr::InsertExtensionStruct(sessionBeginInfo, secondaryViewConfigInfo);
        }

        CHECK_XRCMD(xrBeginSession(SceneContext().Session.Handle, &sessionBeginInfo));
        m_sessionRunning = true;
    }

    void ImplementXrApp::EndSession() {
        StopRenderThreadIfRunning();
        m_sessionRunning = false;
        CHECK_XRCMD(xrEndSession(SceneContext().Session.Handle));
    }

    void ImplementXrApp::UpdateFrame() {
        XrFrameState frameState{XR_TYPE_FRAME_STATE};

        // secondaryViewConfigFrameState needs to have the same lifetime as frameState
        XrSecondaryViewConfigurationFrameStateMSFT secondaryViewConfigFrameState{XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT};

        const size_t enabledSecondaryViewConfigCount = SceneContext().Session.EnabledSecondaryViewConfigurationTypes.size();
        std::vector<XrSecondaryViewConfigurationStateMSFT> secondaryViewConfigStates(enabledSecondaryViewConfigCount,
                                                                                     {XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT});

        if (SceneContext().Extensions.SupportsSecondaryViewConfiguration && enabledSecondaryViewConfigCount > 0) {
            secondaryViewConfigFrameState.viewConfigurationCount = (uint32_t)secondaryViewConfigStates.size();
            secondaryViewConfigFrameState.viewConfigurationStates = secondaryViewConfigStates.data();
            xr::InsertExtensionStruct(frameState, secondaryViewConfigFrameState);
        }

        XrFrameWaitInfo waitFrameInfo{XR_TYPE_FRAME_WAIT_INFO};
        CHECK_XRCMD(xrWaitFrame(SceneContext().Session.Handle, &waitFrameInfo, &frameState));

        if (SceneContext().Extensions.SupportsSecondaryViewConfiguration) {
            std::scoped_lock lock(m_secondaryViewConfigActiveMutex);
            m_secondaryViewConfigurationsState = std::move(secondaryViewConfigStates);
        }

        {
            std::scoped_lock sceneLock(m_sceneMutex);

            SyncActions(sceneLock);

            m_currentFrameTime.Update(frameState);

            for (auto& scene : m_scenes) {
                if (scene->IsActive()) {
                    scene->Update(m_currentFrameTime);
                }
            }
        }
    }

    void ImplementXrApp::SetSecondaryViewConfigurationActive(xr::ViewConfigurationState& secondaryViewConfigState, bool active) {
        if (secondaryViewConfigState.Active != active) {
            secondaryViewConfigState.Active = active;

            // When a returned secondary view configuration is changed to active and recommended swapchain size is changed,
            // reset resources in layers related to this secondary view configuration.
            if (active) {
                std::vector<XrViewConfigurationView> newViewConfigViews = xr::EnumerateViewConfigurationViews(
                    SceneContext().Instance.Handle, SceneContext().System.Id, secondaryViewConfigState.Type);
                if (xr::IsRecommendedSwapchainSizeChanged(secondaryViewConfigState.ViewConfigViews, newViewConfigViews)) {
                    secondaryViewConfigState.ViewConfigViews = std::move(newViewConfigViews);
                    m_projectionLayers.ForEachLayerWithLock([secondaryViewConfigType = secondaryViewConfigState.Type](auto&& layer) {
                        layer.Config(secondaryViewConfigType).ForceReset = true;
                    });
                }
            }
        }
    }

    void ImplementXrApp::RenderFrame() {
        // Must snapshot the frame time for the render thread before xrBeginFrame because it will unblock xrWaitFrame concurrently and
        // m_currentFrameTime will be updated for the next frame.
        const FrameTime renderFrameTime = m_currentFrameTime;

        XrFrameBeginInfo beginFrameDescription{XR_TYPE_FRAME_BEGIN_INFO};
        CHECK_XRCMD(xrBeginFrame(SceneContext().Session.Handle, &beginFrameDescription));

        if (SceneContext().Extensions.SupportsSecondaryViewConfiguration) {
            std::scoped_lock lock(m_secondaryViewConfigActiveMutex);
            for (auto& state : m_secondaryViewConfigurationsState) {
                SetSecondaryViewConfigurationActive(m_viewConfigStates.at(state.viewConfigurationType), state.active);
            }
        }

        m_projectionLayers.ForEachLayerWithLock([this](auto&& layer) {
            for (auto& [viewConfigType, state] : m_viewConfigStates) {
                if (xr::IsPrimaryViewConfigurationType(viewConfigType) || state.Active) {
                    layer.PrepareRendering(SceneContext(), viewConfigType, state.ViewConfigViews);
                }
            }
        });

        XrFrameEndInfo endFrameInfo{XR_TYPE_FRAME_END_INFO};
        endFrameInfo.environmentBlendMode = SceneContext().Session.PrimaryViewConfigurationBlendMode;
        endFrameInfo.displayTime = renderFrameTime.PredictedDisplayTime;

        // Secondary view config frame info need to have same lifetime as XrFrameEndInfo;
        XrSecondaryViewConfigurationFrameEndInfoMSFT frameEndSecondaryViewConfigInfo{
            XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT};
        std::vector<XrSecondaryViewConfigurationLayerInfoMSFT> activeSecondaryViewConfigLayerInfos;

        // Chain secondary view configuration layers data to endFrameInfo
        if (SceneContext().Extensions.SupportsSecondaryViewConfiguration &&
            SceneContext().Session.EnabledSecondaryViewConfigurationTypes.size() > 0) {
            for (auto& secondaryViewConfigType : SceneContext().Session.EnabledSecondaryViewConfigurationTypes) {
                auto& secondaryViewConfig = m_viewConfigStates.at(secondaryViewConfigType);
                if (secondaryViewConfig.Active) {
                    activeSecondaryViewConfigLayerInfos.emplace_back(XrSecondaryViewConfigurationLayerInfoMSFT{
                        XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT,
                        nullptr,
                        secondaryViewConfigType,
                        SceneContext().System.ViewProperties.at(secondaryViewConfigType).BlendMode});
                }
            }

            if (activeSecondaryViewConfigLayerInfos.size() > 0) {
                frameEndSecondaryViewConfigInfo.viewConfigurationCount = (uint32_t)activeSecondaryViewConfigLayerInfos.size();
                frameEndSecondaryViewConfigInfo.viewConfigurationLayersInfo = activeSecondaryViewConfigLayerInfos.data();
                xr::InsertExtensionStruct(endFrameInfo, frameEndSecondaryViewConfigInfo);
            }
        }

        // Prepare array of layer data for each active view configurations.
        std::vector<CompositionLayers> layersForAllViewConfigs(1 + activeSecondaryViewConfigLayerInfos.size());

        if (renderFrameTime.ShouldRender) {
            std::scoped_lock sceneLock(m_sceneMutex);

            // Render for the primary view configuration.
            CompositionLayers& primaryViewConfigLayers = layersForAllViewConfigs[0];
            RenderViewConfiguration(sceneLock, PrimaryViewConfigurationType, primaryViewConfigLayers);
            endFrameInfo.layerCount = primaryViewConfigLayers.LayerCount();
            endFrameInfo.layers = primaryViewConfigLayers.LayerData();

            // Render layers for any active secondary view configurations too.
            if (SceneContext().Extensions.SupportsSecondaryViewConfiguration && activeSecondaryViewConfigLayerInfos.size() > 0) {
                for (size_t i = 0; i < activeSecondaryViewConfigLayerInfos.size(); i++) {
                    XrSecondaryViewConfigurationLayerInfoMSFT& secondaryViewConfigLayerInfo = activeSecondaryViewConfigLayerInfos.at(i);
                    CompositionLayers& secondaryViewConfigLayers = layersForAllViewConfigs.at(i + 1);
                    RenderViewConfiguration(sceneLock, secondaryViewConfigLayerInfo.viewConfigurationType, secondaryViewConfigLayers);
                    secondaryViewConfigLayerInfo.layerCount = secondaryViewConfigLayers.LayerCount();
                    secondaryViewConfigLayerInfo.layers = secondaryViewConfigLayers.LayerData();
                }
            }
        }

        CHECK_XRCMD(xrEndFrame(SceneContext().Session.Handle, &endFrameInfo));
    }

    void ImplementXrApp::RenderViewConfiguration(const std::scoped_lock<std::mutex>& proofOfSceneLock,
                                                 XrViewConfigurationType viewConfigurationType,
                                                 CompositionLayers& layers) {
        // Locate the views in VIEW space to get the per-view offset from the VIEW "camera"
        XrViewState viewState{XR_TYPE_VIEW_STATE};
        std::vector<XrView>& views = m_viewConfigStates.at(viewConfigurationType).Views;
        {
            XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
            viewLocateInfo.viewConfigurationType = viewConfigurationType;
            viewLocateInfo.displayTime = m_currentFrameTime.PredictedDisplayTime;
            viewLocateInfo.space = m_viewSpace.Get();

            uint32_t viewCount = 0;
            CHECK_XRCMD(xrLocateViews(
                SceneContext().Session.Handle, &viewLocateInfo, &viewState, (uint32_t)views.size(), &viewCount, views.data()));
            assert(viewCount == views.size());
            if (!xr::math::Pose::IsPoseValid(viewState)) {
                return;
            }
        }

        // Locate the VIEW space in the scene space to get the "camera" pose and combine the per-view offsets with the camera pose.
        XrSpaceLocation viewLocation{XR_TYPE_SPACE_LOCATION};
        CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_sceneSpace.Get(), m_currentFrameTime.PredictedDisplayTime, &viewLocation));
        if (!xr::math::Pose::IsPoseValid(viewLocation)) {
            return;
        }

        for (XrView& view : views) {
            view.pose = xr::math::Pose::Multiply(view.pose, viewLocation.pose);
        }

        std::vector<std::shared_ptr<QuadLayerObject>> underlays, overlays;
        {
            // Collect all quad layers in active scenes
            for (const std::unique_ptr<Scene>& scene : m_scenes) {
                if (!scene->IsActive()) {
                    continue;
                }
                for (const std::shared_ptr<QuadLayerObject>& quad : scene->GetQuadLayerObjects()) {
                    if (!quad->IsVisible()) {
                        continue;
                    }
                    if (quad->LayerGroup == LayerGrouping::Underlay) {
                        underlays.push_back(quad);
                    } else if (quad->LayerGroup == LayerGrouping::Overlay) {
                        overlays.push_back(quad);
                    }
                }
            }
        }

        for (const std::shared_ptr<QuadLayerObject>& quad : underlays) {
            AppendQuadLayer(layers, quad.get());
        }

        m_projectionLayers.ForEachLayerWithLock([this, &layers, &views, viewConfigurationType](ProjectionLayer& projectionLayer) {
            bool opaqueClearColor = (layers.LayerCount() == 0); // Only the first projection layer need opaque background
            opaqueClearColor &= (SceneContext().Session.PrimaryViewConfigurationBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
            DirectX::XMStoreFloat4(&projectionLayer.Config().ClearColor,
                                   opaqueClearColor ? DirectX::XMColorSRGBToRGB(DirectX::Colors::CornflowerBlue)
                                                    : DirectX::Colors::Transparent);
            const bool shouldSubmitProjectionLayer = projectionLayer.Render(
                SceneContext(), m_currentFrameTime, SceneContext().SceneSpace, views, m_scenes, viewConfigurationType);

            // Create the multi projection layer
            if (shouldSubmitProjectionLayer) {
                AppendProjectionLayer(layers, &projectionLayer, viewConfigurationType);
            }
        });

        for (const std::shared_ptr<QuadLayerObject>& quad : overlays) {
            AppendQuadLayer(layers, quad.get());
        }
    }

} // namespace

std::unique_ptr<XrApp> CreateXrApp(XrAppConfiguration appConfiguration) {
    return std::make_unique<ImplementXrApp>(std::move(appConfiguration));
}

