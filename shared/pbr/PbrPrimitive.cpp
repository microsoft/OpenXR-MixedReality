////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#include "pch.h"
#include "PbrCommon.h"
#include "PbrResources.h"
#include "PbrPrimitive.h"

using namespace DirectX;

namespace {
    UINT GetPbrVertexByteSize(size_t size) {
        return (UINT)(sizeof(decltype(Pbr::PrimitiveBuilder::Vertices)::value_type) * size);
    }

    winrt::com_ptr<ID3D11Buffer> CreateVertexBuffer(_In_ ID3D11Device* device,
                                                    const Pbr::PrimitiveBuilder& primitiveBuilder,
                                                    bool updatableBuffers) {
        // Create Vertex Buffer
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = GetPbrVertexByteSize(primitiveBuilder.Vertices.size());
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        if (updatableBuffers) {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
        }

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = primitiveBuilder.Vertices.data();

        winrt::com_ptr<ID3D11Buffer> vertexBuffer;
        Pbr::Internal::ThrowIfFailed(device->CreateBuffer(&desc, &initData, vertexBuffer.put()));
        return vertexBuffer;
    }

    winrt::com_ptr<ID3D11Buffer> CreateIndexBuffer(_In_ ID3D11Device* device,
                                                   const Pbr::PrimitiveBuilder& primitiveBuilder,
                                                   bool updatableBuffers) {
        // Create Index Buffer
        D3D11_BUFFER_DESC desc{};
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.ByteWidth = (UINT)(sizeof(decltype(primitiveBuilder.Indices)::value_type) * primitiveBuilder.Indices.size());
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        if (updatableBuffers) {
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;
        }

        D3D11_SUBRESOURCE_DATA initData{};
        initData.pSysMem = primitiveBuilder.Indices.data();

        winrt::com_ptr<ID3D11Buffer> indexBuffer;
        Pbr::Internal::ThrowIfFailed(device->CreateBuffer(&desc, &initData, indexBuffer.put()));
        return indexBuffer;
    }
} // namespace

namespace Pbr {
    Primitive::Primitive(UINT indexCount,
                         winrt::com_ptr<ID3D11Buffer> indexBuffer,
                         winrt::com_ptr<ID3D11Buffer> vertexBuffer,
                         std::shared_ptr<Material> material)
        : m_indexCount(indexCount)
        , m_indexBuffer(std::move(indexBuffer))
        , m_vertexBuffer(std::move(vertexBuffer))
        , m_material(std::move(material)) {
    }

    Primitive::Primitive(Pbr::Resources const& pbrResources,
                         const Pbr::PrimitiveBuilder& primitiveBuilder,
                         std::shared_ptr<Pbr::Material> material,
                         bool updatableBuffers)
        : Primitive((UINT)primitiveBuilder.Indices.size(),
                    CreateIndexBuffer(pbrResources.GetDevice().get(), primitiveBuilder, updatableBuffers),
                    CreateVertexBuffer(pbrResources.GetDevice().get(), primitiveBuilder, updatableBuffers),
                    std::move(material)) {
    }

    Primitive Primitive::Clone(Pbr::Resources const& pbrResources) const {
        return Primitive(m_indexCount, m_indexBuffer, m_vertexBuffer, m_material->Clone(pbrResources));
    }

    void Primitive::UpdateBuffers(_In_ ID3D11Device* device,
                                  _In_ ID3D11DeviceContext* context,
                                  const Pbr::PrimitiveBuilder& primitiveBuilder) {
        // Update vertex buffer.
        {
            D3D11_BUFFER_DESC vertDesc;
            m_vertexBuffer->GetDesc(&vertDesc);

            UINT requiredSize = GetPbrVertexByteSize(primitiveBuilder.Vertices.size());
            if (vertDesc.ByteWidth >= requiredSize) {
                context->UpdateSubresource(m_vertexBuffer.get(), 0, nullptr, primitiveBuilder.Vertices.data(), requiredSize, requiredSize);
            } else {
                m_vertexBuffer = CreateVertexBuffer(device, primitiveBuilder, true);
            }
        }

        // Update index buffer.
        {
            D3D11_BUFFER_DESC idxDesc;
            m_indexBuffer->GetDesc(&idxDesc);

            UINT requiredSize = (UINT)(primitiveBuilder.Indices.size() * sizeof(decltype(primitiveBuilder.Indices)::value_type));
            if (idxDesc.ByteWidth >= requiredSize) {
                context->UpdateSubresource(m_indexBuffer.get(), 0, nullptr, primitiveBuilder.Indices.data(), requiredSize, requiredSize);
            } else {
                m_indexBuffer = CreateIndexBuffer(device, primitiveBuilder, true);
            }

            m_indexCount = (UINT)primitiveBuilder.Indices.size();
        }
    }

    void Primitive::Render(_In_ ID3D11DeviceContext* context) const {
        const UINT stride = sizeof(Pbr::Vertex);
        const UINT offset = 0;
        ID3D11Buffer* const vertexBuffers[] = {m_vertexBuffer.get()};
        context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
        context->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R32_UINT, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        context->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
    }
} // namespace Pbr
