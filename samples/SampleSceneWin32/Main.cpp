// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <windowsx.h>
#include <shellapi.h>
#include <XrSceneLib/XrApp.h>
#include "Resource.h"

std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateControllerModelScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateControllerActionsScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateHandTrackingScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateTrackingStateScene(engine::Context& context);

// Global Variables:
std::thread sceneThread;
std::unique_ptr<engine::XrApp> app;

void EnterVR();
INT_PTR CALLBACK DialogWinProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
    int nArgs;
    LPWSTR* szArglist = ::CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (nArgs >= 2 && ::_wcsicmp(szArglist[1], L"-openxr") == 0) {
        EnterVR();
    }

    DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOGBOX), NULL, DialogWinProc);
    return 0;
}

void EnterVR() {
    sceneThread = std::thread([] {
        CHECK_HRCMD(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        auto on_exit = MakeScopeGuard([] { ::CoUninitialize(); });

        engine::XrAppConfiguration appConfig({"SampleSceneWin32", 1});
        appConfig.RequestedExtensions.push_back(XR_MSFT_CONTROLLER_MODEL_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME);

        // NOTE: Uncomment a filter below to test specific action binding of given profile.
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/microsoft/hand_interaction");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/microsoft/motion_controller");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/oculus/touch_controller");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/hp/mixed_reality_controller");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/samsung/odyssey_controller");
        // appConfig.InteractionProfilesFilter.push_back("/interaction_profiles/khr/simple_controller");

        app = engine::CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateControllerModelScene(app->Context()));
        app->AddScene(TryCreateControllerActionsScene(app->Context()));
        app->AddScene(TryCreateHandTrackingScene(app->Context()));
        app->AddScene(TryCreateTrackingStateScene(app->Context()));
        app->Run();
        app = nullptr;
    });
}

void ExitVR() {
    if (sceneThread.joinable()) {
        if (app) {
            app->Stop();
        }
        sceneThread.join();
    }
}

// https://docs.microsoft.com/en-us/windows/win32/menurc/using-cursors#confining-a-cursor
// The effect of ClipCursor is restricted to this process.
// When the focus is switched to another window, the cursor is free to full screen.
// Therefore redo this operation when this window becomes active or being moved.
void ConfineCursor(HWND hwnd) {
    RECT rc;
    ::GetWindowRect(hwnd, &rc);
    ::ClipCursor(&rc);
}

INT_PTR CALLBACK DialogWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            ExitVR();
            EnterVR();
        } else if (LOWORD(wParam) == IDCANCEL) {
            ::EndDialog(hwnd, 0);
        }
        break;

    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) {
            ConfineCursor(hwnd);
        }
        break;

    case WM_EXITSIZEMOVE:
        ConfineCursor(hwnd);
        break;

    case WM_MOUSEMOVE:
        ::SetWindowText(hwnd, fmt::format(L"Mouse pos: {}, {}", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)).c_str());
        break;

    case WM_DESTROY:
        ExitVR();
        ::PostQuitMessage(0);
        break;
    }

    return (INT_PTR)FALSE;
}
