////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <vector>
#include <map>
#include <memory>
#include <winrt/base.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include <DirectXMath.h>
#include "PbrCommon.h"

namespace Pbr {
    namespace ShaderSlots {
        enum VSResourceViews {
            Transforms = 0,
        };

        enum PSMaterial { // For both samplers and textures.
            BaseColor = 0,
            MetallicRoughness,
            Normal,
            Occlusion,
            Emissive,
            LastMaterialSlot = Emissive
        };

        enum Pbr { // For both samplers and textures.
            Brdf = LastMaterialSlot + 1
        };

        enum EnvironmentMap { // For both samplers and textures.
            SpecularTexture = Brdf + 1,
            DiffuseTexture = SpecularTexture + 1,
            EnvironmentMapSampler = Brdf + 1
        };

        enum ConstantBuffers {
            Scene,    // Used by VS and PS
            Model,    // PS only
            Material, // PS only
        };
    } // namespace ShaderSlots

    enum class ShadingMode : uint32_t {
        Regular,
        Highlight,
    };

    enum class FillMode : uint32_t {
        Solid,
        Wireframe,
    };

    enum class FrontFaceWindingOrder : uint32_t {
        ClockWise,
        CounterClockWise,
    };

    // Global PBR resources required for rendering a scene.
    struct Resources final {
        explicit Resources(_In_ ID3D11Device* d3dDevice);
        Resources(Resources&&);

        ~Resources();

        // Sets the Bidirectional Reflectance Distribution Function Lookup Table texture, required by the shader to compute surface
        // reflectance from the IBL.
        void SetBrdfLut(_In_ ID3D11ShaderResourceView* brdfLut);

        // Create device-dependent resources.
        void CreateDeviceDependentResources(_In_ ID3D11Device* device);

        // Release device-dependent resources.
        void ReleaseDeviceDependentResources();

        // Get the D3D11Device that the PBR resources are associated with.
        winrt::com_ptr<ID3D11Device> GetDevice() const;

        // Set the directional light.
        void SetLight(DirectX::XMFLOAT3 direction, RGBColor diffuseColor);

        // Calculate the time since the last highlight ripple effect was started.
        using Duration = std::chrono::high_resolution_clock::duration;
        void UpdateAnimationTime(Duration currentTotalTimeElapsed);

        // Start the highlight pulse animation from the given location.
        void StartHighlightAnimation(DirectX::XMFLOAT3 location, Duration currentTotalTimeElapsed);

        // Set the specular and diffuse image-based lighting (IBL) maps. ShaderResourceViews must be TextureCubes.
        void SetEnvironmentMap(_In_ ID3D11ShaderResourceView* specularEnvironmentMap, _In_ ID3D11ShaderResourceView* diffuseEnvironmentMap);

        // Set the current view and projection matrices.
        void XM_CALLCONV SetViewProjection(DirectX::FXMMATRIX view, DirectX::CXMMATRIX projection);

        // Many 1x1 pixel colored textures are used in the PBR system. This is used to create textures backed by a cache to reduce the
        // number of textures created.
        winrt::com_ptr<ID3D11ShaderResourceView> CreateSolidColorTexture(RGBAColor color) const;

        // Bind the the PBR resources to the current context.
        void Bind(_In_ ID3D11DeviceContext* context) const;

        // Set and update the model to world constant buffer value.
        void XM_CALLCONV SetModelToWorld(DirectX::FXMMATRIX modelToWorld, _In_ ID3D11DeviceContext* context) const;

        // Set or get the shading and fill modes.
        void SetShadingMode(ShadingMode mode);
        ShadingMode GetShadingMode() const;
        void SetFillMode(FillMode mode);
        FillMode GetFillMode() const;
        void SetFrontFaceWindingOrder(FrontFaceWindingOrder windingOrder);
        FrontFaceWindingOrder GetFrontFaceWindingOrder() const;

        void SetDepthFuncReversed(bool reverseZ);

    private:
        void SetBlendState(_In_ ID3D11DeviceContext* context, bool enabled) const;
        void SetRasterizerState(_In_ ID3D11DeviceContext* context, bool doubleSided, bool wireframe) const;
        void SetDepthStencilState(_In_ ID3D11DeviceContext* context, bool disableDepthWrite) const;

        friend struct Material;

        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };
} // namespace Pbr
