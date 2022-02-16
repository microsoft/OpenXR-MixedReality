// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#ifdef XR_USE_PLATFORM_WIN32
#define FOR_EACH_WIN32_EXTENSION_FUNCTION(_) _(xrConvertWin32PerformanceCounterToTimeKHR)
#else
#define FOR_EACH_WIN32_EXTENSION_FUNCTION(_)
#endif

#ifdef XR_USE_GRAPHICS_API_D3D11
#define FOR_EACH_D3D11_EXTENSION_FUNCTION(_) _(xrGetD3D11GraphicsRequirementsKHR)
#else
#define FOR_EACH_D3D11_EXTENSION_FUNCTION(_)
#endif

#if XR_KHR_visibility_mask
#define FOR_EACH_VISIBILITY_MASK_FUNCTION(_) _(xrGetVisibilityMaskKHR)
#else
#define FOR_EACH_VISIBILITY_MASK_FUNCTION(_)
#endif

#ifdef XR_MSFT_controller_model
#define FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_) \
    _(xrGetControllerModelKeyMSFT)                      \
    _(xrLoadControllerModelMSFT)                        \
    _(xrGetControllerModelPropertiesMSFT)               \
    _(xrGetControllerModelStateMSFT)
#else
#define FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_)
#endif

#if XR_MSFT_spatial_anchor
#define FOR_EACH_SPATIAL_ANCHOR_FUNCTION(_) \
    _(xrCreateSpatialAnchorMSFT)            \
    _(xrCreateSpatialAnchorSpaceMSFT)       \
    _(xrDestroySpatialAnchorMSFT)
#else
#define FOR_EACH_SPATIAL_ANCHOR_FUNCTION(_)
#endif

#if XR_EXT_hand_tracking
#define FOR_EACH_HAND_TRACKING_FUNCTION(_) \
    _(xrCreateHandTrackerEXT)              \
    _(xrDestroyHandTrackerEXT)             \
    _(xrLocateHandJointsEXT)
#else
#define FOR_EACH_HAND_TRACKING_FUNCTION(_)
#endif

#if XR_MSFT_hand_tracking_mesh
#define FOR_EACH_HAND_TRACKING_MESH_FUNCTION(_) \
    _(xrCreateHandMeshSpaceMSFT)                \
    _(xrUpdateHandMeshMSFT)
#else
#define FOR_EACH_HAND_TRACKING_MESH_FUNCTION(_)
#endif

#if XR_MSFT_spatial_graph_bridge
#define FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_)   \
    _(xrCreateSpatialGraphNodeSpaceMSFT)            \
    _(xrTryCreateSpatialGraphStaticNodeBindingMSFT) \
    _(xrDestroySpatialGraphNodeBindingMSFT)         \
    _(xrGetSpatialGraphNodeBindingPropertiesMSFT)
#else
#define FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_)
#endif

#if XR_MSFT_perception_anchor_interop && defined(XR_USE_PLATFORM_WIN32)
#define FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_) \
    _(xrCreateSpatialAnchorFromPerceptionAnchorMSFT)   \
    _(xrTryGetPerceptionAnchorFromSpatialAnchorMSFT)
#else
#define FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_)
#endif

#if XR_MSFT_scene_understanding
#define FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_) \
    _(xrEnumerateSceneComputeFeaturesMSFT)       \
    _(xrCreateSceneObserverMSFT)                 \
    _(xrDestroySceneObserverMSFT)                \
    _(xrCreateSceneMSFT)                         \
    _(xrDestroySceneMSFT)                        \
    _(xrComputeNewSceneMSFT)                     \
    _(xrGetSceneComputeStateMSFT)                \
    _(xrGetSceneComponentsMSFT)                  \
    _(xrLocateSceneComponentsMSFT)               \
    _(xrGetSceneMeshBuffersMSFT)
#else
#define FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_)
#endif

#if XR_MSFT_scene_understanding_serialization
#define FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_) \
    _(xrDeserializeSceneMSFT)                                  \
    _(xrGetSerializedSceneFragmentDataMSFT)
#else
#define FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_)
#endif

#if XR_MSFT_spatial_anchor_persistence
#define FOR_EACH_SPATIAL_ANCHOR_PERSISTENCE_FUNCTION(_) \
    _(xrCreateSpatialAnchorStoreConnectionMSFT)         \
    _(xrDestroySpatialAnchorStoreConnectionMSFT)        \
    _(xrPersistSpatialAnchorMSFT)                       \
    _(xrEnumeratePersistedSpatialAnchorNamesMSFT)       \
    _(xrCreateSpatialAnchorFromPersistedNameMSFT)       \
    _(xrUnpersistSpatialAnchorMSFT)                     \
    _(xrClearSpatialAnchorStoreMSFT)
#else
#define FOR_EACH_SPATIAL_ANCHOR_PERSISTENCE_FUNCTION(_)
#endif

#if XR_MSFT_composition_layer_reprojection
#define FOR_EACH_COMPOSITION_LAYER_REPROJECTION_FUNCTION(_) _(xrEnumerateReprojectionModesMSFT)
#else
#define FOR_EACH_COMPOSITION_LAYER_REPROJECTION_FUNCTION(_)
#endif

#define FOR_EACH_EXTENSION_FUNCTION(_)                     \
    FOR_EACH_WIN32_EXTENSION_FUNCTION(_)                   \
    FOR_EACH_D3D11_EXTENSION_FUNCTION(_)                   \
    FOR_EACH_VISIBILITY_MASK_FUNCTION(_)                   \
    FOR_EACH_HAND_TRACKING_FUNCTION(_)                     \
    FOR_EACH_HAND_TRACKING_MESH_FUNCTION(_)                \
    FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_)              \
    FOR_EACH_SPATIAL_ANCHOR_FUNCTION(_)                    \
    FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_)        \
    FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_)         \
    FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_)               \
    FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_) \
    FOR_EACH_SPATIAL_ANCHOR_PERSISTENCE_FUNCTION(_)        \
    FOR_EACH_COMPOSITION_LAYER_REPROJECTION_FUNCTION(_)

#define GET_INSTANCE_PROC_ADDRESS(name) \
    (void)xrGetInstanceProcAddr(instance, #name, reinterpret_cast<PFN_xrVoidFunction*>(const_cast<PFN_##name*>(&name)));
#define DEFINE_PROC_MEMBER(name) PFN_##name name{nullptr};

// Define a local variable of given function name and get proc address from given instance handle.
// The returned function pointer may be nullptr when the function is not supported by the xr instance.
// NOTE: The app should cache the function pointer for the lifetime of the corresponding instance handle,
//       because this xrGetInstanceProcAddr operation may be expensive to do repeatedly in a frame loop.
#define DEFINE_XR_FUNCTION_AND_GET_INSTANCE_PROC_ADDRESS(xrFunctionName, instance, PFN_xrGetInstanceProcAddr) \
    PFN_##xrFunctionName xrFunctionName = nullptr;                                                            \
    (void)(*PFN_xrGetInstanceProcAddr)(instance, #xrFunctionName, (PFN_xrVoidFunction*)&xrFunctionName);

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
