// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/XrApp.h>

std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateThreeSpacesScene(engine::Context& context);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        CHECK_HRCMD(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        auto on_exit = MakeScopeGuard([] { ::CoUninitialize(); });

        engine::XrAppConfiguration appConfig({"ThreeSpacesUwp", 1});
        appConfig.RequestedExtensions.push_back(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_HAND_INTERACTION_EXTENSION_NAME);

        auto app = engine::CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateThreeSpacesScene(app->Context()));
        app->Run();
    } catch (const std::exception& ex) {
        sample::Trace("Unhandled Exception: {}", ex.what());
        return 1;
    } catch (...) {
        sample::Trace("Unhandled Exception");
        return 1;
    }

    return 0;
}
