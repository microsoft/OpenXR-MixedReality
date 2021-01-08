// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/XrApp.h>

std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateEyeGazeInteractionScene(engine::Context& context);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    try {
        CHECK_HRCMD(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        auto on_exit = MakeScopeGuard([] { ::CoUninitialize(); });

        engine::XrAppConfiguration appConfig({"EyeGazeInteractionUwp", 1});
        appConfig.RequestedExtensions.push_back(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME);

        auto app = CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateEyeGazeInteractionScene(app->Context()));
        app->Run();
    } catch (const std::exception& ex) {
        sample::Trace("Unhandled Exception: {}", ex.what());
        return 1;
    } catch (...) {
        sample::Trace(L"Unhandled Exception");
        return 1;
    }

    return 0;
}
