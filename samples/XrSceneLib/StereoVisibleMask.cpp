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

#include "SceneContext.h"
#include "VisibilityMask.h"
#include <StereoVisibleMaskVertexShader.h>
#include <SampleShared/Trace.h>

using namespace DirectX;

namespace {
    static constexpr uint32_t NumViews = 2;
    static constexpr XrViewConfigurationType ViewConfigType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    static constexpr XrVisibilityMaskTypeKHR VisibilityMaskType = XR_VISIBILITY_MASK_TYPE_VISIBLE_TRIANGLE_MESH_KHR;
    static constexpr D3D11_INPUT_ELEMENT_DESC VertexDesc = {
        "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0};

    struct ProjTransformConstantData {
        XMFLOAT4X4 ProjTransform;
    };

    struct StereoVisibleMask : public IVisibilityMask {
        StereoVisibleMask(SceneContext* sceneContext)
            : m_sceneContext(sceneContext) {
            CHECK_HRCMD(m_sceneContext->Device->CreateVertexShader(
                g_StereoVisibleMaskVertexShader, sizeof(g_StereoVisibleMaskVertexShader), nullptr, m_meshVertexShader.put()));

            CHECK_HRCMD(m_sceneContext->Device->CreateInputLayout(
                &VertexDesc, 1, g_StereoVisibleMaskVertexShader, sizeof(g_StereoVisibleMaskVertexShader), m_meshInputLayout.put()));

            const CD3D11_BUFFER_DESC cbDesc{(UINT)(sizeof(ProjTransformConstantData)), D3D11_BIND_CONSTANT_BUFFER};
            CHECK_HRCMD(m_sceneContext->Device->CreateBuffer(&cbDesc, nullptr, m_projTransformConstantBuffer.put()));

            CD3D11_RASTERIZER_DESC rsStateDesc(CD3D11_DEFAULT{});
            rsStateDesc.FrontCounterClockwise = TRUE;
            CHECK_HRCMD(m_sceneContext->Device->CreateRasterizerState(&rsStateDesc, m_ccwWindingRSState.put()));

            for (uint32_t i = 0; i < NumViews; i++) {
                UpdateVisibleMeshObject(i);
            }
        }

        // Prior calling render, the caller need to ensure viewport and depth stencil state are correctly set and bind; stencil buffer is
        // cleared
        bool RenderMask(uint32_t viewIndex, FXMMATRIX viewToProj) const override {
            assert(viewIndex < NumViews);

            if (!m_isMaskValid) {
                return false;
            }

            ProjTransformConstantData data{};
            XMStoreFloat4x4(&data.ProjTransform, XMMatrixTranspose(viewToProj));
            m_sceneContext->DeviceContext->UpdateSubresource(m_projTransformConstantBuffer.get(), 0, nullptr, &data, 0, 0);
            ID3D11Buffer* projConstantBuffers[] = {m_projTransformConstantBuffer.get()};

            m_sceneContext->DeviceContext->VSSetShader(m_meshVertexShader.get(), nullptr, 0);
            m_sceneContext->DeviceContext->VSSetConstantBuffers(0, 1, projConstantBuffers);

            m_sceneContext->DeviceContext->PSSetShader(nullptr, nullptr, 0);
            m_sceneContext->DeviceContext->GSSetShader(nullptr, nullptr, 0);

            ID3D11RasterizerState* prevRSState;
            m_sceneContext->DeviceContext->RSGetState(&prevRSState);
            m_sceneContext->DeviceContext->RSSetState(m_ccwWindingRSState.get());

            ID3D11Buffer* iaVertBuffers[] = {m_meshVertexBuffer[viewIndex].get()};
            const UINT iaStrides[] = {sizeof(XrVector2f)};
            const UINT iaOffsets[] = {0};

            m_sceneContext->DeviceContext->IASetVertexBuffers(0, 1, iaVertBuffers, iaStrides, iaOffsets);
            m_sceneContext->DeviceContext->IASetIndexBuffer(m_meshIndexBuffer[viewIndex].get(), DXGI_FORMAT_R32_UINT, 0);
            m_sceneContext->DeviceContext->IASetInputLayout(m_meshInputLayout.get());
            m_sceneContext->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_sceneContext->DeviceContext->DrawIndexed(static_cast<uint32_t>(m_visibleMeshIndices[viewIndex].size()), 0, 0);

            m_sceneContext->DeviceContext->RSSetState(prevRSState);

            return true;
        }

        void NotifyMaskChanged(uint32_t viewIndex) override {
            UpdateVisibleMeshObject(viewIndex);
        }

    private:
        void UpdateVisibleMeshObject(uint32_t viewIndex) {
            assert(viewIndex < NumViews);

            m_visibleMeshVertices[viewIndex].clear();
            m_visibleMeshIndices[viewIndex].clear();

            XrVisibilityMaskKHR visibilityMask{XR_TYPE_VISIBILITY_MASK_KHR};

            CHECK_XRCMD(m_sceneContext->Extensions.xrGetVisibilityMaskKHR(
                m_sceneContext->Session, ViewConfigType, viewIndex, VisibilityMaskType, &visibilityMask));

            m_isMaskValid = visibilityMask.vertexCountOutput != 0;
            if (!m_isMaskValid) {
                return;
            }

            // allocate memory for vertices
            m_visibleMeshVertices[viewIndex].resize(visibilityMask.vertexCountOutput);
            visibilityMask.vertices = m_visibleMeshVertices[viewIndex].data();

            m_visibleMeshIndices[viewIndex].resize(visibilityMask.vertexCountOutput);
            visibilityMask.indices = m_visibleMeshIndices[viewIndex].data();

            visibilityMask.vertexCapacityInput = visibilityMask.vertexCountOutput;
            visibilityMask.indexCapacityInput = visibilityMask.indexCountOutput;

            CHECK_XRCMD(m_sceneContext->Extensions.xrGetVisibilityMaskKHR(
                m_sceneContext->Session, ViewConfigType, viewIndex, VisibilityMaskType, &visibilityMask));

            // Re-create vertex/index buffer
            D3D11_BUFFER_DESC desc{};
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.ByteWidth = (UINT)(sizeof(XrVector2f) * m_visibleMeshVertices[viewIndex].size());
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

            D3D11_SUBRESOURCE_DATA initData{};
            initData.pSysMem = m_visibleMeshVertices[viewIndex].data();
            m_meshVertexBuffer[viewIndex] = nullptr;
            CHECK_HRCMD(m_sceneContext->Device->CreateBuffer(&desc, &initData, m_meshVertexBuffer[viewIndex].put()));

            desc.ByteWidth = (UINT)(sizeof(uint32_t) * m_visibleMeshIndices[viewIndex].size());
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            initData.pSysMem = m_visibleMeshIndices[viewIndex].data();
            m_meshIndexBuffer[viewIndex] = nullptr;
            CHECK_HRCMD(m_sceneContext->Device->CreateBuffer(&desc, &initData, m_meshIndexBuffer[viewIndex].put()));
        }

        std::vector<XrVector2f> m_visibleMeshVertices[NumViews];
        std::vector<uint32_t> m_visibleMeshIndices[NumViews];

        winrt::com_ptr<ID3D11InputLayout> m_meshInputLayout;
        winrt::com_ptr<ID3D11VertexShader> m_meshVertexShader;
        winrt::com_ptr<ID3D11Buffer> m_meshVertexBuffer[NumViews];
        winrt::com_ptr<ID3D11Buffer> m_meshIndexBuffer[NumViews];
        winrt::com_ptr<ID3D11Buffer> m_projTransformConstantBuffer;
        winrt::com_ptr<ID3D11RasterizerState> m_ccwWindingRSState;

        const SceneContext* m_sceneContext;

        bool m_isMaskValid{false};
    };

} // namespace

std::unique_ptr<IVisibilityMask> CreateStereoVisibilityMask(SceneContext* sceneContext) {
    return std::make_unique<StereoVisibleMask>(sceneContext);
}
