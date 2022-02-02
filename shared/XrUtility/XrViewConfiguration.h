// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace xr {
    // The properties of a view configuration that's immutable for a system
    struct ViewProperties {
        XrViewConfigurationType Type;
        XrBool32 FovMutable;
        XrEnvironmentBlendMode BlendMode;
        std::vector<XrEnvironmentBlendMode> SupportedBlendModes;
    };

    // The state of a view configuration that's changing during a session but immutable for a frame
    struct ViewConfigurationState {
        XrViewConfigurationType Type;
        std::vector<XrViewConfigurationView> ViewConfigViews;
        std::vector<XrView> Views;
        bool Active;
    };

    constexpr bool IsPrimaryViewConfigurationType(XrViewConfigurationType type) {
        return type == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO || type == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO ||
               type == XR_VIEW_CONFIGURATION_TYPE_PRIMARY_QUAD_VARJO;
    }

    inline ViewProperties CreateViewProperties(XrInstance instance,
                                               XrSystemId systemId,
                                               XrViewConfigurationType viewConfigurationType,
                                               const std::vector<XrEnvironmentBlendMode>& appSupportedEnvironmentBlendModes) {
        XrViewConfigurationProperties viewConfigProperties{XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
        CHECK_XRCMD(xrGetViewConfigurationProperties(instance, systemId, viewConfigurationType, &viewConfigProperties));

        ViewProperties properties{};
        properties.Type = viewConfigurationType;
        properties.FovMutable = viewConfigProperties.fovMutable;
        properties.SupportedBlendModes = xr::EnumerateEnvironmentBlendModes(instance, systemId, viewConfigurationType);
        properties.BlendMode = xr::PickEnvironmentBlendMode(properties.SupportedBlendModes, appSupportedEnvironmentBlendModes);

        return properties;
    }

    inline ViewConfigurationState CreateViewConfigurationState(XrViewConfigurationType viewConfigurationType,
                                                               XrInstance instance,
                                                               XrSystemId systemId) {
        ViewConfigurationState state{};
        state.Type = viewConfigurationType;
        state.ViewConfigViews = xr::EnumerateViewConfigurationViews(instance, systemId, viewConfigurationType);
        state.Views.resize(state.ViewConfigViews.size(), {XR_TYPE_VIEW});

        return state;
    }

    inline bool IsRecommendedSwapchainSizeChanged(const std::vector<XrViewConfigurationView>& oldConfigs,
                                                  const std::vector<XrViewConfigurationView>& newConfigs) {
        assert(oldConfigs.size() == newConfigs.size());
        size_t end = (std::min)(oldConfigs.size(), newConfigs.size());
        for (size_t i = 0; i < end; i++) {
            if ((oldConfigs[i].recommendedImageRectWidth != newConfigs[i].recommendedImageRectWidth) ||
                (oldConfigs[i].recommendedImageRectHeight != newConfigs[i].recommendedImageRectHeight)) {
                return true;
            }
        }
        return false;
    }
} // namespace xr
