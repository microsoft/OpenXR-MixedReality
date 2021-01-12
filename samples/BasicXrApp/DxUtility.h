// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <d3dcommon.h>  //ID3DBlob

namespace sample::dx {

    winrt::com_ptr<IDXGIAdapter1> GetAdapter(LUID adapterId);

    void CreateD3D11DeviceAndContext(IDXGIAdapter1* adapter,
                                     const std::vector<D3D_FEATURE_LEVEL>& featureLevels,
                                     ID3D11Device** device,
                                     ID3D11DeviceContext** deviceContext);

    winrt::com_ptr<ID3DBlob> CompileShader(const char* hlsl, const char* entrypoint, const char* shaderTarget);
} // namespace sample::dx
