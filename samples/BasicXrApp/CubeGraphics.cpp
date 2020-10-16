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
#include "OpenXrProgram.h"
#include "DxUtility.h"

namespace {
    namespace CubeShader {
        struct Vertex {
            XrVector3f Position;
            XrVector3f Color;
        };

        constexpr XrVector3f Red{1, 0, 0};
        constexpr XrVector3f DarkRed{0.25f, 0, 0};
        constexpr XrVector3f Green{0, 1, 0};
        constexpr XrVector3f DarkGreen{0, 0.25f, 0};
        constexpr XrVector3f Blue{0, 0, 1};
        constexpr XrVector3f DarkBlue{0, 0, 0.25f};

        // Vertices for a 1x1x1 meter cube. (Left/Right, Top/Bottom, Front/Back)
        constexpr XrVector3f LBB{-0.5f, -0.5f, -0.5f};
        constexpr XrVector3f LBF{-0.5f, -0.5f, 0.5f};
        constexpr XrVector3f LTB{-0.5f, 0.5f, -0.5f};
        constexpr XrVector3f LTF{-0.5f, 0.5f, 0.5f};
        constexpr XrVector3f RBB{0.5f, -0.5f, -0.5f};
        constexpr XrVector3f RBF{0.5f, -0.5f, 0.5f};
        constexpr XrVector3f RTB{0.5f, 0.5f, -0.5f};
        constexpr XrVector3f RTF{0.5f, 0.5f, 0.5f};

#define CUBE_SIDE(V1, V2, V3, V4, V5, V6, COLOR) {V1, COLOR}, {V2, COLOR}, {V3, COLOR}, {V4, COLOR}, {V5, COLOR}, {V6, COLOR},

        constexpr Vertex c_cubeVertices[] = {
            CUBE_SIDE(LTB, LBF, LBB, LTB, LTF, LBF, DarkRed)   // -X
            CUBE_SIDE(RTB, RBB, RBF, RTB, RBF, RTF, Red)       // +X
            CUBE_SIDE(LBB, LBF, RBF, LBB, RBF, RBB, DarkGreen) // -Y
            CUBE_SIDE(LTB, RTB, RTF, LTB, RTF, LTF, Green)     // +Y
            CUBE_SIDE(LBB, RBB, RTB, LBB, RTB, LTB, DarkBlue)  // -Z
            CUBE_SIDE(LBF, LTF, RTF, LBF, RTF, RBF, Blue)      // +Z
        };

        // Winding order is clockwise. Each side uses a different color.
        constexpr unsigned short c_cubeIndices[] = {
            0,  1,  2,  3,  4,  5,  // -X
            6,  7,  8,  9,  10, 11, // +X
            12, 13, 14, 15, 16, 17, // -Y
            18, 19, 20, 21, 22, 23, // +Y
            24, 25, 26, 27, 28, 29, // -Z
            30, 31, 32, 33, 34, 35, // +Z
        };

        struct ModelConstantBuffer {
            DirectX::XMFLOAT4X4 Model;
        };

        struct ViewProjectionConstantBuffer {
            DirectX::XMFLOAT4X4 ViewProjection[2];
        };

        constexpr uint32_t MaxViewInstance = 2;

        // Separate entrypoints for the vertex and pixel shader functions.
        constexpr char ShaderHlsl[] = R"_(
            struct VSOutput {
                float4 Pos : SV_POSITION;
                float3 Color : COLOR0;
                uint viewId : SV_RenderTargetArrayIndex;
            };
            struct VSInput {
                float3 Pos : POSITION;
                float3 Color : COLOR0;
                uint instId : SV_InstanceID;
            };
            cbuffer ModelConstantBuffer : register(b0) {
                float4x4 Model;
            };
            cbuffer ViewProjectionConstantBuffer : register(b1) {
                float4x4 ViewProjection[2];
            };

            VSOutput MainVS(VSInput input) {
                VSOutput output;
                output.Pos = mul(mul(float4(input.Pos, 1), Model), ViewProjection[input.instId]);
                output.Color = input.Color;
                output.viewId = input.instId;
                return output;
            }

