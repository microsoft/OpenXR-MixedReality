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
#include "BgfxUtility.h"
#include "Trace.h"

#pragma comment(lib, "D3DCompiler.lib")

#include <DirectXMath.h>

#include <bgfx/bgfx.h>
#include <bx/platform.h>
#include <bx/math.h>
#include <bx/pixelformat.h>
#include <bx/file.h>
#include <bx/mutex.h>

#include <bgfx/platform.h>
#include <bgfx/embedded_shader.h>

#include <bx/uint32_t.h>

namespace sample::bg {
    enum class RendererType {
        D3D11,
        D3D12,
    };

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
        //if (!extensions.SupportsD3D11) {
        //    throw std::exception("The runtime doesn't support D3D11 extensions.");
        //}
        //_Analysis_assume_(extensions.xrGetD3D11GraphicsRequirementsKHR != nullptr);

        //// automatically assume any function calling this is using D3D11 rendering
        //auto rendererType = sample::bg::RendererType::D3D11;
        //// Create the D3D11 device for the adapter associated with the system.
        //XrGraphicsRequirementsD3D11KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
        //CHECK_XRCMD(extensions.xrGetD3D11GraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));
        //const winrt::com_ptr<IDXGIAdapter1> adapter = GetAdapter(graphicsRequirements.adapterLuid);

        //DXGI_ADAPTER_DESC desc{};
        //adapter->GetDesc(&desc);

        //bgfx::renderFrame();
        bgfx::Init init;
        
        /*switch (rendererType) {
        case sample::bg::RendererType::D3D11:
            init.type = bgfx::RendererType::Direct3D11;
            break;

        case sample::bg::RendererType::D3D12:
            init.type = bgfx::RendererType::Direct3D12;
            break;

        default:
            CHECK(false);
        }*/
        //init.vendorId = static_cast<uint16_t>(desc.VendorId);
        //init.deviceId = static_cast<uint16_t>(desc.DeviceId);
        //init.resolution.width = 1280;
        //init.resolution.height = 720;
        //init.resolution.reset = BGFX_RESET_SRGB_BACKBUFFER;

        //bgfx::init(init);
 
        
        

        //bgfx::frame();


        // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        //std::vector<D3D_FEATURE_LEVEL> featureLevels;
        //for (auto level : appSupportedFeatureLevels) {
        //    if (level >= graphicsRequirements.minFeatureLevel) {
        //        featureLevels.push_back(level);
        //    }
        //}

        //if (featureLevels.size() == 0) {
        //    throw std::exception("Unsupported minimum feature level!");
        //}

        /*ID3D11Device* rawDevicePtr = (ID3D11Device*)(bgfx::getInternalData()->context);
        ID3D11DeviceContext* ppImmediateContext;
        rawDevicePtr->GetImmediateContext(&ppImmediateContext);*/

        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        /*device.copy_from(rawDevicePtr);
        //deviceContext.copy_from(ppImmediateContext);*/

        XrGraphicsBindingD3D11KHR d3d11Binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
        //d3d11Binding.device = device.get();

        return {d3d11Binding, device, deviceContext};

        
    }

} // namespace sample::bgfx
