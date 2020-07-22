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
#include <XrSceneLib/XrApp.h>

std::unique_ptr<engine::Scene> TryCreateTitleScene(engine::Context& context);
std::unique_ptr<engine::Scene> TryCreateControllerModelScene(engine::Context& context);

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
    try {
        CHECK_HRCMD(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        auto on_exit = MakeScopeGuard([] { ::CoUninitialize(); });

        engine::XrAppConfiguration appConfig({"SampleSceneWin32", 1});
        appConfig.RequestedExtensions.push_back(XR_MSFT_CONTROLLER_MODEL_PREVIEW_EXTENSION_NAME);

        auto app = engine::CreateXrApp(appConfig);
        app->AddScene(TryCreateTitleScene(app->Context()));
        app->AddScene(TryCreateControllerModelScene(app->Context()));
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
