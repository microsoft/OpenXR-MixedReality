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
#include <windowsx.h>
#include <shellapi.h>
#include <XrSceneLib/XrApp.h>
#include "Resource.h"

std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateControllerModelScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateControllerActionsScene(engine::Context& context);

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
        appConfig.RequestedExtensions.push_back(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        appConfig.RequestedExtensions.push_back(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);

        app = engine::CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateControllerModelScene(app->Context()));
        app->AddScene(TryCreateControllerActionsScene(app->Context()));
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

void TrapCursorIfMouseLeave(HWND hwnd) {
    RECT rc;
    ::GetWindowRect(hwnd, &rc);

    POINT pt;
    ::GetCursorPos(&pt);

    if (!::PtInRect(&rc, pt)) {
        ::SetCursorPos((rc.left + rc.right) / 2, (rc.top + rc.bottom) / 2);
    }
}

INT_PTR CALLBACK DialogWinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    constexpr UINT_PTR IDT_MOUSETRAP = 1;

    switch (message) {
    case WM_INITDIALOG:
        // Setup a recurring timer to move mouse back when it moves out of this window.
        // It's a simple way to keep WM_MOUSEMOVE event to remain sending to this window.
        ::SetTimer(hwnd, IDT_MOUSETRAP, 100, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            ExitVR();
            EnterVR();
        } else if (LOWORD(wParam) == IDCANCEL) {
            ::EndDialog(hwnd, 0);
        }
        break;

    case WM_TIMER:
        if (wParam == IDT_MOUSETRAP && hwnd == ::GetForegroundWindow()) {
            TrapCursorIfMouseLeave(hwnd);
        }
        break;

    case WM_MOUSEMOVE:
        ::SetWindowText(hwnd, fmt::format(L"Mouse pos: {}, {}", GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)).c_str());
        break;

    case WM_DESTROY:
        ExitVR();
        ::KillTimer(hwnd, IDT_MOUSETRAP);
        ::PostQuitMessage(0);
        break;
    }

    return (INT_PTR)FALSE;
}
