////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
//
// Shared data types and functions used throughout the Pbr rendering library.
//

#pragma once

#include <vector>
#include <array>
#include <winrt/base.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

namespace Pbr {
    namespace Internal {
        void ThrowIfFailed(HRESULT hr);
    }

    using NodeIndex_t = uint16_t; // This type must align with the type used in the Pbr shaders.

    // Indicates an invalid node index (similar to std::variant_npos)
    inline constexpr Pbr::NodeIndex_t NodeIndex_npos = static_cast<Pbr::NodeIndex_t>(-1);
    constexpr Pbr::NodeIndex_t RootNodeIndex = 0;

    // These colors are in linear color space unless otherwise specified.
    using RGBAColor = DirectX::XMFLOAT4;
    using RGBColor = DirectX::XMFLOAT3;

    // DirectX::Colors are in sRGB color space.
    RGBAColor XM_CALLCONV FromSRGB(DirectX::XMVECTOR color);
    RGBColor XM_CALLCONV RGBFromSRGB(DirectX::XMVECTOR color);

    namespace RGBA {
        constexpr RGBAColor White{1, 1, 1, 1};
        constexpr RGBAColor Black{0, 0, 0, 1};
        constexpr RGBAColor FlatNormal{0.5f, 0.5f, 1, 1};
        constexpr RGBAColor Transparent{0, 0, 0, 0};
    } // namespace RGBA

    namespace RGB {
        constexpr RGBColor White{1, 1, 1};
        constexpr RGBColor Black{0, 0, 0};
    } // namespace RGB

    // Vertex structure used by the PBR shaders.
    struct Vertex {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT4 Tangent;
        DirectX::XMFLOAT4 Color0;
        DirectX::XMFLOAT2 TexCoord0;
        NodeIndex_t ModelTransformIndex; // Index into the node transforms

        static const D3D11_INPUT_ELEMENT_DESC s_vertexDesc[6];
    };

    struct PrimitiveBuilder {
        std::vector<Pbr::Vertex> Vertices;
        std::vector<uint32_t> Indices;

        PrimitiveBuilder& AddAxis(float axisLength = 1.0f,
                                  float axisThickness = 0.1f,
                                  float originAdditionalThickness = 0.01f,
                                  Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex);
        PrimitiveBuilder& AddSphere(float diameter,
                                    uint32_t tessellation,
                                    Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex,
                                    RGBAColor vertexColor = RGBA::White);
        PrimitiveBuilder& AddCube(float sideLength,
                                  Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex,
                                  RGBAColor vertexColor = RGBA::White);
        PrimitiveBuilder& AddCube(DirectX::XMFLOAT3 sideLengths,
                                  Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex,
                                  RGBAColor vertexColor = RGBA::White);
        PrimitiveBuilder& AddCube(DirectX::XMFLOAT3 sideLengths,
                                  DirectX::CXMVECTOR translation,
                                  Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex,
                                  RGBAColor vertexColor = RGBA::White);
        PrimitiveBuilder& AddQuad(DirectX::XMFLOAT2 sideLengths,
                                  DirectX::XMFLOAT2 textureCoord = {1, 1},
                                  Pbr::NodeIndex_t transformIndex = Pbr::RootNodeIndex,
                                  RGBAColor vertexColor = RGBA::White);
    };

    namespace Texture {
        std::array<uint8_t, 4> LoadRGBAUI4(RGBAColor color);

        winrt::com_ptr<ID3D11ShaderResourceView> LoadTextureImage(_In_ ID3D11Device* device,
                                                                  _In_reads_bytes_(fileSize) const uint8_t* fileData,
                                                                  uint32_t fileSize);
        winrt::com_ptr<ID3D11ShaderResourceView> CreateFlatCubeTexture(_In_ ID3D11Device* device,
                                                                       RGBAColor color,
                                                                       DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
        winrt::com_ptr<ID3D11ShaderResourceView> CreateTexture(_In_ ID3D11Device* device,
                                                               _In_reads_bytes_(size) const uint8_t* rgba,
                                                               uint32_t size,
                                                               int width,
                                                               int height,
                                                               DXGI_FORMAT format);
        winrt::com_ptr<ID3D11SamplerState> CreateSampler(_In_ ID3D11Device* device,
                                                         D3D11_TEXTURE_ADDRESS_MODE addressMode = D3D11_TEXTURE_ADDRESS_CLAMP);
    } // namespace Texture
} // namespace Pbr
