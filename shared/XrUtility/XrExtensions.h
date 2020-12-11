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

#if XR_MSFT_perception_anchor_interop && defined(XR_USE_PLATFORM_WIN32)
#define FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_) \
    _(xrCreateSpatialAnchorFromPerceptionAnchorMSFT)   \
    _(xrTryGetPerceptionAnchorFromSpatialAnchorMSFT)
#else
#define FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_)
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
#define FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_) _(xrCreateSpatialGraphNodeSpaceMSFT)
#else
#define FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_)
#endif

#if XR_MSFT_scene_understanding_preview
#define FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_) \
    _(xrCreateSceneObserverMSFT)                 \
    _(xrDestroySceneObserverMSFT)                \
    _(xrCreateSceneMSFT)                         \
    _(xrDestroySceneMSFT)                        \
    _(xrComputeNewSceneMSFT)                     \
    _(xrGetSceneComputeStateMSFT)                \
    _(xrLocateSceneObjectsMSFT)                  \
    _(xrGetSceneObjectsMSFT)                     \
    _(xrGetSceneObjectPropertiesMSFT)            \
    _(xrGetSceneMeshMSFT)                        \
    _(xrGetScenePlanePropertiesMSFT)
#else
#define FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_)
#endif

#if XR_MSFT_scene_understanding_serialization_preview
#define FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_) \
    _(xrDeserializeSceneMSFT)                                  \
    _(xrGetSceneSerializedDataMSFT)

#else
#define FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_)
#endif

#define FOR_EACH_EXTENSION_FUNCTION(_)              \
    FOR_EACH_WIN32_EXTENSION_FUNCTION(_)            \
    FOR_EACH_D3D11_EXTENSION_FUNCTION(_)            \
    FOR_EACH_VISIBILITY_MASK_FUNCTION(_)            \
    FOR_EACH_HAND_TRACKING_FUNCTION(_)              \
    FOR_EACH_HAND_TRACKING_MESH_FUNCTION(_)         \
    FOR_EACH_SPATIAL_GRAPH_BRIDGE_FUNCTION(_)       \
    FOR_EACH_SPATIAL_ANCHOR_FUNCTION(_)             \
    FOR_EACH_CONTROLLER_MODEL_EXTENSION_FUNCTION(_) \
    FOR_EACH_PERCEPTION_ANCHOR_INTEROP_FUNCTION(_)  \
    FOR_EACH_SCENE_UNDERSTANDING_FUNCTION(_)        \
    FOR_EACH_SCENE_UNDERSTANDING_SERIALIZATION_FUNCTION(_)


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

