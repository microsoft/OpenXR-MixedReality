//*********************************************************//
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

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
constexpr const char* ProgramName = "BasicXrApp_win32";
#else
constexpr const char* ProgramName = "BasicXrApp_uwp";
#endif

int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::unique_ptr<xr::IGraphicsPlugin> graphics = xr::CreateCubeGraphics();
    std::unique_ptr<xr::IOpenXrProgram> program = xr::CreateOpenXrProgram(ProgramName, std::move(graphics));
    program->Run();
    return 0;
}
