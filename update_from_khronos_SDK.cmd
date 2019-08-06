@REM Update the openxr sdk from https://github.com/KhronosGroup/OpenXR-SDK.git
@echo off
setlocal ENABLEDELAYEDEXPANSION

set KhronosSdkRoot=%~1
set TargetRoot=%~dp0

if not exist "%KhronosSdkRoot%" (
    echo The source repo path '%KhronosSdkRoot%' does not exist.
    goto :error_out
)

call :copy_files %KhronosSdkRoot%\include\openxr\*.h %TargetRoot%\include\openxr\
call :copy_files %KhronosSdkRoot%\src\loader\*.c* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\loader\*.h* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\extra_algorithms.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\hex_and_handles.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\filesystem_utils.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\loader_interfaces.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\platform_utils.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\common\xr_dependencies.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\xr_generated_dispatch_table.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\loader\xr_generated_loader.* %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\loader\loader.rc %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\external\jsoncpp\dist\jsoncpp.cpp %TargetRoot%\loader\
call :copy_files %KhronosSdkRoot%\src\external\jsoncpp\dist\json\json-forwards.h %TargetRoot%\loader\json\
call :copy_files %KhronosSdkRoot%\src\external\jsoncpp\dist\json\json.h %TargetRoot%\loader\json\

REM SUCCEEDED

:succeeded
echo. &echo ---------------------------------------------------------
echo SUCCEEDED : Script is completed.
exit /b 0

:error_out
echo. &echo ---------------------------------------------------------
echo ERROR : Script is aborted!
exit /b 1

:copy_files
@echo xcopy /y %*
echo F | xcopy /y %*
if errorlevel 1 goto :error_out
goto :EOF

:delete_file
@echo del /q %*
del /q %*
if errorlevel 1 goto :error_out
goto :EOF