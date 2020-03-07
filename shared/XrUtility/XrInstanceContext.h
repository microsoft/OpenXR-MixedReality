#pragma once

#include <XrUtility/XrError.h>
#include <XrUtility/XrExtensions.h>
#include <XrUtility/XrEnumerate.h>
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrStruct.h>

namespace xr {
    struct ExtensionContext : xr::ExtensionDispatchTable {
        bool SupportsDepthInfo;
        bool SupportsVisibilityMask;
        bool SupportsUnboundedSpace;
        bool SupportsSpatialAnchor;
        bool SupportsHandInteraction;
        bool SupportsHandJointTracking;
        bool SupportsHandMeshTracking;
        bool SupportsSpatialGraphBridge;
    };

    struct ViewConfiguration {
        XrViewConfigurationType Type;
        XrBool32 FovMutable;
        std::vector<XrEnvironmentBlendMode> BlendModes;
    };

    struct SystemContext {
        XrSystemId Id = XR_NULL_SYSTEM_ID;
        XrSystemProperties SystemProperties{XR_TYPE_SYSTEM_PROPERTIES};
        XrSystemHandTrackingPropertiesMSFT HandTrackingSystemProperties{XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_MSFT};
        XrSystemHandTrackingMeshPropertiesMSFT HandMeshSystemProperties{XR_TYPE_SYSTEM_HAND_TRACKING_MESH_PROPERTIES_MSFT};

        XrViewConfigurationType PrimaryViewConfigurationType;
        std::vector<XrViewConfigurationType> SecondaryViewConfigurationTypes;
        std::unordered_map<XrViewConfigurationType, ViewConfiguration> ViewConfigurations;
    };

    class InstanceContext final {
    public:
        NameVersion AppInfo;
        NameVersion EngineInfo;
        ExtensionContext Extensions;
        XrInstanceProperties InstanceProperties{XR_TYPE_INSTANCE_PROPERTIES};

    public:
        InstanceContext(const NameVersion& appInfo,
                        const NameVersion& engineInfo,
                        const std::vector<const char*>& desiredExtensions);

        InstanceContext() = delete;
        InstanceContext(InstanceContext&) = delete;
        InstanceContext(InstanceContext&&) = default;

        XrInstance Handle() const {
            return m_instance.Get();
        }

        std::optional<xr::SystemContext>
        TryGetSystem(XrFormFactor formFactor,
                     XrViewConfigurationType primaryViewConfigurationType,
                     const std::vector<XrViewConfigurationType>& desiredSecondaryViewConfigurationTypes = {}) const;

    private:
        xr::InstanceHandle m_instance;
        std::vector<const char*> FilterSupportedExtensions(const std::vector<const char*>& extensions);
    };

#pragma region Implementation details
    inline InstanceContext::InstanceContext(const NameVersion& appInfo,
                                            const NameVersion& engineInfo,
                                            const std::vector<const char*>& desiredExtensions)
        : AppInfo(appInfo)
        , EngineInfo(engineInfo) {
        // Filter extensions to make sure all are supported by current runtime
        std::vector<const char*> supportedExtensions = FilterSupportedExtensions(desiredExtensions);

        XrInstanceCreateInfo instanceCreateInfo{XR_TYPE_INSTANCE_CREATE_INFO};
        xr::SetEnabledExtensions(instanceCreateInfo, supportedExtensions);
        xr::SetApplicationInfo(instanceCreateInfo.applicationInfo, appInfo, engineInfo);
        CHECK_XRCMD(xrCreateInstance(&instanceCreateInfo, m_instance.Put()));
        Extensions.PopulateDispatchTable(Handle());
        CHECK_XRCMD(xrGetInstanceProperties(Handle(), &InstanceProperties));
    }

