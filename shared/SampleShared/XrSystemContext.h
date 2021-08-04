// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <unordered_map>
#include "XrUtility/XrEnumerate.h"
#include "XrUtility/XrExtensionContext.h"
#include "XrUtility/XrStruct.h"
#include "SampleShared/XrViewConfiguration.h"

namespace sample {
    struct SystemContext {
        XrSystemId Id = XR_NULL_SYSTEM_ID;
        XrFormFactor FormFactor{};
        XrSystemProperties Properties{XR_TYPE_SYSTEM_PROPERTIES};
        XrSystemHandTrackingPropertiesEXT HandTrackingProperties{XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT};
        XrSystemHandTrackingMeshPropertiesMSFT HandMeshProperties{XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT};
        XrSystemEyeGazeInteractionPropertiesEXT EyeGazeInteractionProperties{XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT};

        std::vector<XrViewConfigurationType> SupportedPrimaryViewConfigurationTypes;
        std::vector<XrViewConfigurationType> SupportedSecondaryViewConfigurationTypes;
        std::unordered_map<XrViewConfigurationType, sample::ViewProperties> ViewProperties;
    };

    inline std::optional<sample::SystemContext>
    CreateSystemContext(XrInstance instance,
                        const xr::ExtensionContext& extensions,
                        XrFormFactor formFactor,
                        const std::vector<XrViewConfigurationType>& appSupportedViewConfigurationTypes,
                        const std::vector<XrEnvironmentBlendMode>& appSupportedEnvironmentBlendMode) {
        sample::SystemContext system{};
        XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
        systemInfo.formFactor = formFactor;
        XrResult result = xrGetSystem(instance, &systemInfo, &system.Id);
        if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE) {
            return std::nullopt; // Cannot find a system with given form factor
        } else {
            CHECK_XRCMD(result);
        }

        system.FormFactor = formFactor;

        // Initialize system properties including extension system properties.
        if (extensions.SupportsHandJointTracking) {
            xr::InsertExtensionStruct(system.Properties, system.HandTrackingProperties);
        }
        if (extensions.SupportsHandMeshTracking) {
            xr::InsertExtensionStruct(system.Properties, system.HandMeshProperties);
        }
        if (extensions.SupportsEyeGazeInteraction) {
            xr::InsertExtensionStruct(system.Properties, system.EyeGazeInteractionProperties);
        }
        CHECK_XRCMD(xrGetSystemProperties(instance, system.Id, &system.Properties));

        // Initialize view configuration properties and environment blend modes
        const std::vector<XrViewConfigurationType> systemSupportedViewConfigurationTypes =
            xr::EnumerateViewConfigurations(instance, system.Id);

        for (const auto viewConfigType : appSupportedViewConfigurationTypes) {
            if (!xr::Contains(systemSupportedViewConfigurationTypes, viewConfigType)) {
                continue; // The system doesn't support this view configuration
            }

            auto viewProperties = sample::CreateViewProperties(instance, system.Id, viewConfigType, appSupportedEnvironmentBlendMode);
            if (viewProperties.SupportedBlendModes.size() > 0) {
                system.ViewProperties.emplace(viewConfigType, viewProperties);

                if (sample::IsPrimaryViewConfigurationType(viewConfigType)) {
                    system.SupportedPrimaryViewConfigurationTypes.push_back(viewConfigType);
                } else {
                    system.SupportedSecondaryViewConfigurationTypes.push_back(viewConfigType);
                }
            }
        }

        return system;
    }
} // namespace sample
