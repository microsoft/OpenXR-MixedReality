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
