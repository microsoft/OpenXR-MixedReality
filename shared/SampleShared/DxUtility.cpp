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
#include "pch.h"
#include "DxUtility.h"
#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

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
                DEBUG_PRINT("Using graphics adapter %ws", adapterDesc.Description);
                return dxgiAdapter;
            }
        }
    }

    void CreateD3D11DeviceAndContext(IDXGIAdapter1* adapter,
                                     const std::vector<D3D_FEATURE_LEVEL>& featureLevels,
                                     bool singleThreaded,
                                     ID3D11Device** device,
                                     ID3D11DeviceContext** deviceContext) {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

        if (singleThreaded) {
            creationFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;
        }
#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Create the Direct3D 11 API device object and a corresponding context.
        D3D_DRIVER_TYPE driverType = adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN;

    TryAgain:
        const HRESULT hr = D3D11CreateDevice(adapter,
                                             driverType,
                                             0,
                                             creationFlags,
                                             featureLevels.data(),
                                             (UINT)featureLevels.size(),
                                             D3D11_SDK_VERSION,
                                             device,
                                             nullptr,
                                             deviceContext);

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
        if (!singleThreaded) {
            winrt::com_ptr<ID3D11Device> comPtr;
            comPtr.copy_from(*device);
            winrt::com_ptr<ID3D11Multithread> d3dMultiThread = comPtr.try_as<ID3D11Multithread>();
            if (d3dMultiThread) {
                d3dMultiThread->SetMultithreadProtected(true);
            }
        }
    }

    winrt::com_ptr<ID3DBlob> CompileShader(const char* hlsl, const char* entrypoint, const char* shaderTarget) {
        winrt::com_ptr<ID3DBlob> compiled;
        winrt::com_ptr<ID3DBlob> errMsgs;
        DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;

#ifdef _DEBUG
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        HRESULT hr =
            D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, entrypoint, shaderTarget, flags, 0, compiled.put(), errMsgs.put());
        if (FAILED(hr)) {
            std::string errMsg((const char*)errMsgs->GetBufferPointer(), errMsgs->GetBufferSize());
            DEBUG_PRINT("D3DCompile failed %X: %s", hr, errMsg.c_str());
            CHECK_HRESULT(hr, "D3DCompile failed");
        }

        return compiled;
    }

    bool IsSRGBFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return true;
        default:
            return false;
        }
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

        XrSwapchainSecondaryViewConfigurationCreateInfoMSFT secondaryViewConfigCreateInfo{
            XR_TYPE_SWAPCHAIN_SECONDARY_VIEW_CONFIGURATION_CREATE_INFO_MSFT};
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
} // namespace sample::dx
