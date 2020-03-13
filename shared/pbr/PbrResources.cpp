////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrMaterial.h"

#include <PbrPixelShader.h>
#include <PbrVertexShader.h>
#include <HighlightPixelShader.h>
#include <HighlightVertexShader.h>

using namespace DirectX;

namespace {
    struct SceneConstantBuffer {
        alignas(16) DirectX::XMFLOAT4X4 ViewProjection;
        alignas(16) DirectX::XMFLOAT4 EyePosition;
        alignas(16) DirectX::XMFLOAT3 LightDirection{};
        alignas(16) DirectX::XMFLOAT3 LightDiffuseColor{};
        alignas(16) int NumSpecularMipLevels{1};
        alignas(16) DirectX::XMFLOAT3 HighlightPosition{};
        alignas(16) float AnimationTime{0};
    };

    struct ModelConstantBuffer {
        alignas(16) DirectX::XMFLOAT4X4 ModelToWorld;
    };
} // namespace

namespace Pbr {
    struct Resources::Impl {
        void Initialize(_In_ ID3D11Device* device) {
            Internal::ThrowIfFailed(device->CreateInputLayout(Pbr::Vertex::s_vertexDesc,
                                                              ARRAYSIZE(Pbr::Vertex::s_vertexDesc),
                                                              g_PbrVertexShader,
                                                              sizeof(g_PbrVertexShader),
                                                              Resources.InputLayout.put()));

            // Set up pixel shader.
            Internal::ThrowIfFailed(
                device->CreatePixelShader(g_PbrPixelShader, sizeof(g_PbrPixelShader), nullptr, Resources.PbrPixelShader.put()));
            Internal::ThrowIfFailed(device->CreatePixelShader(
                g_HighlightPixelShader, sizeof(g_HighlightPixelShader), nullptr, Resources.HighlightPixelShader.put()));

            Internal::ThrowIfFailed(
                device->CreateVertexShader(g_PbrVertexShader, sizeof(g_PbrVertexShader), nullptr, Resources.PbrVertexShader.put()));
            Internal::ThrowIfFailed(device->CreateVertexShader(
                g_HighlightVertexShader, sizeof(g_HighlightVertexShader), nullptr, Resources.HighlightVertexShader.put()));

            // Set up the constant buffers.
            static_assert((sizeof(SceneConstantBuffer) % 16) == 0, "Constant Buffer must be divisible by 16 bytes");
            const CD3D11_BUFFER_DESC pbrConstantBufferDesc(sizeof(SceneConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            Internal::ThrowIfFailed(device->CreateBuffer(&pbrConstantBufferDesc, nullptr, Resources.SceneConstantBuffer.put()));

            static_assert((sizeof(ModelConstantBuffer) % 16) == 0, "Constant Buffer must be divisible by 16 bytes");
            const CD3D11_BUFFER_DESC modelConstantBufferDesc(sizeof(ModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            Internal::ThrowIfFailed(device->CreateBuffer(&modelConstantBufferDesc, nullptr, Resources.ModelConstantBuffer.put()));

            // Samplers for environment map and BRDF.
            Resources.EnvironmentMapSampler = Texture::CreateSampler(device);
            Resources.BrdfSampler = Texture::CreateSampler(device);

            CD3D11_BLEND_DESC blendStateDesc(D3D11_DEFAULT);
            Internal::ThrowIfFailed(device->CreateBlendState(&blendStateDesc, Resources.DefaultBlendState.put()));

            D3D11_RENDER_TARGET_BLEND_DESC rtBlendDesc;
            rtBlendDesc.BlendEnable = TRUE;
            rtBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
            rtBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
            rtBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
            rtBlendDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
            rtBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
            rtBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
            rtBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i) {
                blendStateDesc.RenderTarget[i] = rtBlendDesc;
            }
            Internal::ThrowIfFailed(device->CreateBlendState(&blendStateDesc, Resources.AlphaBlendState.put()));

            for (bool doubleSided : {false, true}) {
                for (bool wireframe : {false, true}) {
                    for (bool frontCounterClockwise : {false, true}) {
                        CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
                        rasterizerDesc.CullMode = doubleSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;
                        rasterizerDesc.FillMode = wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
                        rasterizerDesc.FrontCounterClockwise = frontCounterClockwise;
                        Internal::ThrowIfFailed(device->CreateRasterizerState(
                            &rasterizerDesc, Resources.RasterizerStates[doubleSided][wireframe][frontCounterClockwise].put()));
                    }
                }
            }

            for (bool reverseZ : {false, true}) {
                for (bool noWrite : {false, true}) {
                    CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(CD3D11_DEFAULT{});
                    depthStencilDesc.DepthFunc = reverseZ ? D3D11_COMPARISON_GREATER : D3D11_COMPARISON_LESS;
                    depthStencilDesc.DepthWriteMask = noWrite ? D3D11_DEPTH_WRITE_MASK_ZERO : D3D11_DEPTH_WRITE_MASK_ALL;
                    Internal::ThrowIfFailed(
                        device->CreateDepthStencilState(&depthStencilDesc, Resources.DepthStencilStates[reverseZ][noWrite].put()));
                }
            }
        }

        struct DeviceResources {
            winrt::com_ptr<ID3D11SamplerState> BrdfSampler;
            winrt::com_ptr<ID3D11SamplerState> EnvironmentMapSampler;
            winrt::com_ptr<ID3D11InputLayout> InputLayout;
            winrt::com_ptr<ID3D11VertexShader> PbrVertexShader;
            winrt::com_ptr<ID3D11PixelShader> PbrPixelShader;
            winrt::com_ptr<ID3D11VertexShader> HighlightVertexShader;
            winrt::com_ptr<ID3D11PixelShader> HighlightPixelShader;
            winrt::com_ptr<ID3D11Buffer> SceneConstantBuffer;
            winrt::com_ptr<ID3D11Buffer> ModelConstantBuffer;
            winrt::com_ptr<ID3D11ShaderResourceView> BrdfLut;
            winrt::com_ptr<ID3D11ShaderResourceView> SpecularEnvironmentMap;
            winrt::com_ptr<ID3D11ShaderResourceView> DiffuseEnvironmentMap;
            winrt::com_ptr<ID3D11BlendState> AlphaBlendState;
            winrt::com_ptr<ID3D11BlendState> DefaultBlendState;
            winrt::com_ptr<ID3D11RasterizerState>
                RasterizerStates[2][2][2]; // Three dimensions for [DoubleSide][Wireframe][FrontCounterClockWise]
            winrt::com_ptr<ID3D11DepthStencilState> DepthStencilStates[2][2]; // Two dimensions for [ReverseZ][NoWrite]
            mutable std::map<uint32_t, winrt::com_ptr<ID3D11ShaderResourceView>> SolidColorTextureCache;
        };

        DeviceResources Resources;
        SceneConstantBuffer SceneBuffer;
        ModelConstantBuffer ModelBuffer;

        Duration HighlightAnimationTimeStart;
        DirectX::XMFLOAT3 HighlightPulseLocation;

        ShadingMode Shading = ShadingMode::Regular;
        FillMode Fill = FillMode::Solid;
        FrontFaceWindingOrder WindingOrder = FrontFaceWindingOrder::ClockWise;
        bool ReverseZ = false;
        mutable std::mutex m_cacheMutex;
    };

    Resources::Resources(_In_ ID3D11Device* device)
        : m_impl(std::make_unique<Impl>()) {
        m_impl->Initialize(device);
    }

    Resources::Resources(Resources&& resources) = default;

    Resources::~Resources() = default;

    void Resources::SetBrdfLut(_In_ ID3D11ShaderResourceView* brdfLut) {
        m_impl->Resources.BrdfLut.copy_from(brdfLut);
    }

    void Resources::CreateDeviceDependentResources(_In_ ID3D11Device* device) {
        m_impl->Initialize(device);
    }

    void Resources::ReleaseDeviceDependentResources() {
        m_impl->Resources = {};
    }

    winrt::com_ptr<ID3D11Device> Resources::GetDevice() const {
        winrt::com_ptr<ID3D11Device> device;
        m_impl->Resources.SceneConstantBuffer->GetDevice(device.put());
        return device;
    }

    void Resources::SetLight(DirectX::XMFLOAT3 direction, RGBColor diffuseColor) {
        m_impl->SceneBuffer.LightDirection = direction;
        m_impl->SceneBuffer.LightDiffuseColor = diffuseColor;
    }

    void Resources::UpdateAnimationTime(Duration currentTotalTimeElapsed) {
        m_impl->SceneBuffer.AnimationTime =
            std::chrono::duration<float>(currentTotalTimeElapsed - m_impl->HighlightAnimationTimeStart).count();
    }

    void Resources::StartHighlightAnimation(DirectX::XMFLOAT3 location, Duration currentTotalTimeElapsed) {
        m_impl->SceneBuffer.AnimationTime = 0;
        m_impl->HighlightAnimationTimeStart = currentTotalTimeElapsed;
        m_impl->SceneBuffer.HighlightPosition = location;
    }

    void XM_CALLCONV Resources::SetModelToWorld(DirectX::FXMMATRIX modelToWorld, _In_ ID3D11DeviceContext* context) const {
        XMStoreFloat4x4(&m_impl->ModelBuffer.ModelToWorld, XMMatrixTranspose(modelToWorld));
        context->UpdateSubresource(m_impl->Resources.ModelConstantBuffer.get(), 0, nullptr, &m_impl->ModelBuffer, 0, 0);
    }

    void XM_CALLCONV Resources::SetViewProjection(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection) {
        XMStoreFloat4x4(&m_impl->SceneBuffer.ViewProjection, XMMatrixTranspose(XMMatrixMultiply(view, projection)));
        XMStoreFloat4(&m_impl->SceneBuffer.EyePosition, XMMatrixInverse(nullptr, view).r[3]);
    }

    void Resources::SetEnvironmentMap(_In_ ID3D11ShaderResourceView* specularEnvironmentMap,
                                      _In_ ID3D11ShaderResourceView* diffuseEnvironmentMap) {
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        diffuseEnvironmentMap->GetDesc(&desc);
        if (desc.ViewDimension != D3D_SRV_DIMENSION_TEXTURECUBE) {
            throw std::exception("Diffuse Resource View Type is not D3D_SRV_DIMENSION_TEXTURECUBE");
        }

        specularEnvironmentMap->GetDesc(&desc);
        if (desc.ViewDimension != D3D_SRV_DIMENSION_TEXTURECUBE) {
            throw std::exception("Specular Resource View Type is not D3D_SRV_DIMENSION_TEXTURECUBE");
        }

        m_impl->SceneBuffer.NumSpecularMipLevels = desc.TextureCube.MipLevels;
        m_impl->Resources.SpecularEnvironmentMap.copy_from(specularEnvironmentMap);
        m_impl->Resources.DiffuseEnvironmentMap.copy_from(diffuseEnvironmentMap);
    }

    winrt::com_ptr<ID3D11ShaderResourceView> Resources::CreateSolidColorTexture(RGBAColor color) const {
        const std::array<uint8_t, 4> rgba = Texture::LoadRGBAUI4(color);

        // Check cache to see if this flat texture already exists.
        const uint32_t colorKey = *reinterpret_cast<const uint32_t*>(rgba.data());
        {
            std::lock_guard guard(m_impl->m_cacheMutex);
            auto textureIt = m_impl->Resources.SolidColorTextureCache.find(colorKey);
            if (textureIt != m_impl->Resources.SolidColorTextureCache.end()) {
                return textureIt->second;
            }
        }

        winrt::com_ptr<ID3D11ShaderResourceView> texture =
            Pbr::Texture::CreateTexture(GetDevice().get(), rgba.data(), 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM);
        std::lock_guard guard(m_impl->m_cacheMutex);
        // If the key already exists then the existing texture will be returned.
        return m_impl->Resources.SolidColorTextureCache.emplace(colorKey, texture).first->second;
    }

    void Resources::Bind(_In_ ID3D11DeviceContext* context) const {
        context->UpdateSubresource(m_impl->Resources.SceneConstantBuffer.get(), 0, nullptr, &m_impl->SceneBuffer, 0, 0);

        if (m_impl->Shading == ShadingMode::Highlight) {
            context->VSSetShader(m_impl->Resources.HighlightVertexShader.get(), nullptr, 0);
            context->PSSetShader(m_impl->Resources.HighlightPixelShader.get(), nullptr, 0);
        } else {
            context->VSSetShader(m_impl->Resources.PbrVertexShader.get(), nullptr, 0);
            context->PSSetShader(m_impl->Resources.PbrPixelShader.get(), nullptr, 0);
        }

        ID3D11Buffer* vsBuffers[] = {m_impl->Resources.SceneConstantBuffer.get(), m_impl->Resources.ModelConstantBuffer.get()};
        context->VSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Scene, _countof(vsBuffers), vsBuffers);
        ID3D11Buffer* psBuffers[] = {m_impl->Resources.SceneConstantBuffer.get()};
        context->PSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Scene, _countof(psBuffers), psBuffers);
        context->IASetInputLayout(m_impl->Resources.InputLayout.get());

        static_assert(ShaderSlots::DiffuseTexture == ShaderSlots::SpecularTexture + 1, "Diffuse must follow Specular slot");
        static_assert(ShaderSlots::SpecularTexture == ShaderSlots::Brdf + 1, "Specular must follow BRDF slot");
        ID3D11ShaderResourceView* shaderResources[] = {
            m_impl->Resources.BrdfLut.get(), m_impl->Resources.SpecularEnvironmentMap.get(), m_impl->Resources.DiffuseEnvironmentMap.get()};
        context->PSSetShaderResources(Pbr::ShaderSlots::Brdf, _countof(shaderResources), shaderResources);
        ID3D11SamplerState* samplers[] = {m_impl->Resources.BrdfSampler.get(), m_impl->Resources.EnvironmentMapSampler.get()};
        context->PSSetSamplers(ShaderSlots::Brdf, _countof(samplers), samplers);
    }

