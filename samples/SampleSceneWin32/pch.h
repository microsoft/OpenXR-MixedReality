// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <sdkddkver.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

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

#include <d3d11_2.h>
#include <DirectXColors.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <XrUtility/XrError.h>
#include <XrUtility/XrMath.h>
#include <SampleShared/Trace.h>
#include <SampleShared/ScopeGuard.h>
