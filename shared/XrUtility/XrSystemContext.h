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

#include <XrUtility/XrInstanceContext.h>

namespace xr {
    struct ViewConfiguration {
        XrViewConfigurationType Type;
        XrBool32 FovMutable;
        std::vector<XrEnvironmentBlendMode> BlendModes;
    };

    struct SystemContext {
        XrSystemId Id = XR_NULL_SYSTEM_ID;
        XrSystemProperties Properties{XR_TYPE_SYSTEM_PROPERTIES};
        XrSystemHandTrackingPropertiesMSFT HandTrackingProperties{XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_MSFT};
        XrSystemHandTrackingMeshPropertiesMSFT HandMeshProperties{XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT};

        XrViewConfigurationType PrimaryViewConfigurationType;
        std::vector<XrViewConfigurationType> SecondaryViewConfigurationTypes;
        std::unordered_map<XrViewConfigurationType, ViewConfiguration> ViewConfigurations;
    };

    inline std::optional<xr::SystemContext>
    CreateSystemContext(const xr::InstanceContext& instance,
                        const xr::ExtensionContext& extensions,
                        XrFormFactor formFactor,
                        XrViewConfigurationType primaryViewConfigurationType,
                        const std::vector<XrViewConfigurationType>& desiredSecondaryViewConfigurationTypes) {
        SystemContext system{};
        XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
        systemInfo.formFactor = formFactor;
        XrResult result = xrGetSystem(instance.Handle, &systemInfo, &system.Id);
        if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE) {
            return std::nullopt; // Cannot find a system with given form factor
        } else {
            CHECK_XRCMD(result);
        }

        // Initialize system properties including extension system properties.
        if (extensions.SupportsHandJointTracking) {
            xr::InsertExtensionStruct(system.Properties, system.HandTrackingProperties);
        }
        if (extensions.SupportsHandMeshTracking) {
            xr::InsertExtensionStruct(system.Properties, system.HandMeshProperties);
        }
        CHECK_XRCMD(xrGetSystemProperties(instance.Handle, system.Id, &system.Properties));

        // Initialize view configuration properties
        const std::vector<XrViewConfigurationType> systemSupportedViewConfigurations =
            xr::EnumerateViewConfigurations(instance.Handle, system.Id);

        auto supportsViewConfiguration = [&](XrViewConfigurationType type) -> bool {
            return systemSupportedViewConfigurations.end() !=
                   std::find(systemSupportedViewConfigurations.begin(), systemSupportedViewConfigurations.end(), type);
        };

        if (!supportsViewConfiguration(primaryViewConfigurationType)) {
            return std::nullopt; // Cannot find a system with given primary view configuration
        }

        // Initialize view configuration properties and views
        auto getViewConfigDetails = [&](XrViewConfigurationType type) -> ViewConfiguration {
            XrViewConfigurationProperties viewConfigProperties{XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
            CHECK_XRCMD(xrGetViewConfigurationProperties(instance.Handle, system.Id, type, &viewConfigProperties));

            ViewConfiguration viewConfig{};
            viewConfig.Type = type;
            viewConfig.FovMutable = viewConfigProperties.fovMutable;
            viewConfig.BlendModes = xr::EnumerateEnvironmentBlendModes(instance.Handle, system.Id, type);

            return viewConfig;
        };

        // Record the detail of primary view configuration and all supported secondary view configuration
        system.ViewConfigurations[primaryViewConfigurationType] = getViewConfigDetails(primaryViewConfigurationType);
        for (const XrViewConfigurationType secondaryViewConfigType : desiredSecondaryViewConfigurationTypes) {
            if (supportsViewConfiguration(secondaryViewConfigType)) {
                system.SecondaryViewConfigurationTypes.push_back(secondaryViewConfigType);
                system.ViewConfigurations[secondaryViewConfigType] = getViewConfigDetails(secondaryViewConfigType);
            }
        }

        return system;
    }
} // namespace xr

