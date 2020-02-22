////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

#include <vector>
#include <winrt/base.h>
#include <d3d11.h>
#include <d3d11_2.h>
#include "PbrMaterial.h"

namespace Pbr {
    // A primitive holds a vertex buffer, index buffer, and a pointer to a PBR material.
    struct Primitive final {
        using Collection = std::vector<Primitive>;

        Primitive() = delete;
        Primitive(UINT indexCount,
                  winrt::com_ptr<ID3D11Buffer> indexBuffer,
                  winrt::com_ptr<ID3D11Buffer> vertexBuffer,
                  std::shared_ptr<Material> material);
        Primitive(Pbr::Resources const& pbrResources,
                  const Pbr::PrimitiveBuilder& primitiveBuilder,
                  std::shared_ptr<Material> material,
                  bool updatableBuffers = false);

        void UpdateBuffers(_In_ ID3D11Device* device, _In_ ID3D11DeviceContext* context, const Pbr::PrimitiveBuilder& primitiveBuilder);

        // Get the material for the primitive.
        std::shared_ptr<Material>& GetMaterial() {
            return m_material;
        }
        const std::shared_ptr<Material>& GetMaterial() const {
            return m_material;
        }

    protected:
        friend struct Model;
        void Render(_In_ ID3D11DeviceContext* context) const;
        Primitive Clone(Pbr::Resources const& pbrResources) const;

    private:
        UINT m_indexCount;
        winrt::com_ptr<ID3D11Buffer> m_indexBuffer;
        winrt::com_ptr<ID3D11Buffer> m_vertexBuffer;
        std::shared_ptr<Material> m_material;
    };
} // namespace Pbr
