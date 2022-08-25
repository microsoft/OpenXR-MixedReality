// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <XrUtility/XrEnumerate.h>
#include <XrUtility/XrToString.h>
#include <XrUtility/XrViewConfiguration.h>

#include <SampleShared/FileUtility.h>
#include <SampleShared/DxUtility.h>
#include <SampleShared/Trace.h>
#include <SampleShared/ScopeGuard.h>

#include "CompositionLayers.h"
#include "Context.h"
#include "XrApp.h"

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
            XR_MSFT_HOLOGRAPHIC_WINDOW_ATTACHMENT_EXTENSION_NAME,
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

    class ImplementXrApp : public engine::XrApp {
    public:
        ImplementXrApp(engine::XrAppConfiguration appConfiguration);
        ~ImplementXrApp();

        void AddScene(std::unique_ptr<engine::Scene> scene) override;
        const std::vector<std::unique_ptr<engine::Scene>>& Scenes() const override;

        void Run() override;
        void Stop() override;
        bool Step() override;

        engine::Context& Context() const override {
            return *m_context;
        }

        engine::ProjectionLayers& ProjectionLayers() override {
            return m_projectionLayers;
        }

    private:

        const engine::XrAppConfiguration m_appConfiguration;

        std::unique_ptr<engine::Context> m_context;
        xr::SpaceHandle m_viewSpace;
        xr::SpaceHandle m_appSpace;

        engine::ProjectionLayers m_projectionLayers;
        std::unordered_map<XrViewConfigurationType, xr::ViewConfigurationState> m_viewConfigStates;

        std::mutex m_secondaryViewConfigActiveMutex;
        std::vector<XrSecondaryViewConfigurationStateMSFT> m_secondaryViewConfigurationsState;

        std::mutex m_sceneMutex;
        std::vector<std::unique_ptr<engine::Scene>> m_scenes;

        std::atomic<XrSessionState> m_sessionState;
        std::atomic<bool> m_sessionRunning{false};
        std::atomic<bool> m_abortFrameLoop{false};
        bool m_actionBindingsFinalized{false};

        std::thread m_renderThread;
        std::atomic<bool> m_renderThreadRunning{false};

        std::mutex m_frameReadyToRenderMutex;
        std::condition_variable m_frameReadyToRenderNotify;
        bool m_frameReadyToRender{false};
        engine::FrameTime m_currentFrameTime{};

    private:
        bool ProcessEvents();
        void StartRenderThreadIfNotRunning();
        void StopRenderThreadIfRunning();
        void UpdateFrame();
        void RenderFrame();
        void NotifyFrameRenderThread();
        void RenderViewConfiguration(const std::scoped_lock<std::mutex>& proofOfSceneLock,
                                     XrViewConfigurationType viewConfigurationType,
                                     engine::CompositionLayers& layers);
        void SetSecondaryViewConfigurationActive(xr::ViewConfigurationState& secondaryViewConfigState, bool active);

        void FinalizeActionBindings();
        void SyncActions(const std::scoped_lock<std::mutex>& proofOfSceneLock);

        void BeginSession();
        void EndSession();
    };

    ImplementXrApp::ImplementXrApp(engine::XrAppConfiguration appConfiguration)
        : m_appConfiguration(std::move(appConfiguration)) {

        // Create an instance using combined extensions of XrSceneLib and the application.
        // The extension context record those supported by the runtime and enabled by the instance.
        std::vector<std::string> requestedExtensions = CombineSceneLibRequestedExtensions(m_appConfiguration.RequestedExtensions);
        std::vector<const char*> requestedExtensionsCStr;
        for (const std::string& ext : requestedExtensions) {
            requestedExtensionsCStr.push_back(ext.c_str());
        }

        xr::ExtensionContext extensions = xr::CreateExtensionContext(requestedExtensionsCStr);

        // For example, this sample currently requires D3D11 extension to be supported.
        if (!extensions.SupportsD3D11) {
            throw std::logic_error("This sample currently only supports D3D11.");
        }
#if UWP
        if (!extensions.SupportsAppContainer) {
            throw std::logic_error("The UWP version of this sample requires XR_EXT_win32_appcontainer_compatible extension.");
        }
#endif

        sample::InstanceContext instance =
            sample::CreateInstanceContext(m_appConfiguration.AppInfo, {"XrSceneLib", 1}, extensions.EnabledExtensions);

        xr::g_dispatchTable.Initialize(instance.Handle, xrGetInstanceProcAddr);

        // Then get the active system with required form factor.
        // If no system is plugged in, wait until the device is plugged in.
        sample::SystemContext system = [&instance, &extensions] {
            std::optional<sample::SystemContext> systemOpt;
            while (!(systemOpt = sample::CreateSystemContext(instance.Handle,
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

        auto [d3d11Binding, device, deviceContext] = sample::dx::CreateD3D11Binding(
            instance.Handle, system.Id, extensions, m_appConfiguration.SingleThreadedD3D11Device, SupportedFeatureLevels);

        xr::SessionHandle sessionHandle;
        XrSessionCreateInfo sessionCreateInfo{XR_TYPE_SESSION_CREATE_INFO, nullptr, 0, system.Id};

        xr::InsertExtensionStruct(sessionCreateInfo, d3d11Binding);

        XrHolographicWindowAttachmentMSFT holographicWindowAttachment;
        if (m_appConfiguration.HolographicWindowAttachment.has_value() && extensions.SupportsHolographicWindowAttachment) {
            holographicWindowAttachment = m_appConfiguration.HolographicWindowAttachment.value();
            xr::InsertExtensionStruct(sessionCreateInfo, holographicWindowAttachment);
        }

        CHECK_XRCMD(xrCreateSession(instance.Handle, &sessionCreateInfo, sessionHandle.Put(xrDestroySession)));

        sample::SessionContext session(std::move(sessionHandle),
                                       system,
                                       extensions,
                                       PrimaryViewConfigurationType,
                                       SupportedViewConfigurationTypes, // enable all supported secondary view config
                                       SupportedColorSwapchainFormats,
                                       SupportedDepthSwapchainFormats);

        // Initialize XrViewConfigurationView and XrView buffers
        for (const auto& viewConfigurationType : sample::GetAllViewConfigurationTypes(session)) {
            m_viewConfigStates.emplace(viewConfigurationType,
                                       xr::CreateViewConfigurationState(viewConfigurationType, instance.Handle, system.Id));
        }

        // Create view app space
        XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
        spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
        CHECK_XRCMD(xrCreateReferenceSpace(session.Handle, &spaceCreateInfo, m_viewSpace.Put(xrDestroySpace)));

        // Create main app space
        spaceCreateInfo.referenceSpaceType =
            extensions.SupportsUnboundedSpace ? XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT : XR_REFERENCE_SPACE_TYPE_LOCAL;
        CHECK_XRCMD(xrCreateReferenceSpace(session.Handle, &spaceCreateInfo, m_appSpace.Put(xrDestroySpace)));

        Pbr::Resources pbrResources = sample::InitializePbrResources(device.get());

        m_context = std::make_unique<engine::Context>(std::move(instance),
                                                      std::move(extensions),
                                                      std::move(system),
                                                      std::move(session),
                                                      m_appSpace.Get(),
                                                      std::move(pbrResources),
                                                      device,
                                                      deviceContext);

        m_projectionLayers.Resize(1, Context(), true /*forceReset*/);
    }

    ImplementXrApp::~ImplementXrApp() {
        StopRenderThreadIfRunning();

        {
            std::scoped_lock lock(m_sceneMutex);
            m_scenes.clear();
        }
    }

    void ImplementXrApp::AddScene(std::unique_ptr<engine::Scene> scene) {
        if (!scene) {
            return; // Some scenes might skip creation due to extension unavailability.
        }

        std::scoped_lock lock(m_sceneMutex);
        m_scenes.push_back(std::move(scene));
    }

    const std::vector<std::unique_ptr<engine::Scene>>& ImplementXrApp::Scenes() const {
        return m_scenes;
    }

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

        std::vector<const sample::ActionContext*> actionContexts;
        for (const auto& scene : m_scenes) {
            actionContexts.push_back(&scene->ActionContext());
        }
        sample::AttachActionsToSession(
            Context().Instance.Handle, Context().Session.Handle, actionContexts, m_appConfiguration.InteractionProfilesFilter);
    }

    void ImplementXrApp::SyncActions(const std::scoped_lock<std::mutex>& proofOfSceneLock) {
        std::vector<const sample::ActionContext*> actionContexts;
        for (const auto& scene : m_scenes) {
            if (scene->IsActive()) {
                actionContexts.push_back(&scene->ActionContext());
            }
        }
        sample::SyncActions(Context().Session.Handle, actionContexts);
    }

    void ImplementXrApp::StartRenderThreadIfNotRunning() {
        bool alreadyRunning = false;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, true)) {
            m_frameReadyToRender = false; // Always wait for xrWaitFrame before begin rendering frames.
            m_renderThread = std::thread([this]() {
                try {
                    ::SetThreadDescription(::GetCurrentThread(), L"Render Thread");

                    auto scopeGuard = MakeFailureGuard([&] {
                        // Abort frame loop on error and ensure to balance begin/wait frame count, so as to
                        // avoid deadlock in xrWaitFrame because there's no more xrBeginFrame after the exception.
                        m_abortFrameLoop = true;
                        XrFrameBeginInfo beginFrameDescription{XR_TYPE_FRAME_BEGIN_INFO};
                        // Ignore errors here because exception in render thread already happened.
                        (void)(xrBeginFrame(Context().Session.Handle, &beginFrameDescription));
                    });

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
                } catch (...) {
                    sample::Trace("Render thread exception");
                }
            });
        }
    }

    void ImplementXrApp::StopRenderThreadIfRunning() {
        bool alreadyRunning = true;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, false)) {
            sample::Trace("Stopping render thread...");
            {
                // Notify "frameReadyToRender" with "renderThreadRunning = false" to exit render thread.
                std::unique_lock lock(m_frameReadyToRenderMutex);
                m_frameReadyToRender = true;
            }
            m_frameReadyToRenderNotify.notify_all();
            if (m_renderThread.joinable()) {
                m_renderThread.join();
                sample::Trace("Render thread joined.");
            }
        }
    }

    void ImplementXrApp::Stop() {
        if (m_sessionRunning) {
            CHECK_XRCMD(xrRequestExitSession(Context().Session.Handle));
        } else {
            m_abortFrameLoop = true; // quit app frame loop
        }
    }

    bool ImplementXrApp::ProcessEvents() {
        XrEventDataBuffer eventData;
        while (true) {
            eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
            eventData.next = nullptr;
            XrResult res = xrPollEvent(Context().Instance.Handle, &eventData);
            CHECK_XRCMD(res);
            if (res == XR_EVENT_UNAVAILABLE) {
                return true;
            }

            if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                auto* sessionStateChanged = xr::event_cast<XrEventDataSessionStateChanged>(&eventData);
                if (sessionStateChanged->session == Context().Session.Handle) {
                    switch (m_sessionState = sessionStateChanged->state) {
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
                for (const std::unique_ptr<engine::Scene>& scene : m_scenes) {
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
        if (Context().Extensions.SupportsSecondaryViewConfiguration &&
            Context().Session.EnabledSecondaryViewConfigurationTypes.size() > 0) {
            secondaryViewConfigInfo.viewConfigurationCount = (uint32_t)Context().Session.EnabledSecondaryViewConfigurationTypes.size();
            secondaryViewConfigInfo.enabledViewConfigurationTypes = Context().Session.EnabledSecondaryViewConfigurationTypes.data();
            xr::InsertExtensionStruct(sessionBeginInfo, secondaryViewConfigInfo);
        }

        CHECK_XRCMD(xrBeginSession(Context().Session.Handle, &sessionBeginInfo));
        m_sessionRunning = true;
    }

    void ImplementXrApp::EndSession() {
        StopRenderThreadIfRunning();
        m_sessionRunning = false;

        m_projectionLayers.ForEachLayerWithLock([this](auto&& layer) { layer.DestroySwapchains(); });

        CHECK_XRCMD(xrEndSession(Context().Session.Handle));
    }

    void ImplementXrApp::UpdateFrame() {
        XrFrameState frameState{XR_TYPE_FRAME_STATE};

        // secondaryViewConfigFrameState needs to have the same lifetime as frameState
        XrSecondaryViewConfigurationFrameStateMSFT secondaryViewConfigFrameState{XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_STATE_MSFT};

        const size_t enabledSecondaryViewConfigCount = Context().Session.EnabledSecondaryViewConfigurationTypes.size();
        std::vector<XrSecondaryViewConfigurationStateMSFT> secondaryViewConfigStates(enabledSecondaryViewConfigCount,
                                                                                     {XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT});

        if (Context().Extensions.SupportsSecondaryViewConfiguration && enabledSecondaryViewConfigCount > 0) {
            secondaryViewConfigFrameState.viewConfigurationCount = (uint32_t)secondaryViewConfigStates.size();
            secondaryViewConfigFrameState.viewConfigurationStates = secondaryViewConfigStates.data();
            xr::InsertExtensionStruct(frameState, secondaryViewConfigFrameState);
        }

        XrFrameWaitInfo waitFrameInfo{XR_TYPE_FRAME_WAIT_INFO};
        CHECK_XRCMD(xrWaitFrame(Context().Session.Handle, &waitFrameInfo, &frameState));

        if (Context().Extensions.SupportsSecondaryViewConfiguration) {
            std::scoped_lock lock(m_secondaryViewConfigActiveMutex);
            m_secondaryViewConfigurationsState = std::move(secondaryViewConfigStates);
        }

        {
            std::scoped_lock sceneLock(m_sceneMutex);

            SyncActions(sceneLock);

            m_currentFrameTime.Update(frameState, m_sessionState);
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
                std::vector<XrViewConfigurationView> newViewConfigViews =
                    xr::EnumerateViewConfigurationViews(Context().Instance.Handle, Context().System.Id, secondaryViewConfigState.Type);
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
        const engine::FrameTime renderFrameTime = m_currentFrameTime;

        XrFrameBeginInfo beginFrameDescription{XR_TYPE_FRAME_BEGIN_INFO};
        CHECK_XRCMD(xrBeginFrame(Context().Session.Handle, &beginFrameDescription));

        if (Context().Extensions.SupportsSecondaryViewConfiguration) {
            std::scoped_lock lock(m_secondaryViewConfigActiveMutex);
            for (auto& state : m_secondaryViewConfigurationsState) {
                SetSecondaryViewConfigurationActive(m_viewConfigStates.at(state.viewConfigurationType), state.active);
            }
        }

        m_projectionLayers.ForEachLayerWithLock([this](auto&& layer) {
            for (auto& [viewConfigType, state] : m_viewConfigStates) {
                if (xr::IsPrimaryViewConfigurationType(viewConfigType) || state.Active) {
                    layer.PrepareRendering(Context(), viewConfigType, state.ViewConfigViews);
                }
            }
        });

        XrFrameEndInfo endFrameInfo{XR_TYPE_FRAME_END_INFO};
        endFrameInfo.environmentBlendMode = Context().Session.PrimaryViewConfigurationBlendMode;
        endFrameInfo.displayTime = renderFrameTime.PredictedDisplayTime;

        // Secondary view config frame info need to have same lifetime as XrFrameEndInfo;
        XrSecondaryViewConfigurationFrameEndInfoMSFT frameEndSecondaryViewConfigInfo{
            XR_TYPE_SECONDARY_VIEW_CONFIGURATION_FRAME_END_INFO_MSFT};
        std::vector<XrSecondaryViewConfigurationLayerInfoMSFT> activeSecondaryViewConfigLayerInfos;

        // Chain secondary view configuration layers data to endFrameInfo
        if (Context().Extensions.SupportsSecondaryViewConfiguration &&
            Context().Session.EnabledSecondaryViewConfigurationTypes.size() > 0) {
            for (auto& secondaryViewConfigType : Context().Session.EnabledSecondaryViewConfigurationTypes) {
                auto& secondaryViewConfig = m_viewConfigStates.at(secondaryViewConfigType);
                if (secondaryViewConfig.Active) {
                    activeSecondaryViewConfigLayerInfos.emplace_back(
                        XrSecondaryViewConfigurationLayerInfoMSFT{XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT,
                                                                  nullptr,
                                                                  secondaryViewConfigType,
                                                                  Context().System.ViewProperties.at(secondaryViewConfigType).BlendMode});
                }
            }

            if (activeSecondaryViewConfigLayerInfos.size() > 0) {
                frameEndSecondaryViewConfigInfo.viewConfigurationCount = (uint32_t)activeSecondaryViewConfigLayerInfos.size();
                frameEndSecondaryViewConfigInfo.viewConfigurationLayersInfo = activeSecondaryViewConfigLayerInfos.data();
                xr::InsertExtensionStruct(endFrameInfo, frameEndSecondaryViewConfigInfo);
            }
        }

        // Prepare array of layer data for each active view configurations.
        std::vector<engine::CompositionLayers> layersForAllViewConfigs(1 + activeSecondaryViewConfigLayerInfos.size());

        if (renderFrameTime.ShouldRender) {
            std::scoped_lock sceneLock(m_sceneMutex);

            for (const std::unique_ptr<engine::Scene>& scene : m_scenes) {
                if (scene->IsActive()) {
                    scene->BeforeRender(m_currentFrameTime);
                }
            }

            // Render for the primary view configuration.
            engine::CompositionLayers& primaryViewConfigLayers = layersForAllViewConfigs[0];
            RenderViewConfiguration(sceneLock, PrimaryViewConfigurationType, primaryViewConfigLayers);
            endFrameInfo.layerCount = primaryViewConfigLayers.LayerCount();
            endFrameInfo.layers = primaryViewConfigLayers.LayerData();

            // Render layers for any active secondary view configurations too.
            if (Context().Extensions.SupportsSecondaryViewConfiguration && activeSecondaryViewConfigLayerInfos.size() > 0) {
                for (size_t i = 0; i < activeSecondaryViewConfigLayerInfos.size(); i++) {
                    XrSecondaryViewConfigurationLayerInfoMSFT& secondaryViewConfigLayerInfo = activeSecondaryViewConfigLayerInfos.at(i);
                    engine::CompositionLayers& secondaryViewConfigLayers = layersForAllViewConfigs.at(i + 1);
                    RenderViewConfiguration(sceneLock, secondaryViewConfigLayerInfo.viewConfigurationType, secondaryViewConfigLayers);
                    secondaryViewConfigLayerInfo.layerCount = secondaryViewConfigLayers.LayerCount();
                    secondaryViewConfigLayerInfo.layers = secondaryViewConfigLayers.LayerData();
                }
            }
        }

        CHECK_XRCMD(xrEndFrame(Context().Session.Handle, &endFrameInfo));
    }

    void ImplementXrApp::RenderViewConfiguration(const std::scoped_lock<std::mutex>& proofOfSceneLock,
                                                 XrViewConfigurationType viewConfigurationType,
                                                 engine::CompositionLayers& layers) {
        // Locate the views in VIEW space to get the per-view offset from the VIEW "camera"
        XrViewState viewState{XR_TYPE_VIEW_STATE};
        std::vector<XrView>& views = m_viewConfigStates.at(viewConfigurationType).Views;
        {
            XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
            viewLocateInfo.viewConfigurationType = viewConfigurationType;
            viewLocateInfo.displayTime = m_currentFrameTime.PredictedDisplayTime;
            viewLocateInfo.space = m_viewSpace.Get();

            uint32_t viewCount = 0;
            CHECK_XRCMD(
                xrLocateViews(Context().Session.Handle, &viewLocateInfo, &viewState, (uint32_t)views.size(), &viewCount, views.data()));
            assert(viewCount == views.size());
            if (!xr::math::Pose::IsPoseValid(viewState)) {
                return;
            }
        }

        // Locate the VIEW space in the app space to get the "camera" pose and combine the per-view offsets with the camera pose.
        XrSpaceLocation viewLocation{XR_TYPE_SPACE_LOCATION};
        CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_appSpace.Get(), m_currentFrameTime.PredictedDisplayTime, &viewLocation));
        if (!xr::math::Pose::IsPoseValid(viewLocation)) {
            return;
        }

        for (XrView& view : views) {
            view.pose = xr::math::Pose::Multiply(view.pose, viewLocation.pose);
        }

        std::vector<std::shared_ptr<engine::QuadLayerObject>> underlays, overlays;
        {
            // Collect all quad layers in active scenes
            for (const std::unique_ptr<engine::Scene>& scene : m_scenes) {
                if (!scene->IsActive()) {
                    continue;
                }
                for (const std::shared_ptr<engine::QuadLayerObject>& quad : scene->GetQuadLayerObjects()) {
                    if (!quad->IsVisible()) {
                        continue;
                    }
                    if (quad->LayerGroup == engine::LayerGrouping::Underlay) {
                        underlays.push_back(quad);
                    } else if (quad->LayerGroup == engine::LayerGrouping::Overlay) {
                        overlays.push_back(quad);
                    }
                }
            }
        }

        for (const std::shared_ptr<engine::QuadLayerObject>& quad : underlays) {
            AppendQuadLayer(layers, quad.get());
        }

        m_projectionLayers.ForEachLayerWithLock([this, &layers, &views, viewConfigurationType](engine::ProjectionLayer& projectionLayer) {
            bool opaqueClearColor = (layers.LayerCount() == 0); // Only the first projection layer need opaque background
            opaqueClearColor &= (Context().Session.PrimaryViewConfigurationBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE);
            DirectX::XMStoreFloat4(&projectionLayer.Config().ClearColor,
                                   opaqueClearColor ? DirectX::XMColorSRGBToRGB(DirectX::Colors::CornflowerBlue)
                                                    : DirectX::Colors::Transparent);
            const bool shouldSubmitProjectionLayer =
                projectionLayer.Render(Context(), m_currentFrameTime, Context().AppSpace, views, m_scenes, viewConfigurationType);

            // Create the multi projection layer
            if (shouldSubmitProjectionLayer) {
                AppendProjectionLayer(layers, &projectionLayer, viewConfigurationType);
            }
        });

        for (const std::shared_ptr<engine::QuadLayerObject>& quad : overlays) {
            AppendQuadLayer(layers, quad.get());
        }
    }

} // namespace

namespace engine {
    std::unique_ptr<engine::XrApp> CreateXrApp(engine::XrAppConfiguration appConfiguration) {
        return std::make_unique<ImplementXrApp>(std::move(appConfiguration));
    }
} // namespace engine

