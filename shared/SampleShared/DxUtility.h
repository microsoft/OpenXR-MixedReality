// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <winrt/base.h> // winrt::com_ptr
#include <d3dcommon.h>  //ID3DBlob
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrExtensionContext.h>

namespace sample::dx {

    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId);

    std::tuple<winrt::com_ptr<ID3D11Device>, winrt::com_ptr<ID3D11DeviceContext>> CreateD3D11DeviceAndContext(
        IDXGIAdapter1* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels, bool singleThreadedD3D11Device);

    std::tuple<XrGraphicsBindingD3D11KHR, winrt::com_ptr<ID3D11Device>, winrt::com_ptr<ID3D11DeviceContext>>
    CreateD3D11Binding(XrInstance instance,
                       XrSystemId systemId,
                       const xr::ExtensionContext& extensions,
                       bool singleThreadedD3D11Device,
                       const std::vector<D3D_FEATURE_LEVEL>& appSupportedFeatureLevels);

    struct SwapchainD3D11 {
        xr::SwapchainHandle Handle;
        DXGI_FORMAT Format{DXGI_FORMAT_UNKNOWN};
        int32_t Width{0};
        int32_t Height{0};
        std::vector<XrSwapchainImageD3D11KHR> Images;
    };

    SwapchainD3D11 CreateSwapchainD3D11(XrSession session,
                                        DXGI_FORMAT format,
                                        int32_t width,
                                        int32_t height,
                                        uint32_t arrayLength,
                                        uint32_t sampleCount,
                                        XrSwapchainCreateFlags createFlags,
                                        XrSwapchainUsageFlags usageFlags,
                                        std::optional<XrViewConfigurationType> viewConfigurationForSwapchain = std::nullopt);
} // namespace sample::dx
