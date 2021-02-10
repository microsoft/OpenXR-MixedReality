// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrError.h"
#include "XrEnumerate.h"
#include "XrExtensions.h"

namespace xr {
    struct ExtensionContext : xr::ExtensionDispatchTable {
        bool SupportsD3D11;
        bool SupportsD3D12;
        bool SupportsDepthInfo;
        bool SupportsVisibilityMask;
        bool SupportsUnboundedSpace;
        bool SupportsSpatialAnchor;
        bool SupportsHandInteraction;
        bool SupportsEyeGazeInteraction;
        bool SupportsHandJointTracking;
        bool SupportsHandMeshTracking;
        bool SupportsSpatialGraphBridge;
        bool SupportsControllerModel;
        bool SupportsSecondaryViewConfiguration;
        bool SupportsAppContainer;
        bool SupportsHolographicWindowAttachment;
        bool SupportsSamsungOdysseyController;
        bool SupportsHPMixedRealityController;
        bool SupportsSpatialAnchorExport;
        bool SupportsPerceptionAnchorInterop;
        bool SupportsColorScaleBias;
        bool SupportsSceneUnderstanding;
        bool SupportsSceneUnderstandingSerialization;
        bool SupportsReprojectionConfiguration;

        std::vector<const char*> EnabledExtensions;
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

        auto isExtensionEnabled = [&list = extensions.EnabledExtensions](const char* extensionName) -> bool {
            return list.end() !=
                   std::find_if(list.begin(), list.end(), [&extensionName](auto&& i) { return 0 == strcmp(i, extensionName); });
        };

        // Record enabled extensions in extension context as bool for easy usage.
#ifdef XR_USE_GRAPHICS_API_D3D11
        extensions.SupportsD3D11 = isExtensionEnabled(XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_GRAPHICS_API_D3D12
        extensions.SupportsD3D12 = isExtensionEnabled(XR_KHR_D3D12_ENABLE_EXTENSION_NAME);
#endif
#ifdef XR_USE_PLATFORM_WIN32
        extensions.SupportsAppContainer = isExtensionEnabled(XR_EXT_WIN32_APPCONTAINER_COMPATIBLE_EXTENSION_NAME);
        extensions.SupportsHolographicWindowAttachment = isExtensionEnabled(XR_MSFT_HOLOGRAPHIC_WINDOW_ATTACHMENT_EXTENSION_NAME);
        extensions.SupportsPerceptionAnchorInterop = isExtensionEnabled(XR_MSFT_PERCEPTION_ANCHOR_INTEROP_EXTENSION_NAME);
#endif
        extensions.SupportsDepthInfo = isExtensionEnabled(XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
        extensions.SupportsVisibilityMask = isExtensionEnabled(XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
        extensions.SupportsUnboundedSpace = isExtensionEnabled(XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
        extensions.SupportsSpatialAnchor = isExtensionEnabled(XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
        extensions.SupportsHandInteraction = isExtensionEnabled(XR_MSFT_HAND_INTERACTION_EXTENSION_NAME);
        extensions.SupportsEyeGazeInteraction = isExtensionEnabled(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME);
        extensions.SupportsSecondaryViewConfiguration = isExtensionEnabled(XR_MSFT_SECONDARY_VIEW_CONFIGURATION_EXTENSION_NAME);
        extensions.SupportsHandJointTracking = isExtensionEnabled(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
        extensions.SupportsHandMeshTracking = isExtensionEnabled(XR_MSFT_HAND_TRACKING_MESH_EXTENSION_NAME);
        extensions.SupportsSpatialGraphBridge = isExtensionEnabled(XR_MSFT_SPATIAL_GRAPH_BRIDGE_EXTENSION_NAME);
        extensions.SupportsControllerModel = isExtensionEnabled(XR_MSFT_CONTROLLER_MODEL_EXTENSION_NAME);
        extensions.SupportsSamsungOdysseyController = isExtensionEnabled(XR_EXT_SAMSUNG_ODYSSEY_CONTROLLER_EXTENSION_NAME);
        extensions.SupportsHPMixedRealityController = isExtensionEnabled(XR_EXT_HP_MIXED_REALITY_CONTROLLER_EXTENSION_NAME);
        extensions.SupportsColorScaleBias = isExtensionEnabled(XR_KHR_COMPOSITION_LAYER_COLOR_SCALE_BIAS_EXTENSION_NAME);
        extensions.SupportsSceneUnderstanding = isExtensionEnabled(XR_MSFT_SCENE_UNDERSTANDING_PREVIEW2_EXTENSION_NAME);
        extensions.SupportsSceneUnderstandingSerialization =
            isExtensionEnabled(XR_MSFT_SCENE_UNDERSTANDING_SERIALIZATION_PREVIEW_EXTENSION_NAME);
        extensions.SupportsReprojectionConfiguration = isExtensionEnabled(XR_MSFT_COMPOSITION_LAYER_REPROJECTION_PREVIEW_EXTENSION_NAME);
        extensions.SupportsSpatialAnchorExport = isExtensionEnabled(XR_MSFT_SPATIAL_ANCHOR_EXPORT_PREVIEW_EXTENSION_NAME);

        return extensions;
    }
} // namespace xr

