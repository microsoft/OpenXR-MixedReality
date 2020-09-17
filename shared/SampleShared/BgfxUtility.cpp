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
#include "BgfxUtility.h"
#include "Trace.h"

#pragma comment(lib, "D3DCompiler.lib")

#include <DirectXMath.h>

namespace sample::bg {
    std::map<std::tuple<void*, void*>, CachedFrameBuffer> m_cachedFrameBuffers;
    bx::AllocatorI* getDefaultAllocator() {
        BX_PRAGMA_DIAGNOSTIC_PUSH();
        BX_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(4459); // warning C4459: declaration of 's_allocator' hides global declaration
        BX_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wshadow");
        static bx::DefaultAllocator s_allocator;
        return &s_allocator;
        BX_PRAGMA_DIAGNOSTIC_POP();
    }
    extern bx::AllocatorI* getDefaultAllocator();
    bx::AllocatorI* g_allocator = getDefaultAllocator();
    typedef bx::StringT<&g_allocator> String;

    static String s_currentDir;

    class FileReader : public bx::FileReader {
        typedef bx::FileReader super;

    public:
        virtual bool open(const bx::FilePath& _filePath, bx::Error* _err) override {
            String filePath(s_currentDir);
            filePath.append(_filePath);
            return super::open(filePath.getPtr(), _err);
        }
    };
    class FileWriter : public bx::FileWriter {
        typedef bx::FileWriter super;

    public:
        virtual bool open(const bx::FilePath& _filePath, bool _append, bx::Error* _err) override {
            String filePath(s_currentDir);
            filePath.append(_filePath);
            return super::open(filePath.getPtr(), _append, _err);
        }
    };

    void InitializeBxResources() {
        s_fileReader = BX_NEW(g_allocator, FileReader);
        s_fileWriter = BX_NEW(g_allocator, FileWriter);
    }
    void FreeBxResources(){
        BX_DELETE(g_allocator, s_fileReader);
        s_fileReader = NULL;

        BX_DELETE(g_allocator, s_fileWriter);
        s_fileWriter = NULL;
    }
    
    bx::FileReaderI* getFileReader() {
        return s_fileReader;
    }

    bx::FileWriterI* getFileWriter() {
        return s_fileWriter;
    }

    bx::AllocatorI* getAllocator() {
        if (NULL == g_allocator) {
            g_allocator = getDefaultAllocator();
        }

        return g_allocator;
    }

    bool IsSRGBFormat(DXGI_FORMAT format) {
        switch (format) {
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return true;

        default:
            return false;
        }
    }

