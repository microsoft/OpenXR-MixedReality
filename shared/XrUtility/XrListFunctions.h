// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrExtensionDefined.h"

// These reflection macros should be in openxr_reflection.h and automatically generated from xr.xml

#define XR_LIST_FUNCTIONS_OPENXR_FUNCTIONS(_) \
    _(xrEnumerateApiLayerProperties)          \
    _(xrEnumerateInstanceExtensionProperties) \
    _(xrCreateInstance)                       \
    _(xrDestroyInstance)                      \
    _(xrGetInstanceProperties)                \
    _(xrPollEvent)                            \
    _(xrResultToString)                       \
    _(xrStructureTypeToString)                \
    _(xrGetSystem)                            \
    _(xrGetSystemProperties)                  \
    _(xrEnumerateEnvironmentBlendModes)       \
    _(xrCreateSession)                        \
    _(xrDestroySession)                       \
    _(xrEnumerateReferenceSpaces)             \
    _(xrCreateReferenceSpace)                 \
    _(xrGetReferenceSpaceBoundsRect)          \
    _(xrCreateActionSpace)                    \
    _(xrLocateSpace)                          \
    _(xrDestroySpace)                         \
    _(xrEnumerateViewConfigurations)          \
    _(xrGetViewConfigurationProperties)       \
    _(xrEnumerateViewConfigurationViews)      \
    _(xrEnumerateSwapchainFormats)            \
    _(xrCreateSwapchain)                      \
    _(xrDestroySwapchain)                     \
    _(xrEnumerateSwapchainImages)             \
    _(xrAcquireSwapchainImage)                \
    _(xrWaitSwapchainImage)                   \
    _(xrReleaseSwapchainImage)                \
    _(xrBeginSession)                         \
    _(xrEndSession)                           \
    _(xrRequestExitSession)                   \
    _(xrWaitFrame)                            \
    _(xrBeginFrame)                           \
    _(xrEndFrame)                             \
    _(xrLocateViews)                          \
    _(xrStringToPath)                         \
    _(xrPathToString)                         \
    _(xrCreateActionSet)                      \
    _(xrDestroyActionSet)                     \
    _(xrCreateAction)                         \
    _(xrDestroyAction)                        \
    _(xrSuggestInteractionProfileBindings)    \
    _(xrAttachSessionActionSets)              \
    _(xrGetCurrentInteractionProfile)         \
    _(xrGetActionStateBoolean)                \
    _(xrGetActionStateFloat)                  \
    _(xrGetActionStateVector2f)               \
    _(xrGetActionStatePose)                   \
    _(xrSyncActions)                          \
    _(xrEnumerateBoundSourcesForAction)       \
    _(xrGetInputSourceLocalizedName)          \
    _(xrApplyHapticFeedback)                  \
    _(xrStopHapticFeedback)

#define XR_LIST_FUNCTIONS_XR_KHR_win32_convert_performance_counter_time(_) \
    _(xrConvertWin32PerformanceCounterToTimeKHR)                           \
    _(xrConvertTimeToWin32PerformanceCounterKHR)

#define XR_LIST_FUNCTIONS_XR_KHR_convert_timespec_time(_) \
    _(xrConvertTimespecTimeToTimeKHR)                     \
    _(xrConvertTimeToTimespecTimeKHR)

#define XR_LIST_FUNCTIONS_XR_KHR_D3D11_enable(_) _(xrGetD3D11GraphicsRequirementsKHR)
#define XR_LIST_FUNCTIONS_XR_KHR_D3D12_enable(_) _(xrGetD3D12GraphicsRequirementsKHR)
#define XR_LIST_FUNCTIONS_XR_KHR_visibility_mask(_) _(xrGetVisibilityMaskKHR)

