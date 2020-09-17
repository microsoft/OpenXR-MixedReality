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
#include "BgfxRenderer.h"

namespace sample::bg {

    struct CachedFrameBuffer {
        std::vector<unique_bgfx_handle<bgfx::FrameBufferHandle>> FrameBuffers;
        winrt::com_ptr<ID3D12CommandAllocator> CommandAllocator;
    };

    std::map<std::tuple<void*, void*>, CachedFrameBuffer> s_cachedFrameBuffers;

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

        auto iter = s_cachedFrameBuffers.find({colorTexture, depthTexture});
        if (iter == s_cachedFrameBuffers.end()) {
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
            iter = s_cachedFrameBuffers.emplace(std::make_tuple(colorTexture, depthTexture), std::move(frameBuffer)).first;
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
