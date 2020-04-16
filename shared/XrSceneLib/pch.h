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
#pragma once

#include <sdkddkver.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include <exception>
#include <memory>
#include <tuple>
#include <vector>
#include <map>
#include <array>
#include <chrono>
#include <string>
#include <atomic>
#include <future>
#include <pathcch.h>
#include <initializer_list>
#include <optional>
#include <set>

#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <XrUtility/XrError.h>
#include <XrUtility/XrMath.h>
#include <XrUtility/XrHandle.h>

#include <winrt/base.h> // for winrt::com_ptr

#define FMT_HEADER_ONLY
#include <fmt/format.h>
