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

#include <winrt/base.h> // winrt::com_ptr
#include <d3dcommon.h>  //ID3DBlob
#include <XrUtility/XrHandle.h>

namespace sample::dx {

    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId);

    void CreateD3D11DeviceAndContext(IDXGIAdapter1* adapter,
                                     const std::vector<D3D_FEATURE_LEVEL>& featureLevels,
                                     bool singleThreaded,
                                     ID3D11Device** device,
                                     ID3D11DeviceContext** deviceContext);

    bool IsSRGBFormat(DXGI_FORMAT format);

    winrt::com_ptr<ID3DBlob> CompileShader(const char* hlsl, const char* entrypoint, const char* shaderTarget);

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
