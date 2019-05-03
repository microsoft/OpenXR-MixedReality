#pragma once

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <array>
#include <map>
#include <algorithm>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "advapi32.lib")

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "windowscodecs.lib")

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "..\XrUtility\XrError.h"
#include "..\XrUtility\XrHandle.h"
#include "..\XrUtility\XrMath.h"
