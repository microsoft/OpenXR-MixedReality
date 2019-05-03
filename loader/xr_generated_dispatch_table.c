// *********** THIS FILE IS GENERATED - DO NOT EDIT ***********
//     See utility_source_generator.py for modifications
// ************************************************************

// Copyright (c) 2017-2019 The Khronos Group Inc.
// Copyright (c) 2017-2019 Valve Corporation
// Copyright (c) 2017-2019 LunarG, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Author: Mark Young <marky@lunarg.com>
//

#include "xr_dependencies.h"
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include "xr_generated_dispatch_table.h"

#ifdef __cplusplus
extern "C" { 
#endif
// Helper function to populate an instance dispatch table
void GeneratedXrPopulateDispatchTable(struct XrGeneratedDispatchTable *table,
                                      XrInstance instance,
                                      PFN_xrGetInstanceProcAddr get_inst_proc_addr) {

    // ---- Core 0.90 commands
    table->GetInstanceProcAddr = get_inst_proc_addr;
    (get_inst_proc_addr(instance, "xrCreateInstance", (PFN_xrVoidFunction*)&table->CreateInstance));
    (get_inst_proc_addr(instance, "xrDestroyInstance", (PFN_xrVoidFunction*)&table->DestroyInstance));
    (get_inst_proc_addr(instance, "xrGetInstanceProperties", (PFN_xrVoidFunction*)&table->GetInstanceProperties));
    (get_inst_proc_addr(instance, "xrPollEvent", (PFN_xrVoidFunction*)&table->PollEvent));
    (get_inst_proc_addr(instance, "xrResultToString", (PFN_xrVoidFunction*)&table->ResultToString));
    (get_inst_proc_addr(instance, "xrStructureTypeToString", (PFN_xrVoidFunction*)&table->StructureTypeToString));
    (get_inst_proc_addr(instance, "xrGetSystem", (PFN_xrVoidFunction*)&table->GetSystem));
    (get_inst_proc_addr(instance, "xrGetSystemProperties", (PFN_xrVoidFunction*)&table->GetSystemProperties));
    (get_inst_proc_addr(instance, "xrEnumerateEnvironmentBlendModes", (PFN_xrVoidFunction*)&table->EnumerateEnvironmentBlendModes));
    (get_inst_proc_addr(instance, "xrCreateSession", (PFN_xrVoidFunction*)&table->CreateSession));
    (get_inst_proc_addr(instance, "xrDestroySession", (PFN_xrVoidFunction*)&table->DestroySession));
    (get_inst_proc_addr(instance, "xrEnumerateReferenceSpaces", (PFN_xrVoidFunction*)&table->EnumerateReferenceSpaces));
    (get_inst_proc_addr(instance, "xrCreateReferenceSpace", (PFN_xrVoidFunction*)&table->CreateReferenceSpace));
    (get_inst_proc_addr(instance, "xrGetReferenceSpaceBoundsRect", (PFN_xrVoidFunction*)&table->GetReferenceSpaceBoundsRect));
    (get_inst_proc_addr(instance, "xrCreateActionSpace", (PFN_xrVoidFunction*)&table->CreateActionSpace));
    (get_inst_proc_addr(instance, "xrLocateSpace", (PFN_xrVoidFunction*)&table->LocateSpace));
    (get_inst_proc_addr(instance, "xrDestroySpace", (PFN_xrVoidFunction*)&table->DestroySpace));
    (get_inst_proc_addr(instance, "xrEnumerateViewConfigurations", (PFN_xrVoidFunction*)&table->EnumerateViewConfigurations));
    (get_inst_proc_addr(instance, "xrGetViewConfigurationProperties", (PFN_xrVoidFunction*)&table->GetViewConfigurationProperties));
    (get_inst_proc_addr(instance, "xrEnumerateViewConfigurationViews", (PFN_xrVoidFunction*)&table->EnumerateViewConfigurationViews));
    (get_inst_proc_addr(instance, "xrEnumerateSwapchainFormats", (PFN_xrVoidFunction*)&table->EnumerateSwapchainFormats));
    (get_inst_proc_addr(instance, "xrCreateSwapchain", (PFN_xrVoidFunction*)&table->CreateSwapchain));
    (get_inst_proc_addr(instance, "xrDestroySwapchain", (PFN_xrVoidFunction*)&table->DestroySwapchain));
    (get_inst_proc_addr(instance, "xrEnumerateSwapchainImages", (PFN_xrVoidFunction*)&table->EnumerateSwapchainImages));
    (get_inst_proc_addr(instance, "xrAcquireSwapchainImage", (PFN_xrVoidFunction*)&table->AcquireSwapchainImage));
    (get_inst_proc_addr(instance, "xrWaitSwapchainImage", (PFN_xrVoidFunction*)&table->WaitSwapchainImage));
    (get_inst_proc_addr(instance, "xrReleaseSwapchainImage", (PFN_xrVoidFunction*)&table->ReleaseSwapchainImage));
    (get_inst_proc_addr(instance, "xrBeginSession", (PFN_xrVoidFunction*)&table->BeginSession));
    (get_inst_proc_addr(instance, "xrEndSession", (PFN_xrVoidFunction*)&table->EndSession));
    (get_inst_proc_addr(instance, "xrWaitFrame", (PFN_xrVoidFunction*)&table->WaitFrame));
    (get_inst_proc_addr(instance, "xrBeginFrame", (PFN_xrVoidFunction*)&table->BeginFrame));
    (get_inst_proc_addr(instance, "xrEndFrame", (PFN_xrVoidFunction*)&table->EndFrame));
    (get_inst_proc_addr(instance, "xrLocateViews", (PFN_xrVoidFunction*)&table->LocateViews));
    (get_inst_proc_addr(instance, "xrStringToPath", (PFN_xrVoidFunction*)&table->StringToPath));
    (get_inst_proc_addr(instance, "xrPathToString", (PFN_xrVoidFunction*)&table->PathToString));
    (get_inst_proc_addr(instance, "xrCreateActionSet", (PFN_xrVoidFunction*)&table->CreateActionSet));
    (get_inst_proc_addr(instance, "xrDestroyActionSet", (PFN_xrVoidFunction*)&table->DestroyActionSet));
    (get_inst_proc_addr(instance, "xrCreateAction", (PFN_xrVoidFunction*)&table->CreateAction));
    (get_inst_proc_addr(instance, "xrDestroyAction", (PFN_xrVoidFunction*)&table->DestroyAction));
    (get_inst_proc_addr(instance, "xrSetInteractionProfileSuggestedBindings", (PFN_xrVoidFunction*)&table->SetInteractionProfileSuggestedBindings));
    (get_inst_proc_addr(instance, "xrGetCurrentInteractionProfile", (PFN_xrVoidFunction*)&table->GetCurrentInteractionProfile));
    (get_inst_proc_addr(instance, "xrGetActionStateBoolean", (PFN_xrVoidFunction*)&table->GetActionStateBoolean));
    (get_inst_proc_addr(instance, "xrGetActionStateVector1f", (PFN_xrVoidFunction*)&table->GetActionStateVector1f));
    (get_inst_proc_addr(instance, "xrGetActionStateVector2f", (PFN_xrVoidFunction*)&table->GetActionStateVector2f));
    (get_inst_proc_addr(instance, "xrGetActionStatePose", (PFN_xrVoidFunction*)&table->GetActionStatePose));
    (get_inst_proc_addr(instance, "xrSyncActionData", (PFN_xrVoidFunction*)&table->SyncActionData));
    (get_inst_proc_addr(instance, "xrGetBoundSourcesForAction", (PFN_xrVoidFunction*)&table->GetBoundSourcesForAction));
    (get_inst_proc_addr(instance, "xrGetInputSourceLocalizedName", (PFN_xrVoidFunction*)&table->GetInputSourceLocalizedName));
    (get_inst_proc_addr(instance, "xrApplyHapticFeedback", (PFN_xrVoidFunction*)&table->ApplyHapticFeedback));
    (get_inst_proc_addr(instance, "xrStopHapticFeedback", (PFN_xrVoidFunction*)&table->StopHapticFeedback));

    // ---- XR_KHR_android_thread_settings extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    (get_inst_proc_addr(instance, "xrSetAndroidApplicationThreadKHR", (PFN_xrVoidFunction*)&table->SetAndroidApplicationThreadKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_android_surface_swapchain extension commands
#if defined(XR_USE_PLATFORM_ANDROID)
    (get_inst_proc_addr(instance, "xrCreateSwapchainAndroidSurfaceKHR", (PFN_xrVoidFunction*)&table->CreateSwapchainAndroidSurfaceKHR));
#endif // defined(XR_USE_PLATFORM_ANDROID)

    // ---- XR_KHR_opengl_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL)
    (get_inst_proc_addr(instance, "xrGetOpenGLGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetOpenGLGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL)

    // ---- XR_KHR_opengl_es_enable extension commands
#if defined(XR_USE_GRAPHICS_API_OPENGL_ES)
    (get_inst_proc_addr(instance, "xrGetOpenGLESGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetOpenGLESGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_OPENGL_ES)

    // ---- XR_KHR_vulkan_enable extension commands
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    (get_inst_proc_addr(instance, "xrGetVulkanInstanceExtensionsKHR", (PFN_xrVoidFunction*)&table->GetVulkanInstanceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    (get_inst_proc_addr(instance, "xrGetVulkanDeviceExtensionsKHR", (PFN_xrVoidFunction*)&table->GetVulkanDeviceExtensionsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    (get_inst_proc_addr(instance, "xrGetVulkanGraphicsDeviceKHR", (PFN_xrVoidFunction*)&table->GetVulkanGraphicsDeviceKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)
#if defined(XR_USE_GRAPHICS_API_VULKAN)
    (get_inst_proc_addr(instance, "xrGetVulkanGraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetVulkanGraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_VULKAN)

    // ---- XR_KHR_D3D10_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D10)
    (get_inst_proc_addr(instance, "xrGetD3D10GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetD3D10GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D10)

    // ---- XR_KHR_D3D11_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D11)
    (get_inst_proc_addr(instance, "xrGetD3D11GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetD3D11GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D11)

    // ---- XR_KHR_D3D12_enable extension commands
#if defined(XR_USE_GRAPHICS_API_D3D12)
    (get_inst_proc_addr(instance, "xrGetD3D12GraphicsRequirementsKHR", (PFN_xrVoidFunction*)&table->GetD3D12GraphicsRequirementsKHR));
#endif // defined(XR_USE_GRAPHICS_API_D3D12)

    // ---- XR_KHR_visibility_mask extension commands
    (get_inst_proc_addr(instance, "xrGetVisibilityMaskKHR", (PFN_xrVoidFunction*)&table->GetVisibilityMaskKHR));

    // ---- XR_KHR_win32_convert_performance_counter_time extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    (get_inst_proc_addr(instance, "xrConvertWin32PerformanceCounterToTimeKHR", (PFN_xrVoidFunction*)&table->ConvertWin32PerformanceCounterToTimeKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)
#if defined(XR_USE_PLATFORM_WIN32)
    (get_inst_proc_addr(instance, "xrConvertTimeToWin32PerformanceCounterKHR", (PFN_xrVoidFunction*)&table->ConvertTimeToWin32PerformanceCounterKHR));
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_KHR_convert_timespec_time extension commands
#if defined(XR_USE_TIMESPEC)
    (get_inst_proc_addr(instance, "xrConvertTimespecTimeToTimeKHR", (PFN_xrVoidFunction*)&table->ConvertTimespecTimeToTimeKHR));
#endif // defined(XR_USE_TIMESPEC)
#if defined(XR_USE_TIMESPEC)
    (get_inst_proc_addr(instance, "xrConvertTimeToTimespecTimeKHR", (PFN_xrVoidFunction*)&table->ConvertTimeToTimespecTimeKHR));
#endif // defined(XR_USE_TIMESPEC)

    // ---- XR_EXT_debug_utils extension commands
    (get_inst_proc_addr(instance, "xrSetDebugUtilsObjectNameEXT", (PFN_xrVoidFunction*)&table->SetDebugUtilsObjectNameEXT));
    (get_inst_proc_addr(instance, "xrCreateDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)&table->CreateDebugUtilsMessengerEXT));
    (get_inst_proc_addr(instance, "xrDestroyDebugUtilsMessengerEXT", (PFN_xrVoidFunction*)&table->DestroyDebugUtilsMessengerEXT));
    (get_inst_proc_addr(instance, "xrSubmitDebugUtilsMessageEXT", (PFN_xrVoidFunction*)&table->SubmitDebugUtilsMessageEXT));
    (get_inst_proc_addr(instance, "xrSessionBeginDebugUtilsLabelRegionEXT", (PFN_xrVoidFunction*)&table->SessionBeginDebugUtilsLabelRegionEXT));
    (get_inst_proc_addr(instance, "xrSessionEndDebugUtilsLabelRegionEXT", (PFN_xrVoidFunction*)&table->SessionEndDebugUtilsLabelRegionEXT));
    (get_inst_proc_addr(instance, "xrSessionInsertDebugUtilsLabelEXT", (PFN_xrVoidFunction*)&table->SessionInsertDebugUtilsLabelEXT));

    // ---- XR_MSFT_secondary_view_configuration extension commands
    (get_inst_proc_addr(instance, "xrSubmitFrameCompositionLayersMSFT", (PFN_xrVoidFunction*)&table->SubmitFrameCompositionLayersMSFT));

    // ---- XR_MSFT_spatial_perception_bridge extension commands
#if defined(XR_USE_PLATFORM_WIN32)
    (get_inst_proc_addr(instance, "xrCreateSpaceFromSpatialCoordinateSystemMSFT", (PFN_xrVoidFunction*)&table->CreateSpaceFromSpatialCoordinateSystemMSFT));
#endif // defined(XR_USE_PLATFORM_WIN32)

    // ---- XR_MSFT_controller_render_model extension commands
    (get_inst_proc_addr(instance, "xrLoadControllerRenderModelMSFT", (PFN_xrVoidFunction*)&table->LoadControllerRenderModelMSFT));

    // ---- XR_MSFT_spatial_anchor extension commands
    (get_inst_proc_addr(instance, "xrCreateSpatialAnchorSpaceMSFT", (PFN_xrVoidFunction*)&table->CreateSpatialAnchorSpaceMSFT));
    (get_inst_proc_addr(instance, "xrCreateSpatialAnchorMSFT", (PFN_xrVoidFunction*)&table->CreateSpatialAnchorMSFT));
    (get_inst_proc_addr(instance, "xrDestroySpatialAnchorMSFT", (PFN_xrVoidFunction*)&table->DestroySpatialAnchorMSFT));

    // ---- XR_MSFT_spatial_anchor_storage extension commands
    (get_inst_proc_addr(instance, "xrStoreSpatialAnchorMSFT", (PFN_xrVoidFunction*)&table->StoreSpatialAnchorMSFT));
    (get_inst_proc_addr(instance, "xrEnumerateStoredAnchorsMSFT", (PFN_xrVoidFunction*)&table->EnumerateStoredAnchorsMSFT));
    (get_inst_proc_addr(instance, "xrCreateSpatialAnchorFromStoredAnchorNameMSFT", (PFN_xrVoidFunction*)&table->CreateSpatialAnchorFromStoredAnchorNameMSFT));
    (get_inst_proc_addr(instance, "xrDeleteStoredAnchorMSFT", (PFN_xrVoidFunction*)&table->DeleteStoredAnchorMSFT));
}

#ifdef __cplusplus
} // extern "C"
#endif

