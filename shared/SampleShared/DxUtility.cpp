// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include "DxUtility.h"
#include "Trace.h"

namespace sample::dx {
    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId) {
        // Create the DXGI factory.
        winrt::com_ptr<IDXGIFactory1> dxgiFactory;
        CHECK_HRCMD(CreateDXGIFactory1(winrt::guid_of<IDXGIFactory1>(), dxgiFactory.put_void()));

        for (UINT adapterIndex = 0;; adapterIndex++) {
            // EnumAdapters1 will fail with DXGI_ERROR_NOT_FOUND when there are no more adapters to enumerate.
            winrt::com_ptr<IDXGIAdapter1> dxgiAdapter;
            CHECK_HRCMD(dxgiFactory->EnumAdapters1(adapterIndex, dxgiAdapter.put()));

            DXGI_ADAPTER_DESC1 adapterDesc;
            CHECK_HRCMD(dxgiAdapter->GetDesc1(&adapterDesc));
            if (memcmp(&adapterDesc.AdapterLuid, &adapterId, sizeof(adapterId)) == 0) {
                sample::Trace(L"Using graphics adapter {}", adapterDesc.Description);
                return dxgiAdapter;
            }
        }
    }

    std::tuple<winrt::com_ptr<ID3D11Device>, winrt::com_ptr<ID3D11DeviceContext>>
    CreateD3D11DeviceAndContext(IDXGIAdapter1* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels, bool singleThreadedD3D11Device) {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        if (singleThreadedD3D11Device) {
            creationFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
        }
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Create the Direct3D 11 API device object and a corresponding context.
        D3D_DRIVER_TYPE driverType = adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN;

    TryAgain:
        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        const HRESULT hr = D3D11CreateDevice(adapter,
                                             driverType,
                                             0,
                                             creationFlags,
                                             featureLevels.data(),
                                             (UINT)featureLevels.size(),
                                             D3D11_SDK_VERSION,
                                             device.put(),
                                             nullptr,
                                             deviceContext.put());

        if (FAILED(hr)) {
            // If initialization failed, it may be because device debugging isn't supported, so retry without that.
            if ((creationFlags & D3D11_CREATE_DEVICE_DEBUG) && (hr == DXGI_ERROR_SDK_COMPONENT_MISSING)) {
                creationFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
                goto TryAgain;
            }

            // If the initialization still fails, fall back to the WARP device.
            // For more information on WARP, see: http://go.microsoft.com/fwlink/?LinkId=286690
            if (driverType != D3D_DRIVER_TYPE_WARP) {
                driverType = D3D_DRIVER_TYPE_WARP;
                goto TryAgain;
            }
        }
        if (!singleThreadedD3D11Device) {
            winrt::com_ptr<ID3D11Multithread> d3dMultiThread = device.try_as<ID3D11Multithread>();
            if (d3dMultiThread) {
                d3dMultiThread->SetMultithreadProtected(true);
            }
        }

        return {device, deviceContext};
    }

    SwapchainD3D11 CreateSwapchainD3D11(XrSession session,
                                        DXGI_FORMAT format,
                                        int32_t width,
                                        int32_t height,
                                        uint32_t arrayLength,
                                        uint32_t sampleCount,
                                        XrSwapchainCreateFlags createFlags,
                                        XrSwapchainUsageFlags usageFlags,
                                        std::optional<XrViewConfigurationType> viewConfigurationForSwapchain) {
        SwapchainD3D11 swapchain;
        swapchain.Format = format;
        swapchain.Width = width;
        swapchain.Height = height;

        XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCreateInfo.arraySize = arrayLength;
        swapchainCreateInfo.format = format;
        swapchainCreateInfo.width = width;
        swapchainCreateInfo.height = height;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = sampleCount;
        swapchainCreateInfo.createFlags = createFlags;
        swapchainCreateInfo.usageFlags = usageFlags;

        XrSecondaryViewConfigurationSwapchainCreateInfoMSFT secondaryViewConfigCreateInfo{
            XR_TYPE_SECONDARY_VIEW_CONFIGURATION_SWAPCHAIN_CREATE_INFO_MSFT};
        if (viewConfigurationForSwapchain.has_value()) {
            secondaryViewConfigCreateInfo.viewConfigurationType = viewConfigurationForSwapchain.value();
            swapchainCreateInfo.next = &secondaryViewConfigCreateInfo;
        }

        CHECK_XRCMD(xrCreateSwapchain(session, &swapchainCreateInfo, swapchain.Handle.Put()));

        uint32_t chainLength;
        CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain.Handle.Get(), 0, &chainLength, nullptr));

        swapchain.Images.resize(chainLength, {XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
        CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain.Handle.Get(),
                                               (uint32_t)swapchain.Images.size(),
                                               &chainLength,
                                               reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchain.Images.data())));

        return swapchain;
    }

    std::tuple<XrGraphicsBindingD3D11KHR, winrt::com_ptr<ID3D11Device>, winrt::com_ptr<ID3D11DeviceContext>>
    CreateD3D11Binding(XrInstance instance,
                       XrSystemId systemId,
                       const xr::ExtensionContext& extensions,
                       bool singleThreadedD3D11Device,
                       const std::vector<D3D_FEATURE_LEVEL>& appSupportedFeatureLevels) {
        if (!extensions.SupportsD3D11) {
            throw std::exception("The runtime doesn't support D3D11 extensions.");
        }
        _Analysis_assume_(extensions.xrGetD3D11GraphicsRequirementsKHR != nullptr);

        // Create the D3D11 device for the adapter associated with the system.
        XrGraphicsRequirementsD3D11KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
        CHECK_XRCMD(extensions.xrGetD3D11GraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));
        const winrt::com_ptr<IDXGIAdapter1> adapter = sample::dx::GetAdapter(graphicsRequirements.adapterLuid);

        // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        std::vector<D3D_FEATURE_LEVEL> featureLevels;
        for (auto level : appSupportedFeatureLevels) {
            if (level >= graphicsRequirements.minFeatureLevel) {
                featureLevels.push_back(level);
            }
        }

        if (featureLevels.size() == 0) {
            throw std::exception("Unsupported minimum feature level!");
        }

        auto [device, deviceContext] = sample::dx::CreateD3D11DeviceAndContext(adapter.get(), featureLevels, singleThreadedD3D11Device);

        XrGraphicsBindingD3D11KHR d3d11Binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
        d3d11Binding.device = device.get();

        return {d3d11Binding, device, deviceContext};
    }

} // namespace sample::dx
