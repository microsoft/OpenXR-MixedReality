// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <openxr/openxr_msft_preview.h>
#include "XrEnumerate.h"

namespace xr {
    struct ExtensionContext {
        bool SupportsD3D11{false};
        bool SupportsD3D12{false};
        bool SupportsDepthInfo{false};
        bool SupportsVisibilityMask{false};
        bool SupportsUnboundedSpace{false};
        bool SupportsSpatialAnchor{false};
        bool SupportsHandInteractionEXT{false};
        bool SupportsHandInteractionMSFT{false};
        bool SupportsEyeGazeInteraction{false};
        bool SupportsHandJointTracking{false};
        bool SupportsHandMeshTracking{false};
        bool SupportsSpatialGraphBridge{false};
        bool SupportsControllerModel{false};
        bool SupportsSecondaryViewConfiguration{false};
        bool SupportsAppContainer{false};
        bool SupportsHolographicWindowAttachment{false};
        bool SupportsSamsungOdysseyController{false};
        bool SupportsHPMixedRealityController{false};
        bool SupportsSpatialAnchorPersistence{false};
        bool SupportsPerceptionAnchorInterop{false};
        bool SupportsColorScaleBias{false};
        bool SupportsSceneUnderstanding{false};
        bool SupportsSceneUnderstandingSerialization{false};
        bool SupportsReprojectionConfiguration{false};
        bool SupportsPalmPose{false};
        bool SupportsSceneMarker{false};

        std::vector<const char*> EnabledExtensions;

        inline bool IsEnabled(const char* extensionName) const {
            const auto it = std::find_if(
                EnabledExtensions.begin(), EnabledExtensions.end(), [&extensionName](auto&& i) { return 0 == strcmp(i, extensionName); });
            return it != EnabledExtensions.end();
        }
    };

    inline ExtensionContext CreateExtensionContext(const std::vector<const char*>& requestedExtensions) {
        const std::vector<XrExtensionProperties> runtimeSupportedExtensions = xr::EnumerateInstanceExtensionProperties();

        // Filter requested extensions to make sure enabled extensions are supported by current runtime
        xr::ExtensionContext extensions{};
        for (auto& requestedExtension : requestedExtensions) {
            for (const auto& supportedExtension : runtimeSupportedExtensions) {
                if (strcmp(supportedExtension.extensionName, requestedExtension) == 0) {
                    extensions.EnabledExtensions.push_back(requestedExtension);
                    break;
                }
            }
        }

        // Record enabled extensions in extension context as bool for easy usage.
#ifdef XR_USE_GRAPHICS_API_D3D11
        extensions.SupportsD3D11 = extensions.IsEnabled(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
        extensions.SupportsD3D12 = extensions.IsEnabled(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_PLATFORM_WIN32
        extensions.SupportsAppContainer = extensions.IsEnabled(XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME);
        extensions.SupportsHolographicWindowAttachment = extensions.IsEnabled(XR_MSFT_HOLOGRAPHIC_WINDOW_ATTACHMENT_EXTENSION_NAME);
        extensions.SupportsPerceptionAnchorInterop = extensions.IsEnabled(XR_MSFT_PERCEPTION_ANCHOR_INTEROP_EXTENSION_NAME);
#endif
        extensions.SupportsDepthInfo = extensions.IsEnabled(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
        extensions.SupportsVisibilityMask = extensions.IsEnabled(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
        extensions.SupportsUnboundedSpace = extensions.IsEnabled(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
        extensions.SupportsSpatialAnchor = extensions.IsEnabled(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
        extensions.SupportsHandInteractionEXT = extensions.IsEnabled(XR_EXT_HAND_INTERACTION_EXTENSION_NAME);
        extensions.SupportsHandInteractionMSFT = extensions.IsEnabled(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        extensions.SupportsEyeGazeInteraction = extensions.IsEnabled(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME);
        extensions.SupportsSecondaryViewConfiguration = extensions.IsEnabled(XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME);
        extensions.SupportsHandJointTracking = extensions.IsEnabled(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
        extensions.SupportsHandMeshTracking = extensions.IsEnabled(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);
        extensions.SupportsSpatialGraphBridge = extensions.IsEnabled(XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME);
        extensions.SupportsControllerModel = extensions.IsEnabled(XR_MSFT_CONTROLLER_MODEL_EXTENSION_NAME);
        extensions.SupportsSamsungOdysseyController = extensions.IsEnabled(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME);
        extensions.SupportsHPMixedRealityController = extensions.IsEnabled(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);
        extensions.SupportsColorScaleBias = extensions.IsEnabled(XR_KHR_COMPOSITION_LAYER_COLOR_SCALE_BIAS_EXTENSION_NAME);
        extensions.SupportsSceneUnderstanding = extensions.IsEnabled(XR_MSFT_SCENE_UNDERSTANDING_EXTENSION_NAME);
        extensions.SupportsSceneUnderstandingSerialization = extensions.IsEnabled(XR_MSFT_SCENE_UNDERSTANDING_SERIALIZATION_EXTENSION_NAME);
        extensions.SupportsReprojectionConfiguration = extensions.IsEnabled(XR_MSFT_COMPOSITION_LAYER_REPROJECTION_EXTENSION_NAME);
        extensions.SupportsSpatialAnchorPersistence = extensions.IsEnabled(XR_MSFT_SPATIAL_ANCHOR_PERSISTENCE_EXTENSION_NAME);
        extensions.SupportsPalmPose = extensions.IsEnabled(XR_EXT_PALM_POSE_EXTENSION_NAME);
        extensions.SupportsSceneMarker = extensions.IsEnabled(XR_MSFTX_SCENE_MARKER_EXTENSION_NAME);

        return extensions;
    }
} // namespace xr

