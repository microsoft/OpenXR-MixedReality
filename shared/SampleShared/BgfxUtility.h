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


namespace sample::bg {
    //enum class RendererType {
    //    D3D11,
    //    D3D12,
    //};


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

    
    void RenderView(const XrRect2Di& imageRect,
                    const float renderTargetClearColor[4],
                    const std::vector<xr::math::ViewProjection>& viewProjections,
                    DXGI_FORMAT colorSwapchainFormat,
                    void* colorTexture,
                    DXGI_FORMAT depthSwapchainFormat,
                    void* depthTexture
                    ,const std::vector<std::unique_ptr<Scene>>& activeScenes,
                    const FrameTime& frameTime, 
                    bool& submitProjectionLayer
    );
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

    struct CachedFrameBuffer {
        std::vector<unique_bgfx_handle<bgfx::FrameBufferHandle>> FrameBuffers;
        winrt::com_ptr<ID3D12CommandAllocator> CommandAllocator;
    };
    
} // namespace sample::bgfx