    void Resources::SetShadingMode(ShadingMode mode) {
        m_impl->Shading = mode;
    }

    ShadingMode Resources::GetShadingMode() const {
        return m_impl->Shading;
    }

    void Resources::SetFillMode(FillMode mode) {
        m_impl->Fill = mode;
    }

    FillMode Resources::GetFillMode() const {
        return m_impl->Fill;
    }

    void Resources::SetFrontFaceWindingOrder(FrontFaceWindingOrder windingOrder) {
        m_impl->WindingOrder = windingOrder;
    }

    FrontFaceWindingOrder Resources::GetFrontFaceWindingOrder() const {
        return m_impl->WindingOrder;
    }

    void Resources::SetDepthFuncReversed(bool reverseZ) {
        m_impl->ReverseZ = reverseZ;
    }

    void Resources::SetBlendState(_In_ ID3D11DeviceContext* context, bool enabled) const {
        context->OMSetBlendState(
            enabled ? m_impl->Resources.AlphaBlendState.get() : m_impl->Resources.DefaultBlendState.get(), nullptr, 0xFFFFFF);
    }

    void Resources::SetRasterizerState(_In_ ID3D11DeviceContext* context, bool doubleSided, bool wireframe) const {
        context->RSSetState(m_impl->Resources
                                .RasterizerStates[doubleSided ? 1 : 0][wireframe ? 1 : 0]
                                                 [m_impl->WindingOrder == FrontFaceWindingOrder::CounterClockWise ? 1 : 0]
                                .get());
    }

    void Resources::SetDepthStencilState(_In_ ID3D11DeviceContext* context, bool disableDepthWrite) const {
        context->OMSetDepthStencilState(m_impl->Resources.DepthStencilStates[m_impl->ReverseZ ? 1 : 0][disableDepthWrite ? 1 : 0].get(), 1);
    }
} // namespace Pbr