    inline std::vector<const char*> InstanceContext::FilterSupportedExtensions(const std::vector<const char*>& desiredExtensions) {

        // Fetch the list of extensions supported by the runtime.
        const std::vector<XrExtensionProperties> extensionProperties = xr::EnumerateInstanceExtensionProperties();

        std::vector<const char*> supportedExtensions;

        // Add supported extensions to the list of enabled extensions
        for (auto& extension : desiredExtensions) {
            for (const auto& property : extensionProperties) {
                if (strcmp(property.extensionName, extension) == 0) {
                    supportedExtensions.push_back(extension);
                    break;
                }
            }
        }

        auto any = [](const std::vector<const char*>& list, const char* item) -> bool {
            return list.end() != std::find_if(list.begin(), list.end(), [&item](auto&& i) { return 0 == strcmp(i, item); });
        };

        // Record extension support in extension context for easy usage.
        Extensions.SupportsDepthInfo = any(supportedExtensions, XR_KHR_COMPOSITION_LAYER_DEPTH_EXTENSION_NAME);
        Extensions.SupportsVisibilityMask = any(supportedExtensions, XR_KHR_VISIBILITY_MASK_EXTENSION_NAME);
        Extensions.SupportsUnboundedSpace = any(supportedExtensions, XR_MSFT_UNBOUNDED_REFERENCE_SPACE_EXTENSION_NAME);
        Extensions.SupportsSpatialAnchor = any(supportedExtensions, XR_MSFT_SPATIAL_ANCHOR_EXTENSION_NAME);
        Extensions.SupportsHandInteraction = any(supportedExtensions, XR_MSFT_HAND_INTERACTION_PREVIEW_EXTENSION_NAME);
        Extensions.SupportsHandJointTracking = any(supportedExtensions, XR_MSFT_HAND_TRACKING_PREVIEW_EXTENSION_NAME);
        Extensions.SupportsHandMeshTracking = any(supportedExtensions, XR_MSFT_HAND_TRACKING_MESH_PREVIEW_EXTENSION_NAME);
        Extensions.SupportsSpatialGraphBridge = any(supportedExtensions, XR_MSFT_SPATIAL_GRAPH_BRIDGE_PREVIEW_EXTENSION_NAME);

        return supportedExtensions;
    }

    inline std::optional<xr::SystemContext>
    InstanceContext::TryGetSystem(XrFormFactor formFactor,
                                  XrViewConfigurationType primaryViewConfigurationType,
                                  const std::vector<XrViewConfigurationType>& desiredSecondaryViewConfigurationTypes) const {
        XrSystemId systemId;
        XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
        systemInfo.formFactor = formFactor;
        XrResult result = xrGetSystem(Handle(), &systemInfo, &systemId);
        if (result == XR_ERROR_FORM_FACTOR_UNAVAILABLE) {
            return std::nullopt; // Cannot find a system with given form factor
        } else {
            CHECK_XRCMD(result);
        }

        const std::vector<XrViewConfigurationType> systemSupportedViewConfigurations = xr::EnumerateViewConfigurations(Handle(), systemId);
        if (systemSupportedViewConfigurations.end() ==
            std::find(systemSupportedViewConfigurations.begin(), systemSupportedViewConfigurations.end(), primaryViewConfigurationType)) {
            return std::nullopt; // Cannot find a system with given primary view configuration
        }

        // Initialize system properties
        SystemContext system{};
        system.Id = systemId;
        if (Extensions.SupportsHandJointTracking) {
            system.SystemProperties.next = &system.HandTrackingSystemProperties;
            if (Extensions.SupportsHandMeshTracking) {
                system.HandTrackingSystemProperties.next = &system.HandMeshSystemProperties;
            }
        }
        CHECK_XRCMD(xrGetSystemProperties(Handle(), systemId, &system.SystemProperties));

        // Initialize view configuration properties and views
        auto getViewConfigDetails = [&](XrViewConfigurationType type) -> ViewConfiguration {
            XrViewConfigurationProperties viewConfigProperties{XR_TYPE_VIEW_CONFIGURATION_PROPERTIES};
            CHECK_XRCMD(xrGetViewConfigurationProperties(Handle(), systemId, type, &viewConfigProperties));

            ViewConfiguration viewConfig{};
            viewConfig.Type = type;
            viewConfig.FovMutable = viewConfigProperties.fovMutable;
            viewConfig.BlendModes = xr::EnumerateEnvironmentBlendModes(Handle(), systemId, type);

            return viewConfig;
        };

        // Record the detail of primary view configuration and all supported secondary view configuration
        system.ViewConfigurations[primaryViewConfigurationType] = getViewConfigDetails(primaryViewConfigurationType);
        for (const XrViewConfigurationType type : desiredSecondaryViewConfigurationTypes) {
            if (systemSupportedViewConfigurations.end() !=
                std::find(systemSupportedViewConfigurations.begin(), systemSupportedViewConfigurations.end(), type)) {
                system.SecondaryViewConfigurationTypes.push_back(type);
                system.ViewConfigurations[type] = getViewConfigDetails(type);
            }
        }

        return system;
    }
#pragma endregion

} // namespace xr

