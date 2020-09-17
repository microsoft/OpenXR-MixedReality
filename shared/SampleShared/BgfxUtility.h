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
#include "pch.h"
//#include "BgfxUtility.h"
#include "Trace.h"

#pragma comment(lib, "D3DCompiler.lib")

#include <DirectXMath.h>

#include <memory>

#include <winrt/base.h> // winrt::com_ptr
#include <d3dcommon.h>  //ID3DBlob
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrExtensionContext.h>
#include <wil/resource.h>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/file.h>
#include <bx/uint32_t.h>
#include "bgfx_utils.h"
#include "../XrSceneLib/Scene.h"

template <typename bgfx_handle_t>
struct bgfx_handle_wrapper_t : public bgfx_handle_t {
    bgfx_handle_wrapper_t() {
        this->idx = bgfx::kInvalidHandle;
    }

    bgfx_handle_wrapper_t(const bgfx_handle_t& handle) {
        this->idx = handle.idx;
    }

    bgfx_handle_wrapper_t(const uint16_t& handle) {
        this->idx = handle;
    }

    operator uint16_t() const {
        return this->idx;
    }
};

template <typename bgfx_handle_t, typename close_fn_t = void (*)(bgfx_handle_t), close_fn_t close_fn = bgfx::destroy>
using unique_bgfx_handle = wil::unique_any<bgfx_handle_wrapper_t<bgfx_handle_t>,
                                           close_fn_t,
                                           close_fn,
                                           wil::details::pointer_access_all,
                                           bgfx_handle_wrapper_t<bgfx_handle_t>,
                                           decltype(bgfx::kInvalidHandle),
                                           bgfx::kInvalidHandle,
                                           bgfx_handle_wrapper_t<bgfx_handle_t>>;

template <typename bgfx_handle_t, typename close_fn_t = void (*)(bgfx_handle_t), close_fn_t close_fn = bgfx::destroy>
using shared_bgfx_handle = wil::shared_any<unique_bgfx_handle<bgfx_handle_t, close_fn_t, close_fn>>;

namespace sample::bg {
    //enum class RendererType {
    //    D3D11,
    //    D3D12,
    //};

    struct Swapchain {
        virtual ~Swapchain() = default;

        xr::SwapchainHandle Handle;
        DXGI_FORMAT Format{DXGI_FORMAT_UNKNOWN};
        uint32_t Width{0};
        uint32_t Height{0};
        uint32_t ArraySize{0};
    };
    struct SwapchainD3D11 : public sample::bg::Swapchain {
        std::vector<XrSwapchainImageD3D11KHR> Images;
    };

    struct SwapchainD3D12 : public sample::bg::Swapchain {
        // std::vector<XrSwapchainImageD3D12KHR> Images;
    };

    static bx::FileReaderI* s_fileReader = NULL;
    static bx::FileWriterI* s_fileWriter = NULL;

    void InitializeBxResources();
    void FreeBxResources();

    bx::FileReaderI* getFileReader();
    bx::FileWriterI* getFileWriter();
    bx::AllocatorI* getAllocator();

    bool IsSRGBFormat(DXGI_FORMAT format);
    bgfx::TextureFormat::Enum DxgiFormatToBgfxFormat(DXGI_FORMAT format);
    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId);

    std::tuple<XrGraphicsBindingD3D11KHR, winrt::com_ptr<ID3D11Device>, winrt::com_ptr<ID3D11DeviceContext>>
    __stdcall
    BgfxCreateD3D11Binding(XrInstance instance,
                       XrSystemId systemId,
                       const xr::ExtensionContext& extensions,
                       bool singleThreadedD3D11Device,
                       const std::vector<D3D_FEATURE_LEVEL>& appSupportedFeatureLevels);

    std::unique_ptr<Swapchain> __stdcall CreateSwapchain(
        XrSession session,
        DXGI_FORMAT format,
        uint32_t width,
        uint32_t height,
        uint32_t arraySize,
        uint32_t sampleCount,
        XrSwapchainCreateFlags createFlags,
        XrSwapchainUsageFlags usageFlags,
        std::optional<XrViewConfigurationType> viewConfigurationForSwapchain = std::nullopt);

} // namespace sample::bgfx
