// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <XrSceneLib/XrApp.h>
std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateOrbitScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateHandTrackingScene(engine::Context& context);

#include <Unknwn.h> // Required to interop with IUnknown. Must be included before C++/WinRT headers.
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.ApplicationModel.Preview.Holographic.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Text.Core.h>
#include <winrt/Windows.UI.ViewManagement.h>
#include <winrt/Windows.Graphics.Holographic.h>

namespace windows {
    using namespace winrt::Windows::ApplicationModel::Activation;
    using namespace winrt::Windows::ApplicationModel::Core;
    using namespace winrt::Windows::UI::Core;
    using namespace winrt::Windows::UI::Text::Core;
    using namespace winrt::Windows::UI::ViewManagement;
    using namespace winrt::Windows::Graphics::Holographic;
    using namespace winrt::Windows::ApplicationModel::Preview::Holographic;
} // namespace windows

namespace {
    std::unique_ptr<engine::XrApp> CreateUwpXrApp(XrHolographicWindowAttachmentMSFT&& holographicWindowAttachment) {
        engine::XrAppConfiguration appConfig({"SampleSceneUwp", 2});
        appConfig.HolographicWindowAttachment = std::move(holographicWindowAttachment);

        appConfig.RequestedExtensions.push_back(XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);

        // NOTE: Uncomment a filter below to test specific action binding of given profile.
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/microsoft/hand_interaction");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/khr/simple_controller");

        auto app = CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateOrbitScene(app->Context()));
        app->AddScene(TryCreateHandTrackingScene(app->Context()));
        return app;
    }

    struct AppView : winrt::implements<AppView, windows::IFrameworkView> {
        void Initialize(windows::CoreApplicationView const& applicationView) {
            sample::Trace("IFrameworkView::Initialize");
            applicationView.Activated({this, &AppView::OnActivated});
        }

        void Load(winrt::hstring const& entryPoint) {
            sample::Trace("IFrameworkView::Load entryPoint : {}", winrt::to_string(entryPoint).c_str());
        }

        void Uninitialize() {
            sample::Trace("IFrameworkView::Uninitialize");
        }

        void OnActivated(windows::CoreApplicationView const&, windows::IActivatedEventArgs const& args) {
            if (args.Kind() == windows::ActivationKind::Protocol) {
                windows::ProtocolActivatedEventArgs eventArgs{args.as<windows::ProtocolActivatedEventArgs>()};
                sample::Trace("Protocol uri : {}", winrt::to_string(eventArgs.Uri().RawUri()).c_str());
            }

            // Inspecting whether the application is launched from within holographic shell or from desktop.
            if (windows::HolographicApplicationPreview::IsHolographicActivation(args)) {
                sample::Trace("App activation is targeted at the holographic shell.");
            } else {
                sample::Trace("App activation is targeted at the desktop.");
            }

            // NOTE: CoreWindow will be activated later after the HolographicSpace has been created.
        }

        void Run() {
            sample::Trace("IFrameworkView::Run");

            // Creating a HolographicSpace before activating the CoreWindow to make it a holographic window
            windows::CoreWindow window = windows::CoreWindow::GetForCurrentThread();
            windows::HolographicSpace holographicSpace = windows::HolographicSpace::CreateForCoreWindow(window);
            window.Activate();

            XrHolographicWindowAttachmentMSFT holographicWindowAttachment{XR_TYPE_HOLOGRAPHIC_WINDOW_ATTACHMENT_MSFT};
            holographicWindowAttachment.coreWindow = window.as<::IUnknown>().get();
            holographicWindowAttachment.holographicSpace = holographicSpace.as<::IUnknown>().get();

            std::unique_ptr<engine::XrApp> app = CreateUwpXrApp(std::move(holographicWindowAttachment));

            while (!m_windowClosed && app->Step()) {
                window.Dispatcher().ProcessEvents(windows::CoreProcessEventsOption::ProcessAllIfPresent);
            }
        }

        void SetWindow(windows::CoreWindow const& window) {
            sample::Trace("IFrameworkView::SetWindow");

            InitializeTextEditingContext();
            window.KeyDown({this, &AppView::OnKeyDown});
            window.Closed({this, &AppView::OnWindowClosed});
        }

        void InitializeTextEditingContext() {
            // This sample customizes the text input pane with manual display policy and email address scope.
            windows::CoreTextServicesManager manager = windows::CoreTextServicesManager::GetForCurrentView();
            windows::CoreTextEditContext editingContext = manager.CreateEditContext();
            editingContext.InputPaneDisplayPolicy(windows::CoreTextInputPaneDisplayPolicy::Manual);
            editingContext.InputScope(windows::CoreTextInputScope::EmailAddress);
        }

        void OnKeyDown(windows::CoreWindow const& sender, windows::KeyEventArgs const& args) {
            sample::Trace("OnKeyDown : 0x{:x}", args.VirtualKey());

            // This sample toggles the software keyboard in HMD using space key
            if (args.VirtualKey() == winrt::Windows::System::VirtualKey::Space) {
                windows::InputPane inputPane = windows::InputPane::GetForCurrentView();
                if (inputPane.Visible()) {
                    const bool hidden = inputPane.TryHide();
                    sample::Trace("InputPane::TryHide() -> {}", hidden);
                } else {
                    const bool shown = inputPane.TryShow();
                    sample::Trace("InputPane::TryShow() -> {}", shown);
                }
            }
        }

        void OnWindowClosed(windows::CoreWindow const& sender, windows::CoreWindowEventArgs const& args) {
            m_windowClosed = true;
        }

    private:
        bool m_windowClosed{false};
    };

    struct AppViewSource : winrt::implements<AppViewSource, windows::IFrameworkViewSource> {
        windows::IFrameworkView CreateView() {
            return winrt::make<AppView>();
        }
    };
} // namespace

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
    windows::CoreApplication::Run(winrt::make<AppViewSource>());
}
