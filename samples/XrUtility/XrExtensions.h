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

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#define EXTENSION_FUNCS_LIST(_)                  \
    _(xrCreateSpatialAnchorMSFT)                 \
    _(xrCreateSpatialAnchorSpaceMSFT)            \
    _(xrDestroySpatialAnchorMSFT)                \
    _(xrConvertWin32PerformanceCounterToTimeKHR) \
    _(xrGetD3D11GraphicsRequirementsKHR)         \
    _(xrGetVisibilityMaskKHR)

#define GET_XR_PROC(name) \
    (void)xrGetInstanceProcAddr(instance, #name, reinterpret_cast<PFN_xrVoidFunction*>(const_cast<PFN_##name*>(&name)));
#define DEFINE_PROC_MEMBER(name) const PFN_##name name{nullptr};

namespace xr {
    struct ExtensionDispatchTable {
        EXTENSION_FUNCS_LIST(DEFINE_PROC_MEMBER);

        explicit ExtensionDispatchTable(XrInstance instance) {
            EXTENSION_FUNCS_LIST(GET_XR_PROC);
        }
    };
} // namespace xr

#undef GET_XR_PROC
#undef DEFINE_PROC_MEMBER
#undef EXTENSION_FUNCS_LIST