#define XR_LIST_FUNCTIONS_XR_MSFT_controller_model(_) \
    _(xrGetControllerModelKeyMSFT)                    \
    _(xrLoadControllerModelMSFT)                      \
    _(xrGetControllerModelPropertiesMSFT)             \
    _(xrGetControllerModelStateMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_spatial_anchor(_) \
    _(xrCreateSpatialAnchorMSFT)                    \
    _(xrCreateSpatialAnchorSpaceMSFT)               \
    _(xrDestroySpatialAnchorMSFT)

#define XR_LIST_FUNCTIONS_XR_EXT_hand_tracking(_) \
    _(xrCreateHandTrackerEXT)                     \
    _(xrDestroyHandTrackerEXT)                    \
    _(xrLocateHandJointsEXT)

#define XR_LIST_FUNCTIONS_XR_MSFT_hand_tracking_mesh(_) \
    _(xrCreateHandMeshSpaceMSFT)                        \
    _(xrUpdateHandMeshMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_spatial_graph_bridge(_) \
    _(xrCreateSpatialGraphNodeSpaceMSFT)                  \
    _(xrTryCreateSpatialGraphStaticNodeBindingMSFT)       \
    _(xrDestroySpatialGraphNodeBindingMSFT)               \
    _(xrGetSpatialGraphNodeBindingPropertiesMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_holographic_remoting(_) \
    _(xrRemotingSetContextPropertiesMSFT)                 \
    _(xrRemotingConnectMSFT)                              \
    _(xrRemotingListenMSFT)                               \
    _(xrRemotingDisconnectMSFT)                           \
    _(xrRemotingGetConnectionStateMSFT)                   \
    _(xrRemotingSetSecureConnectionClientCallbacksMSFT)   \
    _(xrRemotingSetSecureConnectionServerCallbacksMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_perception_anchor_interop(_) \
    _(xrCreateSpatialAnchorFromPerceptionAnchorMSFT)           \
    _(xrTryGetPerceptionAnchorFromSpatialAnchorMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_scene_understanding(_) \
    _(xrEnumerateSceneComputeFeaturesMSFT)               \
    _(xrCreateSceneObserverMSFT)                         \
    _(xrDestroySceneObserverMSFT)                        \
    _(xrCreateSceneMSFT)                                 \
    _(xrDestroySceneMSFT)                                \
    _(xrComputeNewSceneMSFT)                             \
    _(xrGetSceneComputeStateMSFT)                        \
    _(xrGetSceneComponentsMSFT)                          \
    _(xrLocateSceneComponentsMSFT)                       \
    _(xrGetSceneMeshBuffersMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_scene_understanding_serialization(_) \
    _(xrDeserializeSceneMSFT)                                          \
    _(xrGetSerializedSceneFragmentDataMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_spatial_anchor_persistence(_) \
    _(xrCreateSpatialAnchorStoreConnectionMSFT)                 \
    _(xrDestroySpatialAnchorStoreConnectionMSFT)                \
    _(xrPersistSpatialAnchorMSFT)                               \
    _(xrEnumeratePersistedSpatialAnchorNamesMSFT)               \
    _(xrCreateSpatialAnchorFromPersistedNameMSFT)               \
    _(xrUnpersistSpatialAnchorMSFT)                             \
    _(xrClearSpatialAnchorStoreMSFT)

#define XR_LIST_FUNCTIONS_XR_MSFT_composition_layer_reprojection(_) _(xrEnumerateReprojectionModesMSFT)

// clang-format off
#define XR_LIST_FUNCTIONS_OPENXR_EXTENSIONS(_, __)                                                                                  \
    XR_KHR_win32_convert_performance_counter_time_DEFINED(XR_LIST_FUNCTIONS_XR_KHR_win32_convert_performance_counter_time, _, __)   \
    XR_KHR_convert_timespec_time_DEFINED(XR_LIST_FUNCTIONS_XR_KHR_convert_timespec_time, _, __)                                     \
    XR_KHR_D3D11_enable_DEFINED(XR_LIST_FUNCTIONS_XR_KHR_D3D11_enable, _, __)                                                       \
    XR_KHR_D3D12_enable_DEFINED(XR_LIST_FUNCTIONS_XR_KHR_D3D12_enable, _, __)                                                       \
    XR_KHR_visibility_mask_DEFINED(XR_LIST_FUNCTIONS_XR_KHR_visibility_mask, _, __)                                                 \
    XR_MSFT_controller_model_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_controller_model, _, __)                                             \
    XR_MSFT_spatial_anchor_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_spatial_anchor, _, __)                                                 \
    XR_EXT_hand_tracking_DEFINED(XR_LIST_FUNCTIONS_XR_EXT_hand_tracking, _, __)                                                     \
    XR_MSFT_hand_tracking_mesh_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_hand_tracking_mesh, _, __)                                         \
    XR_MSFT_spatial_graph_bridge_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_spatial_graph_bridge, _, __)                                     \
    XR_MSFT_holographic_remoting_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_holographic_remoting, _, __)                                     \
    XR_MSFT_perception_anchor_interop_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_perception_anchor_interop, _, __)                           \
    XR_MSFT_scene_understanding_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_scene_understanding, _, __)                                       \
    XR_MSFT_scene_understanding_serialization_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_scene_understanding_serialization, _, __)           \
    XR_MSFT_spatial_anchor_persistence_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_spatial_anchor_persistence, _, __)                         \
    XR_MSFT_composition_layer_reprojection_DEFINED(XR_LIST_FUNCTIONS_XR_MSFT_composition_layer_reprojection, _, __)                 \
// clang-format on
