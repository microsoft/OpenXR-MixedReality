////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrMaterial.h"

using namespace DirectX;

namespace Pbr {
    Material::Material(Pbr::Resources const& pbrResources) {
        const CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ConstantBufferData), D3D11_BIND_CONSTANT_BUFFER);
        Internal::ThrowIfFailed(pbrResources.GetDevice()->CreateBuffer(&constantBufferDesc, nullptr, m_constantBuffer.put()));
    }

    std::shared_ptr<Material> Material::Clone(Pbr::Resources const& pbrResources) const {
        auto clone = std::make_shared<Material>(pbrResources);
        clone->Name = Name;
        clone->Hidden = Hidden;
        clone->m_parameters = m_parameters;
        clone->m_textures = m_textures;
        clone->m_samplers = m_samplers;
        clone->m_alphaBlended = m_alphaBlended;
        clone->m_doubleSided = m_doubleSided;
        return clone;
    }

    /* static */
    std::shared_ptr<Material> Material::CreateFlat(const Resources& pbrResources,
                                                   RGBAColor baseColorFactor,
                                                   float roughnessFactor /* = 1.0f */,
                                                   float metallicFactor /* = 0.0f */,
                                                   RGBColor emissiveFactor /* = XMFLOAT3(0, 0, 0) */) {
        std::shared_ptr<Material> material = std::make_shared<Material>(pbrResources);

        if (baseColorFactor.w < 1.0f) { // Alpha channel
            material->SetAlphaBlended(true);
        }

        Pbr::Material::ConstantBufferData& parameters = material->Parameters();
        parameters.BaseColorFactor = baseColorFactor;
        parameters.EmissiveFactor = emissiveFactor;
        parameters.MetallicFactor = metallicFactor;
        parameters.RoughnessFactor = roughnessFactor;

        const winrt::com_ptr<ID3D11SamplerState> defaultSampler = Pbr::Texture::CreateSampler(pbrResources.GetDevice().get());
        material->SetTexture(ShaderSlots::BaseColor, pbrResources.CreateSolidColorTexture(RGBA::White).get(), defaultSampler.get());
        material->SetTexture(ShaderSlots::MetallicRoughness, pbrResources.CreateSolidColorTexture(RGBA::White).get(), defaultSampler.get());
        // No occlusion.
        material->SetTexture(ShaderSlots::Occlusion, pbrResources.CreateSolidColorTexture(RGBA::White).get(), defaultSampler.get());
        // Flat normal.
        material->SetTexture(ShaderSlots::Normal, pbrResources.CreateSolidColorTexture(RGBA::FlatNormal).get(), defaultSampler.get());
        material->SetTexture(ShaderSlots::Emissive, pbrResources.CreateSolidColorTexture(RGBA::White).get(), defaultSampler.get());

        return material;
    }

    void Material::SetTexture(ShaderSlots::PSMaterial slot,
                              _In_ ID3D11ShaderResourceView* textureView,
                              _In_opt_ ID3D11SamplerState* sampler) {
        m_textures[slot].copy_from(textureView);

        if (sampler) {
            m_samplers[slot].copy_from(sampler);
        }
    }

    void Material::SetDoubleSided(bool doubleSided) {
        m_doubleSided = doubleSided;
    }

    void Material::SetWireframe(bool wireframeMode) {
        m_wireframe = wireframeMode;
    }

    void Material::SetAlphaBlended(bool alphaBlended) {
        m_alphaBlended = alphaBlended;
    }

    void Material::Bind(_In_ ID3D11DeviceContext* context, const Resources& pbrResources) const {
        // If the parameters of the constant buffer have changed, update the constant buffer.
        if (m_parametersChanged) {
            m_parametersChanged = false;
            context->UpdateSubresource(m_constantBuffer.get(), 0, nullptr, &m_parameters, 0, 0);
        }

        pbrResources.SetBlendState(context, m_alphaBlended);
        pbrResources.SetDepthStencilState(context, m_alphaBlended);
        pbrResources.SetRasterizerState(context, m_doubleSided, m_wireframe);

        ID3D11Buffer* psConstantBuffers[] = {m_constantBuffer.get()};
        context->PSSetConstantBuffers(Pbr::ShaderSlots::ConstantBuffers::Material, 1, psConstantBuffers);

        static_assert(Pbr::ShaderSlots::BaseColor == 0, "BaseColor must be the first slot");

        std::array<ID3D11ShaderResourceView*, TextureCount> textures;
        std::transform(m_textures.begin(), m_textures.end(), textures.begin(), [](const auto& texture) { return texture.get(); });
        context->PSSetShaderResources(Pbr::ShaderSlots::BaseColor, (UINT)textures.size(), textures.data());

        std::array<ID3D11SamplerState*, TextureCount> samplers;
        std::transform(m_samplers.begin(), m_samplers.end(), samplers.begin(), [](const auto& sampler) { return sampler.get(); });
        context->PSSetSamplers(Pbr::ShaderSlots::BaseColor, (UINT)samplers.size(), samplers.data());
    }

    Material::ConstantBufferData& Material::Parameters() {
        m_parametersChanged = true;
        return m_parameters;
    }

    const Material::ConstantBufferData& Material::Parameters() const {
        return m_parameters;
    }
} // namespace Pbr