    bgfx::TextureFormat::Enum DxgiFormatToBgfxFormat(DXGI_FORMAT format) {
        switch (format) {
            // Color Formats
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            return bgfx::TextureFormat::BGRA8;

        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            return bgfx::TextureFormat::RGBA8;

            // Depth Formats
        case DXGI_FORMAT_D16_UNORM:
            return bgfx::TextureFormat::D16;

        case DXGI_FORMAT_D24_UNORM_S8_UINT:
            return bgfx::TextureFormat::D24S8;

        case DXGI_FORMAT_D32_FLOAT:
            return bgfx::TextureFormat::D32F;

        default:
            throw std::exception{/* Unsupported texture format */};
        }
    }

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

std::unique_ptr<Swapchain> __stdcall CreateSwapchain(
                                                XrSession session,
                                               DXGI_FORMAT format,
                                               uint32_t width,
                                               uint32_t height,
                                               uint32_t arraySize,
                                               uint32_t sampleCount,
                                               XrSwapchainCreateFlags createFlags,
                                               XrSwapchainUsageFlags usageFlags,
                                               std::optional<XrViewConfigurationType> viewConfigurationForSwapchain) {
        Swapchain* swapchain;
        std::unique_ptr<SwapchainD3D11> swapchainD3D11;
        std::unique_ptr<SwapchainD3D12> swapchainD3D12;
        switch (bgfx::getRendererType()) {
        case bgfx::RendererType::Direct3D11:
            swapchainD3D11 = std::make_unique<SwapchainD3D11>();
            swapchain = swapchainD3D11.get();
            break;

        case bgfx::RendererType::Direct3D12:
            swapchainD3D12 = std::make_unique<SwapchainD3D12>();
            swapchain = swapchainD3D12.get();
            break;

        default:
            CHECK(false);
        }
        swapchain->Format = format;
        swapchain->Width = width;
        swapchain->Height = height;
        swapchain->ArraySize = arraySize;

        XrSwapchainCreateInfo swapchainCreateInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCreateInfo.arraySize = arraySize;
        swapchainCreateInfo.format = format;
        swapchainCreateInfo.width = width;
        swapchainCreateInfo.height = height;
        swapchainCreateInfo.mipCount = 1;
        swapchainCreateInfo.faceCount = 1;
        swapchainCreateInfo.sampleCount = sampleCount;
        swapchainCreateInfo.createFlags = createFlags;
        swapchainCreateInfo.usageFlags = usageFlags;

        CHECK_XRCMD(xrCreateSwapchain(session, &swapchainCreateInfo, swapchain->Handle.Put()));

        uint32_t chainLength;
        CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain->Handle.Get(), 0, &chainLength, nullptr));

        switch (bgfx::getRendererType()) {
        case bgfx::RendererType::Direct3D11:
            swapchainD3D11->Images.resize(chainLength, {XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
            CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain->Handle.Get(),
                                                   (uint32_t)swapchainD3D11->Images.size(),
                                                   &chainLength,
                                                   reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainD3D11->Images.data())));

            return swapchainD3D11;

        /*case sample::bg::RendererType::D3D12:
            swapchainD3D12->Images.resize(chainLength, {XR_TYPE_SWAPCHAIN_IMAGE_D3D12_KHR});
            CHECK_XRCMD(xrEnumerateSwapchainImages(swapchain->Handle.Get(),
                                                   (uint32_t)swapchainD3D12->Images.size(),
                                                   &chainLength,
                                                   reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainD3D12->Images.data())));

            return swapchainD3D12;*/

        default:
            CHECK(false);
        }
    }

    std::tuple<XrGraphicsBindingD3D11KHR,
               winrt::com_ptr<ID3D11Device>,
               winrt::com_ptr<ID3D11DeviceContext>> __stdcall
    BgfxCreateD3D11Binding(XrInstance instance,
                       XrSystemId systemId,
                       const xr::ExtensionContext& extensions,
                       bool singleThreadedD3D11Device,
                       const std::vector<D3D_FEATURE_LEVEL>& appSupportedFeatureLevels) {
        if (!extensions.SupportsD3D11) {
            throw std::exception("The runtime doesn't support D3D11 extensions.");
        }
        _Analysis_assume_(extensions.xrGetD3D11GraphicsRequirementsKHR != nullptr);

        // automatically assume any function calling this is using D3D11 rendering
        auto rendererType = bgfx::RendererType::Direct3D11;
        // Create the D3D11 device for the adapter associated with the system.
        XrGraphicsRequirementsD3D11KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
        CHECK_XRCMD(extensions.xrGetD3D11GraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));
        const winrt::com_ptr<IDXGIAdapter1> adapter = GetAdapter(graphicsRequirements.adapterLuid);

        DXGI_ADAPTER_DESC desc{};
        adapter->GetDesc(&desc);

        bgfx::renderFrame();
        bgfx::Init init;
        
        switch (rendererType) {
        case bgfx::RendererType::Direct3D11:
            init.type = bgfx::RendererType::Direct3D11;
            break;

        case bgfx::RendererType::Direct3D12:
            init.type = bgfx::RendererType::Direct3D12;
            break;

        default:
            CHECK(false);
        }
        init.vendorId = static_cast<uint16_t>(desc.VendorId);
        init.deviceId = static_cast<uint16_t>(desc.DeviceId);
        init.resolution.width = 1280;
        init.resolution.height = 720;
        init.resolution.reset = BGFX_RESET_SRGB_BACKBUFFER;
        init.debug = true;
        bgfx::init(init);
        bgfx::frame();

        //Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        std::vector<D3D_FEATURE_LEVEL> featureLevels;
        for (auto level : appSupportedFeatureLevels) {
            if (level >= graphicsRequirements.minFeatureLevel) {
                featureLevels.push_back(level);
            }
        }

        if (featureLevels.size() == 0) {
            throw std::exception("Unsupported minimum feature level!");
        }

        ID3D11Device* rawDevicePtr = (ID3D11Device*)(bgfx::getInternalData()->context);
        ID3D11DeviceContext* ppImmediateContext;
        rawDevicePtr->GetImmediateContext(&ppImmediateContext);

        winrt::com_ptr<ID3D11Device> device;
        winrt::com_ptr<ID3D11DeviceContext> deviceContext;
        device.copy_from(rawDevicePtr);
        deviceContext.copy_from(ppImmediateContext);

        XrGraphicsBindingD3D11KHR d3d11Binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
        d3d11Binding.device = device.get();

        return {d3d11Binding, device, deviceContext};

        
    }

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
    ) {
        
        //static int frameCount = 0;
        //wchar_t text_buffer[20] = {0};                                               // temporary buffer
        //swprintf(text_buffer, _countof(text_buffer), L"frame: %d \n", ++frameCount); // convert
        //OutputDebugString(text_buffer);                                              // print

        const uint32_t viewInstanceCount = (uint32_t)viewProjections.size();
        /* CHECK_MSG(viewInstanceCount <= CubeShader::MaxViewInstance,
                   "Sample shader supports 2 or fewer view instances. Adjust shader to accommodate more.");*/

        const bool reversedZ = viewProjections[0].NearFar.Near > viewProjections[0].NearFar.Far;
        const float depthClearValue = reversedZ ? 0.f : 1.f;
        const uint64_t state =
            BGFX_STATE_WRITE_MASK | (reversedZ ? BGFX_STATE_DEPTH_TEST_GREATER : BGFX_STATE_DEPTH_TEST_LESS) | BGFX_STATE_CULL_CCW;

        auto iter = m_cachedFrameBuffers.find({colorTexture, depthTexture});
        if (iter == m_cachedFrameBuffers.end()) {
            unique_bgfx_handle<bgfx::TextureHandle> colorTex(
                bgfx::createTexture2D(1,
                                      1,
                                      false,
                                      (uint16_t)viewInstanceCount,
                                      DxgiFormatToBgfxFormat(colorSwapchainFormat),
                                      (IsSRGBFormat(colorSwapchainFormat) ? BGFX_TEXTURE_SRGB : 0) | BGFX_TEXTURE_RT));
            unique_bgfx_handle<bgfx::TextureHandle> depthTex(bgfx::createTexture2D(
                1, 1, false, (uint16_t)viewInstanceCount, DxgiFormatToBgfxFormat(depthSwapchainFormat), BGFX_TEXTURE_RT_WRITE_ONLY));

            // Force BGFX to create the texture now, which is necessary in order to use overrideInternal.
            bgfx::frame();
            auto x = colorTex.get();
            auto y = depthTex.get();
            bgfx::overrideInternal(x, reinterpret_cast<uintptr_t>(colorTexture));
            bgfx::overrideInternal(y, reinterpret_cast<uintptr_t>(depthTexture));

            CachedFrameBuffer frameBuffer;

            if (bgfx::getRendererType() == bgfx::RendererType::Direct3D12) {
                ID3D12Device* device = reinterpret_cast<ID3D12Device*>(bgfx::getInternalData()->context);

                CHECK_HRCMD(device->CreateCommandAllocator(
                    D3D12_COMMAND_LIST_TYPE_DIRECT, IID_ID3D12CommandAllocator, frameBuffer.CommandAllocator.put_void()));

                winrt::com_ptr<ID3D12GraphicsCommandList> commandList;
                CHECK_HRCMD(device->CreateCommandList(0,
                                                      D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                      frameBuffer.CommandAllocator.get(),
                                                      nullptr,
                                                      winrt::guid_of<ID3D12GraphicsCommandList>(),
                                                      commandList.put_void()));

                // Bgfx is expecting a depth texture with COMMON state, but OpenXR is providing a texture with DEPTH_WRITE state. Need to
                // switch it.
                D3D12_RESOURCE_BARRIER barrier;
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
                barrier.Transition.pResource = reinterpret_cast<ID3D12Resource*>(depthTexture);
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                commandList->ResourceBarrier(1, &barrier);

                winrt::com_ptr<ID3D12CommandQueue> commandQueue;
                UINT size = sizeof(commandQueue.get());
                CHECK_HRCMD(device->GetPrivateData(IID_ID3D12CommandQueue, &size, commandQueue.put()));

                CHECK_HRCMD(commandList->Close());

                ID3D12CommandList* commandLists[] = {commandList.get()};
                commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);
            }

            frameBuffer.FrameBuffers.resize(viewInstanceCount);
            for (uint32_t k = 0; k < viewInstanceCount; k++) {
                std::array<bgfx::Attachment, 2> attachments{};
                attachments[0].init(colorTex.get(), bgfx::Access::Write, static_cast<uint16_t>(k));
                attachments[1].init(depthTex.get(), bgfx::Access::Write, static_cast<uint16_t>(k));

                frameBuffer.FrameBuffers[k] = unique_bgfx_handle<bgfx::FrameBufferHandle>(
                    bgfx::createFrameBuffer(static_cast<uint8_t>(attachments.size()), attachments.data(), false));
            }
            iter = m_cachedFrameBuffers.emplace(std::make_tuple(colorTexture, depthTexture), std::move(frameBuffer)).first;
        }

        std::vector<unique_bgfx_handle<bgfx::FrameBufferHandle>>& frameBuffers = iter->second.FrameBuffers;

        uint32_t clearColorRgba;
        uint8_t* dst = reinterpret_cast<uint8_t*>(&clearColorRgba);
        dst[3] = uint8_t(bx::toUnorm(renderTargetClearColor[0], 255.0f));
        dst[2] = uint8_t(bx::toUnorm(renderTargetClearColor[1], 255.0f));
        dst[1] = uint8_t(bx::toUnorm(renderTargetClearColor[2], 255.0f));
        dst[0] = uint8_t(bx::toUnorm(renderTargetClearColor[3], 255.0f));

        // Render each view
        for (uint32_t k = 0; k < viewInstanceCount; k++) {
            const DirectX::XMMATRIX spaceToView = xr::math::LoadInvertedXrPose(viewProjections[k].Pose);
            const DirectX::XMMATRIX projectionMatrix = ComposeProjectionMatrix(viewProjections[k].Fov, viewProjections[k].NearFar);

            DirectX::XMFLOAT4X4 view;
            DirectX::XMFLOAT4X4 proj;
            DirectX::XMStoreFloat4x4(&view, spaceToView);
            DirectX::XMStoreFloat4x4(&proj, projectionMatrix);

            const bgfx::ViewId viewId = bgfx::ViewId(k);
            bgfx::setViewFrameBuffer(viewId, frameBuffers[k].get());

            bgfx::setViewClear(viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clearColorRgba, depthClearValue, 0);  

            bgfx::setViewRect(viewId,
                              (uint16_t)imageRect.offset.x,
                              (uint16_t)imageRect.offset.y,
                              (uint16_t)imageRect.extent.width,
                              (uint16_t)imageRect.extent.height);
            bgfx::setViewTransform(viewId, view.m, proj.m);

            bgfx::touch(viewId);

            //activeScenes[0]->Render(frameTime, viewId);
            //submitProjectionLayer = true;
            {
                for (const std::unique_ptr<Scene>& scene : activeScenes) {
                    if (scene->IsActive() && !std::empty(scene->GetSceneObjects())) {
                        submitProjectionLayer = true;
                        scene->Render(frameTime, viewId);
                    }
                }
            }

        }
        bgfx::frame();
    }
    

} // namespace sample::bgfx
