// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <openxr/openxr.h>
#include <vector>
#include <set>
#include "XrError.h"

namespace xr {
    template <typename TArray, typename TValue>
    inline bool Contains(const TArray& array, const TValue& value) {
        return std::end(array) != std::find(std::begin(array), std::end(array), value);
    }

    template <class A, class B, class TIntersectPredicate>
    inline std::vector<A> Intersect(const std::vector<A>& listA, const std::vector<B>& listB, TIntersectPredicate predicate) {
        std::vector<A> result;
        for (auto& a : listA) {
            for (auto& b : listB) {
                if (predicate(a, b)) {
                    result.push_back(a);
                    break;
                }
            }
        }
        return result;
    }

    inline std::vector<XrApiLayerProperties> EnumerateApiLayerProperties() {
        uint32_t apiLayerCount = 0;
        CHECK_XRCMD(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr));
        std::vector<XrApiLayerProperties> apiLayerProperties(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
        CHECK_XRCMD(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()));
        apiLayerProperties.resize(apiLayerCount);
        return apiLayerProperties;
    }

    inline std::vector<XrExtensionProperties> EnumerateInstanceExtensionProperties(const char* layerName = nullptr) {
        uint32_t extensionCount = 0;
        CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(layerName, 0, &extensionCount, nullptr));
        std::vector<XrExtensionProperties> extensionProperties(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
        CHECK_XRCMD(xrEnumerateInstanceExtensionProperties(layerName, extensionCount, &extensionCount, extensionProperties.data()));
        extensionProperties.resize(extensionCount);
        return extensionProperties;
    }

    inline std::vector<XrViewConfigurationType> EnumerateViewConfigurations(XrInstance instance, XrSystemId systemId) {
        uint32_t viewConfigurationCount = 0;
        CHECK_XRCMD(xrEnumerateViewConfigurations(instance, systemId, 0, &viewConfigurationCount, nullptr));

        std::vector<XrViewConfigurationType> viewConfigs(viewConfigurationCount);
        CHECK_XRCMD(xrEnumerateViewConfigurations(instance, systemId, viewConfigurationCount, &viewConfigurationCount, viewConfigs.data()));
        return viewConfigs;
    }

    inline std::vector<XrViewConfigurationView> EnumerateViewConfigurationViews(XrInstance instance,
                                                                                XrSystemId systemId,
                                                                                XrViewConfigurationType viewConfigurationType) {
        uint32_t viewCount = 0;
        CHECK_XRCMD(xrEnumerateViewConfigurationViews(instance, systemId, viewConfigurationType, 0, &viewCount, nullptr));

        std::vector<XrViewConfigurationView> viewConfigViews(viewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
        CHECK_XRCMD(xrEnumerateViewConfigurationViews(
            instance, systemId, viewConfigurationType, (uint32_t)viewConfigViews.size(), &viewCount, viewConfigViews.data()));

        return viewConfigViews;
    }

    inline std::vector<XrEnvironmentBlendMode> EnumerateEnvironmentBlendModes(XrInstance instance,
                                                                              XrSystemId systemId,
                                                                              XrViewConfigurationType viewConfigType) {
        uint32_t blendModeCount = 0;
        CHECK_XRCMD(xrEnumerateEnvironmentBlendModes(instance, systemId, viewConfigType, 0, &blendModeCount, nullptr));

        std::vector<XrEnvironmentBlendMode> blendModes(blendModeCount);
        CHECK_XRCMD(
            xrEnumerateEnvironmentBlendModes(instance, systemId, viewConfigType, blendModeCount, &blendModeCount, blendModes.data()));

        return blendModes;
    }

    // Pick the first supported EnvironmentBlendMode from runtime's supported list.
    inline XrEnvironmentBlendMode PickEnvironmentBlendMode(const std::vector<XrEnvironmentBlendMode>& systemSupportedBlendModes,
                                                           const std::vector<XrEnvironmentBlendMode>& appSupportedBlendModes) {
        auto blendModeIt = std::find_first_of(systemSupportedBlendModes.begin(),
                                              systemSupportedBlendModes.end(),
                                              appSupportedBlendModes.begin(),
                                              appSupportedBlendModes.end());
        if (blendModeIt == std::end(systemSupportedBlendModes)) {
            throw std::runtime_error("No blend modes supported");
        }

        return *blendModeIt;
    }

    inline std::vector<int64_t> EnumerateSwapchainFormats(XrSession session) {
        uint32_t swapchainFormatsCount = 0;
        CHECK_XRCMD(xrEnumerateSwapchainFormats(session, 0, &swapchainFormatsCount, nullptr));

        std::vector<int64_t> runtimeFormats(swapchainFormatsCount);
        CHECK_XRCMD(xrEnumerateSwapchainFormats(session, (uint32_t)runtimeFormats.size(), &swapchainFormatsCount, runtimeFormats.data()));
        return runtimeFormats;
    }

    // Pick the first supported swapchain format from runtime's supported format list.
    template <typename TSystemPixelFormat, typename TAppPixelFormat>
    inline TAppPixelFormat PickSwapchainFormat(const std::vector<TSystemPixelFormat>& systemSupportedFormats,
                                               const std::vector<TAppPixelFormat>& appSupportedFormats) {
        // Here we prioritize Runtime format preference over App format preference
        auto swapchainFormatIt = std::find_first_of(
            systemSupportedFormats.begin(), systemSupportedFormats.end(), appSupportedFormats.begin(), appSupportedFormats.end());

        if (swapchainFormatIt == std::end(systemSupportedFormats)) {
            throw std::runtime_error("No swapchain format supported");
        }

        return (TAppPixelFormat)*swapchainFormatIt;
    }

    inline std::vector<XrReferenceSpaceType> EnumerateReferenceSpaceTypes(XrSession session) {
        uint32_t spaceCountOutput = 0;
        CHECK_XRCMD(xrEnumerateReferenceSpaces(session, 0, &spaceCountOutput, nullptr));

        std::vector<XrReferenceSpaceType> runtimeSupportedReferenceSpaces(spaceCountOutput);
        CHECK_XRCMD(xrEnumerateReferenceSpaces(
            session, (uint32_t)runtimeSupportedReferenceSpaces.size(), &spaceCountOutput, runtimeSupportedReferenceSpaces.data()));
        return runtimeSupportedReferenceSpaces;
    }

    inline std::set<std::string> QueryActionLocalizedName(XrSession session, XrAction action, XrInputSourceLocalizedNameFlags flags) {
        std::set<std::string> names;
        uint32_t nameCapacityOutput = 0;
        uint32_t sourcesCount = 0;

        XrBoundSourcesForActionEnumerateInfo enumerateBoundSourcesInfo{XR_TYPE_BOUND_SOURCES_FOR_ACTION_ENUMERATE_INFO};
        enumerateBoundSourcesInfo.action = action;
        CHECK_XRCMD(xrEnumerateBoundSourcesForAction(session, &enumerateBoundSourcesInfo, 0, &sourcesCount, nullptr));

        std::vector<XrPath> sourcesBuffer;
        sourcesBuffer.resize(sourcesCount);
        CHECK_XRCMD(
            xrEnumerateBoundSourcesForAction(session, &enumerateBoundSourcesInfo, sourcesCount, &sourcesCount, sourcesBuffer.data()));

        std::string nameBuffer;
        for (uint32_t i = 0; i < sourcesCount; i++) {
            XrInputSourceLocalizedNameGetInfo getLocalizedNameInfo{XR_TYPE_INPUT_SOURCE_LOCALIZED_NAME_GET_INFO};
            getLocalizedNameInfo.sourcePath = sourcesBuffer[i];
            getLocalizedNameInfo.whichComponents = flags;

            CHECK_XRCMD(xrGetInputSourceLocalizedName(session, &getLocalizedNameInfo, 0, &nameCapacityOutput, nullptr));

            nameBuffer.resize(nameCapacityOutput);
            CHECK_XRCMD(
                xrGetInputSourceLocalizedName(session, &getLocalizedNameInfo, nameCapacityOutput, &nameCapacityOutput, nameBuffer.data()));

            // Some names are duplicated due to binding to the same input component and the left and right controllers
            names.emplace(nameBuffer.data());
        }

        return names;
    };

} // namespace xr
