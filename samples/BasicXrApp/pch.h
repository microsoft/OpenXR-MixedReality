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

// Look up an XR function pointer on the spot. Typesafe!
#define GET_XR_PROC(__instance, __name)                                                                     \
    [&]() -> PFN_##__name {                                                                                 \
        PFN_##__name __xrFunc{nullptr};                                                                     \
        (void)xrGetInstanceProcAddr(__instance, #__name, reinterpret_cast<PFN_xrVoidFunction*>(&__xrFunc)); \
        return __xrFunc;                                                                                    \
    }                                                                                                       \
    ();

#include "../XrUtility/XrError.h"
#include "../XrUtility/XrHandle.h"
#include "../XrUtility/XrMath.h"
#include "../XrUtility/XrString.h"

#include <winrt/base.h> // winrt::com_ptr
