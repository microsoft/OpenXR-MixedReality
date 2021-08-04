// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "XrUtility/XrEnumerate.h"
#include "SampleShared/XrSystemContext.h"
#include "SampleShared/XrViewConfiguration.h"

namespace sample {
    struct SessionContext {
        const XrSession Handle;
        const XrViewConfigurationType PrimaryViewConfigurationType;
        const XrEnvironmentBlendMode PrimaryViewConfigurationBlendMode;
        std::vector<XrViewConfigurationType> EnabledSecondaryViewConfigurationTypes;

        std::vector<DXGI_FORMAT> SupportedColorSwapchainFormats;
        std::vector<DXGI_FORMAT> SupportedDepthSwapchainFormats;

        // note: all runtimes must support VIEW and LOCAL reference spaces.
        std::vector<XrReferenceSpaceType> SupportedReferenceSpaces;
        bool SupportsStageSpace;
        bool SupportsUnboundedSpace;

        explicit SessionContext(xr::SessionHandle sessionHandle,
                                const sample::SystemContext& system,
                                const xr::ExtensionContext& extensions,
                                XrViewConfigurationType primaryViewConfigurationType,
                                const std::vector<XrViewConfigurationType>& appEnabledSecondaryViewConfigurationTypes,
                                const std::vector<DXGI_FORMAT>& appSupportedColorSwapchainFormats,
                                const std::vector<DXGI_FORMAT>& appSupportedDepthSwapchainFormats)
            : Handle(sessionHandle.Get())
            , m_session(std::move(sessionHandle))
            , PrimaryViewConfigurationType(primaryViewConfigurationType)
            , PrimaryViewConfigurationBlendMode(system.ViewProperties.at(primaryViewConfigurationType).BlendMode) {
            for (const auto secondaryViewConfigurationType : appEnabledSecondaryViewConfigurationTypes) {
                if (!xr::Contains(system.SupportedSecondaryViewConfigurationTypes, secondaryViewConfigurationType)) {
                    continue; // Not supported by the system
                }
                EnabledSecondaryViewConfigurationTypes.push_back(secondaryViewConfigurationType);
            }

            for (auto systemSupportedSwapchainFormat : xr::EnumerateSwapchainFormats(Handle)) {
                if (xr::Contains(appSupportedColorSwapchainFormats, systemSupportedSwapchainFormat)) {
                    SupportedColorSwapchainFormats.push_back(static_cast<DXGI_FORMAT>(systemSupportedSwapchainFormat));
                }
                if (xr::Contains(appSupportedDepthSwapchainFormats, systemSupportedSwapchainFormat)) {
                    SupportedDepthSwapchainFormats.push_back(static_cast<DXGI_FORMAT>(systemSupportedSwapchainFormat));
                }
            }

            SupportedReferenceSpaces = xr::EnumerateReferenceSpaceTypes(Handle);
            assert(xr::Contains(SupportedReferenceSpaces, XR_REFERENCE_SPACE_TYPE_VIEW));
            assert(xr::Contains(SupportedReferenceSpaces, XR_REFERENCE_SPACE_TYPE_LOCAL));
            SupportsStageSpace = xr::Contains(SupportedReferenceSpaces, XR_REFERENCE_SPACE_TYPE_STAGE);
            SupportsUnboundedSpace =
                extensions.SupportsUnboundedSpace && xr::Contains(SupportedReferenceSpaces, XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT);
        }

    private:
        xr::SessionHandle m_session;
    };

    inline std::vector<XrViewConfigurationType> GetAllViewConfigurationTypes(const SessionContext& sessionContext) {
        std::vector<XrViewConfigurationType> result;
        result.push_back(sessionContext.PrimaryViewConfigurationType);
        result.insert(result.end(),
                      sessionContext.EnabledSecondaryViewConfigurationTypes.begin(),
                      sessionContext.EnabledSecondaryViewConfigurationTypes.end());
        return result;
    }

} // namespace sample
