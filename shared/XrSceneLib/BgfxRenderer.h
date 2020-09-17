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
#include "Scene.h"
#include "FrameTime.h"
#include <SampleShared/BgfxUtility.h>

namespace sample::bg {
    //enum class RendererType {
    //    D3D11,
    //    D3D12,
    //};

    void RenderView(const XrRect2Di& imageRect,
                    const float renderTargetClearColor[4],
                    const std::vector<xr::math::ViewProjection>& viewProjections,
                    DXGI_FORMAT colorSwapchainFormat,
                    void* colorTexture,
                    DXGI_FORMAT depthSwapchainFormat,
                    void* depthTexture,
                    const std::vector<std::unique_ptr<Scene>>& activeScenes,
                    const FrameTime& frameTime, 
                    bool& submitProjectionLayer
    );
} // namespace sample::bgfx
