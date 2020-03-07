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

std::unique_ptr<Scene> CreateTitleScene(SceneContext* sceneContext);

int APIENTRY wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int) {
    try {
        CHECK_HRCMD(::CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        auto on_exit = MakeScopeGuard([] { ::CoUninitialize(); });

        const std::vector<const char*> requiredExtensions = {};

        auto app = CreateXrApp({"SampleSceneWin32", 1}, requiredExtensions);
        app->AddScene(CreateTitleScene(app->SceneContext()));
        app->Run();
    } catch (const std::exception& ex) {
        sample::Trace("Unhandled Exception: {}\n", ex.what());
        return 1;
    } catch (...) {
        sample::Trace(L"Unhandled Exception\n");
        return 1;
    }

    return 0;
}
