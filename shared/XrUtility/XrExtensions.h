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

#ifdef XR_USE_PLATFORM_WIN32
#define FOR_EACH_WIN32_EXTENSION_FUNCTION(_) \
    _(xrConvertWin32PerformanceCounterToTimeKHR)
#else
#define FOR_EACH_WIN32_EXTENSION_FUNCTION(_)
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
#define FOR_EACH_D3D11_EXTENSION_FUNCTION(_) \
    _(xrGetD3D11GraphicsRequirementsKHR)
#else
#define FOR_EACH_D3D11_EXTENSION_FUNCTION(_)
#endif

#ifdef XR_MSFT_CONTROLLER_MODEL_PREVIEW_EXTENSION_NAME
#define FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_) \
    _(xrGetControllerModelKeyMSFT)               \
    _(xrLoadControllerModelMSFT)                 \
    _(xrGetControllerModelPropertiesMSFT)        \
    _(xrGetControllerModelStateMSFT)
#else
#define FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_)
#endif

#define FOR_EACH_EXTENSION_FUNCTION(_)              \
    _(xrCreateSpatialAnchorMSFT)                    \
    _(xrCreateSpatialAnchorSpaceMSFT)               \
    _(xrDestroySpatialAnchorMSFT)                   \
    _(xrCreateHandTrackerEXT)                       \
    _(xrDestroyHandTrackerEXT)                      \
    _(xrLocateHandJointsEXT)                        \
    _(xrCreateHandMeshSpaceMSFT)                    \
    _(xrUpdateHandMeshMSFT)                         \
    _(xrCreateSpatialGraphNodeSpaceMSFT)            \
    _(xrGetVisibilityMaskKHR)                       \
    FOR_EACH_WIN32_EXTENSION_FUNCTION(_)            \
    FOR_EACH_D3D11_EXTENSION_FUNCTION(_)            \
    FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_)

#define GET_INSTANCE_PROC_ADDRESS(name) \
    (void)xrGetInstanceProcAddr(instance, #name, reinterpret_cast<PFN_xrVoidFunction*>(const_cast<PFN_##name*>(&name)));
#define DEFINE_PROC_MEMBER(name) const PFN_##name name{nullptr};

namespace xr {
    struct ExtensionDispatchTable {
        FOR_EACH_EXTENSION_FUNCTION(DEFINE_PROC_MEMBER);

        ExtensionDispatchTable() = default;
        void PopulateDispatchTable(XrInstance instance) {
            FOR_EACH_EXTENSION_FUNCTION(GET_INSTANCE_PROC_ADDRESS);
        }
    };
} // namespace xr

#undef DEFINE_PROC_MEMBER
#undef GET_INSTANCE_PROC_ADDRESS
#undef FOR_EACH_EXTENSION_FUNCTION

