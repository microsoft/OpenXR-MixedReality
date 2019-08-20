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
#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

namespace xr::dx {
    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId) {
        // Create the DXGI factory.
        winrt::com_ptr<IDXGIFactory1> dxgiFactory;
        CHECK_HRCMD(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory.put())));

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
                                     ID3D11Device** device,
                                     ID3D11DeviceContext** deviceContext) {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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
            // If initialization failed, it may be because device debugging isn't supprted, so retry without that.
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

    // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
    std::vector<D3D_FEATURE_LEVEL> SelectFeatureLevels(D3D_FEATURE_LEVEL minimumFeatureLevel) {
        // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        std::vector<D3D_FEATURE_LEVEL> featureLevels = {D3D_FEATURE_LEVEL_12_1,
                                                        D3D_FEATURE_LEVEL_12_0,
                                                        D3D_FEATURE_LEVEL_11_1,
                                                        D3D_FEATURE_LEVEL_11_0,
                                                        D3D_FEATURE_LEVEL_10_1,
                                                        D3D_FEATURE_LEVEL_10_0};
        featureLevels.erase(std::remove_if(featureLevels.begin(),
                                           featureLevels.end(),
                                           [&](D3D_FEATURE_LEVEL fl) { return fl < minimumFeatureLevel; }),
                            featureLevels.end());
        CHECK_MSG(featureLevels.size() != 0, "Unsupported minimum feature level!");
        return featureLevels;
    }

} // namespace xr::dx
