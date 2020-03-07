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

#include <SampleShared/FileUtility.h>
#include <SampleShared/DxUtility.h>
#include <SampleShared/Trace.h>

#include "App.h"
#include "CompositionLayers.h"
#include "VisibilityMask.h"
#include "SceneContext.h"

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

    const XrViewConfigurationType PrimaryViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    const XrViewConfigurationType MRCViewConfiguration = XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT;

    const std::vector<XrViewConfigurationType> SupportedViewConfigurations = {PrimaryViewConfiguration, MRCViewConfiguration};
    std::vector<XrEnvironmentBlendMode> SupportedBlendModes = {
        XR_ENVIRONMENT_BLEND_MODE_ADDITIVE,
        XR_ENVIRONMENT_BLEND_MODE_OPAQUE,
        XR_ENVIRONMENT_BLEND_MODE_ALPHA_BLEND,
    };

    std::vector<const char*> AppendSceneLibRequiredExtensions(const std::vector<const char*>& extensions) {
        std::vector<const char*> mergedExtensions = extensions;
        for (auto extension : {
                 XR_KHR_D3D11_ENABLE_EXTENSION_NAME,
                 XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME,
                 XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME,
                 XR_MSFT_SECONDARY_VIEW_CONFIGURATION_PREVIEW_EXTENSION_NAME,
                 XR_MSFT_FIRST_PERSON_OBSERVER_PREVIEW_EXTENSION_NAME,
             }) {
            if (std::find(mergedExtensions.begin(), mergedExtensions.end(), extension) == mergedExtensions.end()) {
                mergedExtensions.push_back(extension);
            }
        }
        return mergedExtensions;
    }

    bool IsRecommendedSwapchainSizeChanged(const std::vector<XrViewConfigurationView>& oldConfigs,
                                           const std::vector<XrViewConfigurationView>& newConfigs) {
        assert(oldConfigs.size() == newConfigs.size());
        for (size_t i = 0; i < oldConfigs.size(); i++) {
            if ((oldConfigs[i].recommendedImageRectWidth != newConfigs[i].recommendedImageRectWidth) ||
                (oldConfigs[i].recommendedImageRectHeight != newConfigs[i].recommendedImageRectHeight)) {
                return true;
            }
        }

        return false;
    }

    class ImplementXrApp : public XrApp {
    public:
        ImplementXrApp(const xr::NameVersion& appInfo, const std::vector<const char*>& extensions, bool singleThreadedGraphics);
        ~ImplementXrApp();

        ::SceneContext* SceneContext() const override {
            return m_sceneContext.get();
        }

        void AddScene(std::unique_ptr<Scene> scene) override;
        const std::vector<std::unique_ptr<Scene>>& Scenes() const override;

        void Run() override;
        void Stop() override;

        ::ProjectionLayers& ProjectionLayers() override {
            return m_projectionLayerCollection;
        }


    private:
        xr::SessionHandle m_session;
        xr::SpaceHandle m_sceneSpace;

        // Make sure we declare projection layers after instance to ensure projection layers are destroyed before instance
        ::ProjectionLayers m_projectionLayerCollection = ::ProjectionLayers([this]() {
            auto ensureSwapchainFormatSupportedFunction = [&](DXGI_FORMAT format, bool isDepth) {
                // Validate the app supports this format
                const std::vector<DXGI_FORMAT> appSupportedFormats =
                    isDepth ? SupportedDepthSwapchainFormats : SupportedColorSwapchainFormats;
                if ((std::find(appSupportedFormats.begin(), appSupportedFormats.end(), format) == appSupportedFormats.end())) {
                    throw std::runtime_error(fmt::format("Unsupported swapchain format: {}", format).c_str());
                }

                // Validate the runtime supports this format
                const std::vector<DXGI_FORMAT> formats = {format};
                const auto systemSupportedFormats = xr::EnumerateSwapchainFormats(m_session.Get());
                if (format != xr::PickSwapchainFormat(systemSupportedFormats, formats)) {
                    throw std::runtime_error(fmt::format("Unsupported swapchain format: {}", format).c_str());
                }
            };

            return std::make_unique<ProjectionLayer>(ensureSwapchainFormatSupportedFunction,
                                                     m_colorSwapchainFormatDefault,
                                                     m_depthSwapchainFormatDefault,
                                                     PrimaryViewConfiguration,
                                                     m_supportedSecondaryViewConfigurations);
        });

        std::unique_ptr<::SceneContext> m_sceneContext;
        std::unique_ptr<xr::ActionContext> m_actionContext;
        winrt::com_ptr<ID3D11Device> m_d3d11Device;

        struct ViewConfigStates {
            std::vector<XrViewConfigurationView> ViewConfigViews;
            std::vector<XrView> Views;
            XrEnvironmentBlendMode BlendMode;
            bool Active;
        };
        std::unordered_map<XrViewConfigurationType, ViewConfigStates> m_viewConfigStates;

        std::mutex m_sceneMutex;
        std::vector<std::unique_ptr<Scene>> m_scenes;

        std::function<void(XrFrameEndInfo* endFrameInfo)> m_endFrameCallback;

        std::atomic<bool> m_sessionRunning{false};
        std::atomic<bool> m_appLoopRunning{false};

        std::thread m_renderThread;
        std::atomic<bool> m_renderThreadRunning{false};

        std::condition_variable m_frameReadyToRenderNotify;
        bool m_frameReadyToRender{false};
        FrameTime m_currentFrameTime;

        DXGI_FORMAT m_colorSwapchainFormatDefault;
        DXGI_FORMAT m_depthSwapchainFormatDefault;

        std::unique_ptr<IVisibilityMask> m_visibilityMask;

        std::vector<XrViewConfigurationType> m_supportedSecondaryViewConfigurations;

    private:
        bool ProcessEvents();
        void StartRenderThreadIfNotRunning();
        void StopRenderThreadIfRunning();
        void UpdateFrame();
        void RenderFrame();

        void BeginSession();
        void EndSession();
    };

    ImplementXrApp::ImplementXrApp(const xr::NameVersion& appInfo,
                                   const std::vector<const char*>& extensions,
                                   bool singleThreadedGraphics) {

        std::vector<const char*> mergedExtensions = AppendSceneLibRequiredExtensions(extensions);

        xr::InstanceContext instance(appInfo, {"XrSceneLib", 1}, mergedExtensions);

        std::optional<xr::SystemContext> systemOpt;
        while (!(systemOpt = instance.TryGetSystem(XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY,
                                                   XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,
                                                   {XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT}))) {
            DEBUG_PRINT("Waiting for system plugin ...");
            std::this_thread::sleep_for(500ms);
        }
        const xr::SystemContext& system = systemOpt.value();

        // Create the D3D11 device for the adapter associated with the system.
        _Analysis_assume_(instance.Extensions.xrGetD3D11GraphicsRequirementsKHR != nullptr);
        XrGraphicsRequirementsD3D11KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
        CHECK_XRCMD(instance.Extensions.xrGetD3D11GraphicsRequirementsKHR(instance.Handle(), system.Id, &graphicsRequirements));
        const winrt::com_ptr<IDXGIAdapter1> adapter = sample::dx::GetAdapter(graphicsRequirements.adapterLuid);

        // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        std::vector<D3D_FEATURE_LEVEL> featureLevels = {
            D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0};
        featureLevels.erase(std::remove_if(std::begin(featureLevels),
                                           std::end(featureLevels),
                                           [&](D3D_FEATURE_LEVEL fl) { return fl < graphicsRequirements.minFeatureLevel; }),
                            std::end(featureLevels));
        if (featureLevels.size() == 0) {
            throw std::runtime_error("Unsupported minimum feature level!");
        }

        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        sample::dx::CreateD3D11DeviceAndContext(adapter.get(), featureLevels, singleThreadedGraphics, device.put(), deviceContext.put());

        XrGraphicsBindingD3D11KHR d3d11Binding{};
        d3d11Binding.type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
        d3d11Binding.device = device.get();

        XrSessionCreateInfo createInfo{XR_TYPE_SESSION_CREATE_INFO};
        createInfo.next = &d3d11Binding;
        createInfo.systemId = system.Id;

        CHECK_XRCMD(xrCreateSession(instance.Handle(), &createInfo, m_session.Put()));

        // Cache view related properties for each view config
        for (const auto& [type, viewConfiguration] : system.ViewConfigurations) {
            if (std::find(SupportedViewConfigurations.begin(), SupportedViewConfigurations.end(), type) ==
                SupportedViewConfigurations.end()) {
                continue; // Not supported by this app
            }
            m_viewConfigStates[type].ViewConfigViews = xr::EnumerateViewConfigurationViews(instance.Handle(), system.Id, type);
            m_viewConfigStates[type].Views.resize(m_viewConfigStates[type].ViewConfigViews.size(), {XR_TYPE_VIEW});
            m_viewConfigStates[type].BlendMode = xr::PickEnvironmentBlendMode(viewConfiguration.BlendModes, SupportedBlendModes);

            const bool isPrimaryViewConfiguration = (type == PrimaryViewConfiguration);
            m_viewConfigStates[type].Active = isPrimaryViewConfiguration; // Primary view configuration is always active
            if (!isPrimaryViewConfiguration) {
                m_supportedSecondaryViewConfigurations.push_back(type);
            }
        }

        const auto systemSupportedFormats = xr::EnumerateSwapchainFormats(m_session.Get());
        m_colorSwapchainFormatDefault = xr::PickSwapchainFormat(systemSupportedFormats, SupportedColorSwapchainFormats);
        m_depthSwapchainFormatDefault = xr::PickSwapchainFormat(systemSupportedFormats, SupportedDepthSwapchainFormats);

        // Create main app space
        XrReferenceSpaceCreateInfo spaceCreateInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        spaceCreateInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        spaceCreateInfo.poseInReferenceSpace = xr::math::Pose::Identity();
        CHECK_XRCMD(xrCreateReferenceSpace(m_session.Get(), &spaceCreateInfo, m_sceneSpace.Put()));

        Pbr::Resources pbrResources = sample::InitializePbrResources(device.get());
        m_actionContext = std::make_unique<xr::ActionContext>(instance.Handle());

        m_sceneContext = std::make_unique<::SceneContext>(std::move(instance),
                                                          std::move(system),
                                                          m_session.Get(),
                                                          m_sceneSpace.Get(),
                                                          std::move(pbrResources),
                                                          device,
                                                          deviceContext,
                                                          *m_actionContext,
                                                          m_viewConfigStates.at(PrimaryViewConfiguration).BlendMode);

        m_projectionLayerCollection.Resize(1, true /*forceReset*/);

        // Only create visibility mask if app has enabled the extension
        if (m_sceneContext->Extensions.SupportsVisibilityMask) {
            m_visibilityMask = CreateStereoVisibilityMask(m_sceneContext.get());
        }
    }

    ImplementXrApp::~ImplementXrApp() {
        StopRenderThreadIfRunning();

        {
            std::lock_guard lock(m_sceneMutex);
            m_scenes.clear();
        }
    }

    void ImplementXrApp::AddScene(std::unique_ptr<Scene> scene) {
        if (!scene) {
            return; // Some scenes might skip creation due to extension unavailability.
        }

        std::lock_guard lock(m_sceneMutex);
        assert(!m_appLoopRunning); // Cannot add scene after app loop is running

        m_scenes.push_back(std::move(scene));
    }

    const std::vector<std::unique_ptr<Scene>>& ImplementXrApp::Scenes() const {
        return m_scenes;
    }

    void ImplementXrApp::Run() {
        ::SetThreadDescription(::GetCurrentThread(), L"App Thread");

        m_appLoopRunning = true;
        m_actionContext->AttachActionsToSession(m_session.Get());

        while (m_appLoopRunning) {
            if (!ProcessEvents()) {
                break;
            }

            if (m_sessionRunning) {
                StartRenderThreadIfNotRunning();

                UpdateFrame();

                m_frameReadyToRender = true;
                m_frameReadyToRenderNotify.notify_all();
            } else {
                std::this_thread::sleep_for(0.1s);
            }
        }
    }

    void ImplementXrApp::StartRenderThreadIfNotRunning() {
        bool alreadyRunning = false;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, true)) {
            m_renderThread = std::thread([this]() {
                ::SetThreadDescription(::GetCurrentThread(), L"Render Thread");

                std::mutex frameReadyToRenderMutex;
                std::unique_lock<std::mutex> lock(frameReadyToRenderMutex);

                while (m_renderThreadRunning && m_sessionRunning) {
                    m_frameReadyToRenderNotify.wait(lock, [this] { return m_frameReadyToRender; });
                    m_frameReadyToRender = false;

                    if (!m_renderThreadRunning || !m_sessionRunning) {
                        break; // check again after waiting
                    }

                    RenderFrame();
                }
            });
        }
    }

    void ImplementXrApp::StopRenderThreadIfRunning() {
        bool alreadyRunning = true;
        if (m_renderThreadRunning.compare_exchange_strong(alreadyRunning, false)) {
            // Notify "frameReadyToRender" with "renderThreadRunning = false" to exit render thread.
            m_frameReadyToRender = true;
            m_frameReadyToRenderNotify.notify_all();
            if (m_renderThread.joinable()) {
                m_renderThread.join();
            }
        }
    }

    void ImplementXrApp::Stop() {
        if (m_sessionRunning) {
            CHECK_XRCMD(xrRequestExitSession(m_sceneContext->Session));
        } else {
            m_appLoopRunning = false; // quit app message loop
        }
    }

    bool ImplementXrApp::ProcessEvents() {
        XrEventDataBuffer eventData;
        while (true) {
            eventData.type = XR_TYPE_EVENT_DATA_BUFFER;
            eventData.next = nullptr;
            XrResult res = xrPollEvent(m_sceneContext->Instance.Handle(), &eventData);
            CHECK_XRCMD(res);
            if (res == XR_EVENT_UNAVAILABLE) {
                return true;
            } else if (eventData.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                auto sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&eventData);
                if (sessionStateChanged->session == m_sceneContext->Session) {
                    m_sceneContext->SessionState = sessionStateChanged->state;
                    switch (m_sceneContext->SessionState) {
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
            } else if (eventData.type == XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED) {
                if (m_sessionRunning) {
                    std::lock_guard lock(m_sceneMutex);
                    for (const std::unique_ptr<Scene>& scene : m_scenes) {
                        scene->NotifyInteractionProfileChangedEvent();
                    }
                }
            } else if (eventData.type == XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING) {
                auto spaceChangingEvent = reinterpret_cast<XrEventDataReferenceSpaceChangePending*>(&eventData);
                std::optional<XrPosef> pose{};
                if (spaceChangingEvent->poseValid) {
                    pose = spaceChangingEvent->poseInPreviousSpace;
                }

                if (m_sessionRunning) {
                    std::lock_guard lock(m_sceneMutex);
                    for (const std::unique_ptr<Scene>& scene : m_scenes) {
                        scene->NotifySpaceChangingEvent(spaceChangingEvent->referenceSpaceType, spaceChangingEvent->changeTime, pose);
                    }
                }
            } else if (eventData.type == XR_TYPE_EVENT_DATA_VISIBILITY_MASK_CHANGED_KHR) {
                if (m_visibilityMask) {
                    auto visibilityMaskChangeEvent = reinterpret_cast<XrEventDataVisibilityMaskChangedKHR*>(&eventData);
                    assert(visibilityMaskChangeEvent->viewConfigurationType == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO);

                    m_visibilityMask->NotifyMaskChanged(visibilityMaskChangeEvent->viewIndex);
                }
            }
        }
    }

    void ImplementXrApp::BeginSession() {
        XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
        sessionBeginInfo.primaryViewConfigurationType = PrimaryViewConfiguration;

        // Need this variable to be outside the scope of the if block below to be properly referenced by sessionBeginInfo.next.
        // Otherwise, it gets deallocated when if block ends
        XrSessionBeginSecondaryViewConfigurationInfoMSFT sessionBeginSecondaryViewConfigInfo{
            XR_TYPE_SESSION_BEGIN_SECONDARY_VIEW_CONFIGURATION_INFO_MSFT,
            nullptr,
            (uint32_t)m_supportedSecondaryViewConfigurations.size(),
            m_supportedSecondaryViewConfigurations.data()};

        if (!m_supportedSecondaryViewConfigurations.empty()) {
            sessionBeginInfo.next = &sessionBeginSecondaryViewConfigInfo;
        }

        CHECK_XRCMD(xrBeginSession(m_sceneContext->Session, &sessionBeginInfo));
        m_sessionRunning = true;
    }

    void ImplementXrApp::EndSession() {
        StopRenderThreadIfRunning();
        m_sessionRunning = false;
        CHECK_XRCMD(xrEndSession(m_sceneContext->Session));
    }

    void ImplementXrApp::UpdateFrame() {
        XrFrameWaitInfo waitFrameInfo{XR_TYPE_FRAME_WAIT_INFO};
        XrFrameState frameState{XR_TYPE_FRAME_STATE};

        // Need these variables to be outside the scope of the if block below to be properly referenced by sessionBeginInfo.next.
        // Otherwise, it gets deallocated when if block ends
        std::vector<XrSecondaryViewConfigurationStateMSFT> secondaryViewConfigStateData;
        for (XrViewConfigurationType type : m_supportedSecondaryViewConfigurations) {
            secondaryViewConfigStateData.emplace_back(
                XrSecondaryViewConfigurationStateMSFT{XR_TYPE_SECONDARY_VIEW_CONFIGURATION_STATE_MSFT, nullptr, type, XR_FALSE});
        }
        XrFrameSecondaryViewConfigurationsStateMSFT secondaryViewConfigStates{XR_TYPE_FRAME_SECONDARY_VIEW_CONFIGURATIONS_STATE_MSFT,
                                                                              nullptr,
                                                                              (uint32_t)secondaryViewConfigStateData.size(),
                                                                              secondaryViewConfigStateData.data()};
        if (!secondaryViewConfigStateData.empty()) {
            frameState.next = &secondaryViewConfigStates;
        }
        CHECK_XRCMD(xrWaitFrame(m_sceneContext->Session, &waitFrameInfo, &frameState));

        m_actionContext->SyncActions(m_sceneContext->Session);

        {
            std::lock_guard lock(m_sceneMutex);
            for (const XrSecondaryViewConfigurationStateMSFT& state : secondaryViewConfigStateData) {
                const bool active = static_cast<bool>(state.active);
                const XrViewConfigurationType secondaryViewConfigType = state.viewConfigurationType;
                if (m_viewConfigStates.at(state.viewConfigurationType).Active != active) {
                    sample::Trace(
                        "ViewConfiguration {} becomes {}\n", xr::ToString(secondaryViewConfigType), active ? "Active" : "Inactive");

                    if (active) {
                        // When a secondary view configuration is turned active, detect if recommended swapchain size is changed.
                        // If so, reset resources in layers related to this secondary view configuration.
                        std::vector<XrViewConfigurationView> newViewConfigViews = xr::EnumerateViewConfigurationViews(
                            m_sceneContext->Instance.Handle(), m_sceneContext->System.Id, secondaryViewConfigType);
                        if (IsRecommendedSwapchainSizeChanged(m_viewConfigStates.at(secondaryViewConfigType).ViewConfigViews,
                                                              newViewConfigViews)) {
                            m_projectionLayerCollection.ForEachLayerWithLock(
                                [secondaryViewConfigType](auto&& layer) { layer.Config(secondaryViewConfigType).ForceReset = true; });

                            m_viewConfigStates.at(secondaryViewConfigType).ViewConfigViews = newViewConfigViews;
                        }
                    }

                    m_viewConfigStates[state.viewConfigurationType].Active = active;
                }
            }

            m_currentFrameTime.Update(frameState);
            for (auto& scene : m_scenes) {
                if (scene->IsActive()) {
                    scene->Update(m_currentFrameTime);
                }
            }

            m_projectionLayerCollection.ForEachLayerWithLock([this](auto&& layer) {
                for (auto& [viewConfigType, states] : m_viewConfigStates) {
                    layer.PrepareRendering(m_sceneContext.get(), viewConfigType, states.ViewConfigViews, states.Active);
                }
            });
        }
    }

    void ImplementXrApp::RenderFrame() {
        XrFrameBeginInfo beginFrameDescription{XR_TYPE_FRAME_BEGIN_INFO};
        CHECK_XRCMD(xrBeginFrame(m_sceneContext->Session, &beginFrameDescription));

        size_t viewConfigCount = 1 + m_supportedSecondaryViewConfigurations.size();
        std::vector<CompositionLayers> layersForAllViewConfigs(viewConfigCount); // Need same lifetime as XrFrameEndInfo;

        XrFrameEndInfo endFrameInfo{XR_TYPE_FRAME_END_INFO};
        endFrameInfo.environmentBlendMode = m_sceneContext->PrimaryViewConfigEnvironmentBlendMode;
        endFrameInfo.displayTime = m_currentFrameTime.PredictedDisplayTime;

        std::vector<XrViewConfigurationType> allViewConfigurationTypes{PrimaryViewConfiguration};
        std::vector<XrSecondaryViewConfigurationLayerInfoMSFT> secondaryViewConfigLayerInfos;
        for (XrViewConfigurationType viewConfigurationType : m_supportedSecondaryViewConfigurations) {
            secondaryViewConfigLayerInfos.emplace_back(
                XrSecondaryViewConfigurationLayerInfoMSFT{XR_TYPE_SECONDARY_VIEW_CONFIGURATION_LAYER_INFO_MSFT,
                                                          nullptr,
                                                          viewConfigurationType,
                                                          m_viewConfigStates.at(viewConfigurationType).BlendMode});
            allViewConfigurationTypes.push_back(viewConfigurationType);
        }

        assert(allViewConfigurationTypes.size() == secondaryViewConfigLayerInfos.size() + 1 &&
               allViewConfigurationTypes.size() == viewConfigCount);

        //  Need same lifetime as XrFrameEndInfo
        const XrFrameEndSecondaryViewConfigurationInfoMSFT frameEndSecondaryViewConfigInfo{
            XR_TYPE_FRAME_END_SECONDARY_VIEW_CONFIGURATION_INFO_MSFT,
            nullptr,
            (uint32_t)secondaryViewConfigLayerInfos.size(),
            secondaryViewConfigLayerInfos.data()};
        if (!secondaryViewConfigLayerInfos.empty()) {
            endFrameInfo.next = &frameEndSecondaryViewConfigInfo;
        }

        if (m_currentFrameTime.ShouldRender) {
            for (size_t i = 0; i < viewConfigCount; i++) {
                const XrViewConfigurationType viewConfigurationType = allViewConfigurationTypes.at(i);
                CompositionLayers& layers = layersForAllViewConfigs.at(i);
                ViewConfigStates& states = m_viewConfigStates.at(viewConfigurationType);

                // Guarding states.Active change and swapchain creation as a result in UpdateFrame()
                std::lock_guard sceneLock(m_sceneMutex);
                if (states.Active) {
                    XrViewLocateInfo viewLocateInfo{XR_TYPE_VIEW_LOCATE_INFO};
                    viewLocateInfo.viewConfigurationType = viewConfigurationType;
                    viewLocateInfo.displayTime = m_currentFrameTime.PredictedDisplayTime;
                    viewLocateInfo.space = m_sceneContext->SceneSpace;

                    uint32_t viewCount = 0;
                    XrViewState viewState{XR_TYPE_VIEW_STATE};

                    std::vector<XrView>& views = states.Views;
                    CHECK_XRCMD(xrLocateViews(
                        m_sceneContext->Session, &viewLocateInfo, &viewState, (uint32_t)views.size(), &viewCount, views.data()));
                    if (xr::math::Pose::IsPoseValid(viewState)) {
                        std::vector<std::shared_ptr<QuadLayerObject>> underlays, overlays;
                        {
                            // Collect all quad layers in active scenes
                            for (const std::unique_ptr<Scene>& scene : m_scenes) {
                                if (!scene->IsActive()) {
                                    continue;
                                }
                                for (const std::shared_ptr<QuadLayerObject>& quad : scene->GetQuadLayerObjects()) {
                                    if (!quad->IsRenderable()) {
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

                        m_projectionLayerCollection.ForEachLayerWithLock(
                            [this, &layers, &views, viewConfigurationType](ProjectionLayer& projectionLayer) {
                                const bool shouldSubmitProjectionLayer = projectionLayer.Render(m_sceneContext.get(),
                                                                                                m_currentFrameTime,
                                                                                                m_sceneContext->SceneSpace,
                                                                                                views,
                                                                                                m_scenes,
                                                                                                m_visibilityMask.get(),
                                                                                                viewConfigurationType);

                                // Create the multi projection layer
                                if (shouldSubmitProjectionLayer) {
                                    AppendProjectionLayer(layers, &projectionLayer, viewConfigurationType);
                                }
                            });

                        for (const std::shared_ptr<QuadLayerObject>& quad : overlays) {
                            AppendQuadLayer(layers, quad.get());
                        }
                    }

                    if (viewConfigurationType == PrimaryViewConfiguration) {
                        endFrameInfo.layerCount = layers.LayerCount();
                        endFrameInfo.layers = layers.LayerData();
                    } else {
                        const size_t layerIndex = i - 1;
                        secondaryViewConfigLayerInfos.at(layerIndex).layerCount = layers.LayerCount();
                        secondaryViewConfigLayerInfos.at(layerIndex).layers = layers.LayerData();
                    }
                }
            }
        }

        if (m_endFrameCallback) {
            m_endFrameCallback(&endFrameInfo);
        }

        CHECK_XRCMD(xrEndFrame(m_sceneContext->Session, &endFrameInfo));
    }
} // namespace

std::unique_ptr<XrApp> CreateXrApp(const xr::NameVersion& appInfo,
                                   const std::vector<const char*>& extensions,
                                   bool singleThreadedGraphics) {
    return std::make_unique<ImplementXrApp>(appInfo, extensions, singleThreadedGraphics);
}