            float4 MainPS(VSOutput input) : SV_TARGET {
                return float4(input.Color, 1);
            }
            )_";

    } // namespace CubeShader

    struct CubeGraphics : sample::IGraphicsPluginD3D11 {
        ID3D11Device* InitializeDevice(LUID adapterLuid, const std::vector<D3D_FEATURE_LEVEL>& featureLevels) override {
            const winrt::com_ptr<IDXGIAdapter1> adapter = sample::dx::GetAdapter(adapterLuid);

            sample::dx::CreateD3D11DeviceAndContext(adapter.get(), featureLevels, m_device.put(), m_deviceContext.put());

            InitializeD3DResources();

            return m_device.get();
        }

        void InitializeD3DResources() {
            const winrt::com_ptr<ID3DBlob> vertexShaderBytes = sample::dx::CompileShader(CubeShader::ShaderHlsl, "MainVS", "vs_5_0");
            CHECK_HRCMD(m_device->CreateVertexShader(
                vertexShaderBytes->GetBufferPointer(), vertexShaderBytes->GetBufferSize(), nullptr, m_vertexShader.put()));

            const winrt::com_ptr<ID3DBlob> pixelShaderBytes = sample::dx::CompileShader(CubeShader::ShaderHlsl, "MainPS", "ps_5_0");
            CHECK_HRCMD(m_device->CreatePixelShader(
                pixelShaderBytes->GetBufferPointer(), pixelShaderBytes->GetBufferSize(), nullptr, m_pixelShader.put()));

            const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            CHECK_HRCMD(m_device->CreateInputLayout(vertexDesc,
                                                    (UINT)std::size(vertexDesc),
                                                    vertexShaderBytes->GetBufferPointer(),
                                                    vertexShaderBytes->GetBufferSize(),
                                                    m_inputLayout.put()));

            const CD3D11_BUFFER_DESC modelConstantBufferDesc(sizeof(CubeShader::ModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&modelConstantBufferDesc, nullptr, m_modelCBuffer.put()));

            const CD3D11_BUFFER_DESC viewProjectionConstantBufferDesc(sizeof(CubeShader::ViewProjectionConstantBuffer),
                                                                      D3D11_BIND_CONSTANT_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&viewProjectionConstantBufferDesc, nullptr, m_viewProjectionCBuffer.put()));

            const D3D11_SUBRESOURCE_DATA vertexBufferData{CubeShader::c_cubeVertices};
            const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(CubeShader::c_cubeVertices), D3D11_BIND_VERTEX_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_cubeVertexBuffer.put()));

            const D3D11_SUBRESOURCE_DATA indexBufferData{CubeShader::c_cubeIndices};
            const CD3D11_BUFFER_DESC indexBufferDesc(sizeof(CubeShader::c_cubeIndices), D3D11_BIND_INDEX_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&indexBufferDesc, &indexBufferData, m_cubeIndexBuffer.put()));

            D3D11_FEATURE_DATA_D3D11_OPTIONS3 options;
            m_device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options, sizeof(options));
            CHECK_MSG(options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer,
                      "This sample requires VPRT support. Adjust sample shaders on GPU without VRPT.");

            CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(CD3D11_DEFAULT{});
            depthStencilDesc.DepthEnable = true;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
            CHECK_HRCMD(m_device->CreateDepthStencilState(&depthStencilDesc, m_reversedZDepthNoStencilTest.put()));
        }

        const std::vector<DXGI_FORMAT>& SupportedColorFormats() const override {
            const static std::vector<DXGI_FORMAT> SupportedColorFormats = {
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            };
            return SupportedColorFormats;
        }

        const std::vector<DXGI_FORMAT>& SupportedDepthFormats() const override {
            const static std::vector<DXGI_FORMAT> SupportedDepthFormats = {
                DXGI_FORMAT_D32_FLOAT,
                DXGI_FORMAT_D16_UNORM,
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
            };
            return SupportedDepthFormats;
        }

        void RenderView(const XrRect2Di& imageRect,
                        const float renderTargetClearColor[4],
                        const std::vector<xr::math::ViewProjection>& viewProjections,
                        DXGI_FORMAT colorSwapchainFormat,
                        ID3D11Texture2D* colorTexture,
                        DXGI_FORMAT depthSwapchainFormat,
                        ID3D11Texture2D* depthTexture,
                        const std::vector<const sample::Cube*>& cubes) override {
            const uint32_t viewInstanceCount = (uint32_t)viewProjections.size();
            CHECK_MSG(viewInstanceCount <= CubeShader::MaxViewInstance,
                      "Sample shader supports 2 or fewer view instances. Adjust shader to accommodate more.")

            CD3D11_VIEWPORT viewport(
                (float)imageRect.offset.x, (float)imageRect.offset.y, (float)imageRect.extent.width, (float)imageRect.extent.height);
            m_deviceContext->RSSetViewports(1, &viewport);

            // Create RenderTargetView with the original swapchain format (swapchain image is typeless).
            winrt::com_ptr<ID3D11RenderTargetView> renderTargetView;
            const CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DARRAY, colorSwapchainFormat);
            CHECK_HRCMD(m_device->CreateRenderTargetView(colorTexture, &renderTargetViewDesc, renderTargetView.put()));

            // Create a DepthStencilView with the original swapchain format (swapchain image is typeless)
            winrt::com_ptr<ID3D11DepthStencilView> depthStencilView;
            CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DARRAY, depthSwapchainFormat);
            CHECK_HRCMD(m_device->CreateDepthStencilView(depthTexture, &depthStencilViewDesc, depthStencilView.put()));

            const bool reversedZ = viewProjections[0].NearFar.Near > viewProjections[0].NearFar.Far;
            const float depthClearValue = reversedZ ? 0.f : 1.f;

            // Clear swapchain and depth buffer. NOTE: This will clear the entire render target view, not just the specified view.
            m_deviceContext->ClearRenderTargetView(renderTargetView.get(), renderTargetClearColor);
            m_deviceContext->ClearDepthStencilView(depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, 0);
            m_deviceContext->OMSetDepthStencilState(reversedZ ? m_reversedZDepthNoStencilTest.get() : nullptr, 0);

            ID3D11RenderTargetView* renderTargets[] = {renderTargetView.get()};
            m_deviceContext->OMSetRenderTargets((UINT)std::size(renderTargets), renderTargets, depthStencilView.get());

            ID3D11Buffer* const constantBuffers[] = {m_modelCBuffer.get(), m_viewProjectionCBuffer.get()};
            m_deviceContext->VSSetConstantBuffers(0, (UINT)std::size(constantBuffers), constantBuffers);
            m_deviceContext->VSSetShader(m_vertexShader.get(), nullptr, 0);
            m_deviceContext->PSSetShader(m_pixelShader.get(), nullptr, 0);

            CubeShader::ViewProjectionConstantBuffer viewProjectionCBufferData{};

            for (uint32_t k = 0; k < viewInstanceCount; k++) {
                const DirectX::XMMATRIX spaceToView = xr::math::LoadInvertedXrPose(viewProjections[k].Pose);
                const DirectX::XMMATRIX projectionMatrix = ComposeProjectionMatrix(viewProjections[k].Fov, viewProjections[k].NearFar);

                // Set view projection matrix for each view, transpose for shader usage.
                DirectX::XMStoreFloat4x4(&viewProjectionCBufferData.ViewProjection[k],
                                         DirectX::XMMatrixTranspose(spaceToView * projectionMatrix));
            }
            m_deviceContext->UpdateSubresource(m_viewProjectionCBuffer.get(), 0, nullptr, &viewProjectionCBufferData, 0, 0);

            // Set cube primitive data.
            const UINT strides[] = {sizeof(CubeShader::Vertex)};
            const UINT offsets[] = {0};
            ID3D11Buffer* vertexBuffers[] = {m_cubeVertexBuffer.get()};
            m_deviceContext->IASetVertexBuffers(0, (UINT)std::size(vertexBuffers), vertexBuffers, strides, offsets);
            m_deviceContext->IASetIndexBuffer(m_cubeIndexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
            m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_deviceContext->IASetInputLayout(m_inputLayout.get());

            // Render each cube
            for (const sample::Cube* cube : cubes) {
                // Compute and update the model transform for each cube, transpose for shader usage.
                CubeShader::ModelConstantBuffer model;
                const DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(cube->Scale.x, cube->Scale.y, cube->Scale.z);
                DirectX::XMStoreFloat4x4(&model.Model, DirectX::XMMatrixTranspose(scaleMatrix * xr::math::LoadXrPose(cube->PoseInAppSpace)));
                m_deviceContext->UpdateSubresource(m_modelCBuffer.get(), 0, nullptr, &model, 0, 0);

                // Draw the cube.
                m_deviceContext->DrawIndexedInstanced((UINT)std::size(CubeShader::c_cubeIndices), viewInstanceCount, 0, 0, 0);
            }
        }

    private:
        winrt::com_ptr<ID3D11Device> m_device;
        winrt::com_ptr<ID3D11DeviceContext> m_deviceContext;
        winrt::com_ptr<ID3D11VertexShader> m_vertexShader;
        winrt::com_ptr<ID3D11PixelShader> m_pixelShader;
        winrt::com_ptr<ID3D11InputLayout> m_inputLayout;
        winrt::com_ptr<ID3D11Buffer> m_modelCBuffer;
        winrt::com_ptr<ID3D11Buffer> m_viewProjectionCBuffer;
        winrt::com_ptr<ID3D11Buffer> m_cubeVertexBuffer;
        winrt::com_ptr<ID3D11Buffer> m_cubeIndexBuffer;
        winrt::com_ptr<ID3D11DepthStencilState> m_reversedZDepthNoStencilTest;
    };
} // namespace

namespace sample {
    std::unique_ptr<sample::IGraphicsPluginD3D11> CreateCubeGraphics() {
        return std::make_unique<CubeGraphics>();
    }
} // namespace sample
