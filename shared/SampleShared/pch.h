// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <sdkddkver.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <wincodec.h>

#include <winrt/base.h> // for winrt::com_ptr

#include <algorithm>
#include <atomic>
#include <chrono>
#include <initializer_list>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <XrUtility/XrError.h>
#include <XrUtility/XrHandle.h>
