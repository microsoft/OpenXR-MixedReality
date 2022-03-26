// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <array>
#include <map>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <d3d11.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_reflection.h>

#define ENABLE_GLOBAL_XR_DISPATCH_TABLE
#include <XrUtility/XrDispatchTable.h>
#include <XrUtility/XrError.h>
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrMath.h>
#include <XrUtility/XrString.h>

#include <winrt/base.h> // winrt::com_ptr
