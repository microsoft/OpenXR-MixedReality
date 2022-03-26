// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <sdkddkver.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <unknwn.h>
#include <winrt/base.h> // for winrt::com_ptr

#include <array>
#include <atomic>
#include <chrono>
#include <chrono>
#include <exception>
#include <future>
#include <initializer_list>
#include <memory>
#include <pathcch.h>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#define XR_NO_PROTOTYPES
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#define ENABLE_GLOBAL_XR_DISPATCH_TABLE
#include <XrUtility/XrDispatchTable.h>
#include <XrUtility/XrError.h>
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrToString.h>

#include <SampleShared/Trace.h>
#include <SampleShared/ScopeGuard.h>

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY
#endif
#include <fmt/format.h>
