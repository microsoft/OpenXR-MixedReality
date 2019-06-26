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
#include "App.h"

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <DirectXColors.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3DCompiler.lib")

using namespace xr;
using namespace xr::math;

namespace {

    namespace Geometry {

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

    } // namespace Geometry

    struct ModelConstantBuffer {
        DirectX::XMFLOAT4X4 Model;
    };

    struct ViewProjectionConstantBuffer {
        DirectX::XMFLOAT4X4 ViewProjection;
    };

    // Separate entrypoints for the vertex and pixel shader functions.
    constexpr char ShaderHlsl[] = R"_(
    struct PSVertex {
        float4 Pos : SV_POSITION;
        float3 Color : COLOR0;
    };
    struct Vertex {
        float3 Pos : POSITION;
        float3 Color : COLOR0;
    };
    cbuffer ModelConstantBuffer : register(b0) {
        float4x4 Model;
    };
    cbuffer ViewProjectionConstantBuffer : register(b1) {
        float4x4 ViewProjection;
    };

    PSVertex MainVS(Vertex input) {
       PSVertex output;
       output.Pos = mul(mul(float4(input.Pos, 1), Model), ViewProjection);
       output.Color = input.Color;
       return output;
    }

    float4 MainPS(PSVertex input) : SV_TARGET {
        return float4(input.Color, 1);
    }
    )_";

    ComPtr<ID3DBlob> CompileShader(const char* hlsl, const char* entrypoint, const char* shaderTarget) {
        ComPtr<ID3DBlob> compiled;
        ComPtr<ID3DBlob> errMsgs;
        DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;

#ifdef _DEBUG
        flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
        flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

        HRESULT hr = D3DCompile(hlsl,
                                strlen(hlsl),
                                nullptr,
                                nullptr,
                                nullptr,
                                entrypoint,
                                shaderTarget,
                                flags,
                                0,
                                compiled.GetAddressOf(),
                                errMsgs.GetAddressOf());
        if (FAILED(hr)) {
            std::string errMsg((const char*)errMsgs->GetBufferPointer(), errMsgs->GetBufferSize());
            DEBUG_PRINT("D3DCompile failed %X: %s", hr, errMsg.c_str());
            CHECK_HRESULT(hr, "D3DCompile failed");
        }

        return compiled;
    }

    ComPtr<IDXGIAdapter1> GetAdapter(LUID adapterId) {
        // Create the DXGI factory.
        ComPtr<IDXGIFactory1> dxgiFactory;
        CHECK_HRCMD(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(dxgiFactory.ReleaseAndGetAddressOf())));

        for (UINT adapterIndex = 0;; adapterIndex++) {
            // EnumAdapters1 will fail with DXGI_ERROR_NOT_FOUND when there are no more adapters to enumerate.
            ComPtr<IDXGIAdapter1> dxgiAdapter;
            CHECK_HRCMD(dxgiFactory->EnumAdapters1(adapterIndex, dxgiAdapter.ReleaseAndGetAddressOf()));

            DXGI_ADAPTER_DESC1 adapterDesc;
            CHECK_HRCMD(dxgiAdapter->GetDesc1(&adapterDesc));
            if (memcmp(&adapterDesc.AdapterLuid, &adapterId, sizeof(adapterId)) == 0) {
                DEBUG_PRINT("Using graphics adapter %ws", adapterDesc.Description);
                return dxgiAdapter;
            }
        }
    }

    void InitializeD3D11DeviceForAdapter(IDXGIAdapter1* adapter,
                                         const std::vector<D3D_FEATURE_LEVEL>& featureLevels,
                                         ID3D11Device** device,
                                         ID3D11DeviceContext** deviceContext) {
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        // Create the Direct3D 11 API device object and a corresponding context.
        const D3D_DRIVER_TYPE driverType = adapter == nullptr ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN;
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
            // If the initialization fails, fall back to the WARP device.
            // For more information on WARP, see: http://go.microsoft.com/fwlink/?LinkId=286690
            CHECK_HRCMD(D3D11CreateDevice(nullptr,
                                          D3D_DRIVER_TYPE_WARP,
                                          0,
                                          creationFlags,
                                          featureLevels.data(),
                                          (UINT)featureLevels.size(),
                                          D3D11_SDK_VERSION,
                                          device,
                                          nullptr,
                                          deviceContext));
        }
    }

    // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
    std::vector<D3D_FEATURE_LEVEL> SelectFeatureLevels(XrGraphicsRequirementsD3D11KHR graphicsRequirements) {
        // Create a list of feature levels which are both supported by the OpenXR runtime and this application.
        std::vector<D3D_FEATURE_LEVEL> featureLevels = {D3D_FEATURE_LEVEL_12_1,
                                                        D3D_FEATURE_LEVEL_12_0,
                                                        D3D_FEATURE_LEVEL_11_1,
                                                        D3D_FEATURE_LEVEL_11_0,
                                                        D3D_FEATURE_LEVEL_10_1,
                                                        D3D_FEATURE_LEVEL_10_0};
        featureLevels.erase(std::remove_if(featureLevels.begin(),
                                           featureLevels.end(),
                                           [&](D3D_FEATURE_LEVEL fl) { return fl < graphicsRequirements.minFeatureLevel; }),
                            featureLevels.end());
        CHECK_MSG(featureLevels.size() != 0, "Unsupported minimum feature level!");
        return featureLevels;
    }

    struct CubeGraphics : IGraphicsPlugin {
        void InitializeDevice(XrInstance instance, XrSystemId systemId) override {
            CHECK(instance != XR_NULL_HANDLE);
            CHECK(systemId != XR_NULL_SYSTEM_ID);

            // Create the D3D11 device for the adapter associated with the system.
            XrGraphicsRequirementsD3D11KHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
            CHECK_XRCMD(xrGetD3D11GraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));

            const ComPtr<IDXGIAdapter1> adapter = GetAdapter(graphicsRequirements.adapterLuid);
            const std::vector<D3D_FEATURE_LEVEL> featureLevels = SelectFeatureLevels(graphicsRequirements);

            InitializeD3D11DeviceForAdapter(
                adapter.Get(), featureLevels, m_device.ReleaseAndGetAddressOf(), m_deviceContext.ReleaseAndGetAddressOf());

            InitializeD3DResources();

            m_xrGraphicsBindingD3D11.device = m_device.Get();
        }

        const XrGraphicsBindingD3D11KHR* GetGraphicsBinding() const override {
            return &m_xrGraphicsBindingD3D11;
        }

        void InitializeD3DResources() {
            const ComPtr<ID3DBlob> vertexShaderBytes = CompileShader(ShaderHlsl, "MainVS", "vs_5_0");
            CHECK_HRCMD(m_device->CreateVertexShader(vertexShaderBytes->GetBufferPointer(),
                                                     vertexShaderBytes->GetBufferSize(),
                                                     nullptr,
                                                     m_vertexShader.ReleaseAndGetAddressOf()));

            const ComPtr<ID3DBlob> pixelShaderBytes = CompileShader(ShaderHlsl, "MainPS", "ps_5_0");
            CHECK_HRCMD(m_device->CreatePixelShader(
                pixelShaderBytes->GetBufferPointer(), pixelShaderBytes->GetBufferSize(), nullptr, m_pixelShader.ReleaseAndGetAddressOf()));

            const D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            CHECK_HRCMD(m_device->CreateInputLayout(vertexDesc,
                                                    (UINT)std::size(vertexDesc),
                                                    vertexShaderBytes->GetBufferPointer(),
                                                    vertexShaderBytes->GetBufferSize(),
                                                    &m_inputLayout));

            const CD3D11_BUFFER_DESC modelConstantBufferDesc(sizeof(ModelConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&modelConstantBufferDesc, nullptr, m_modelCBuffer.ReleaseAndGetAddressOf()));

            const CD3D11_BUFFER_DESC viewProjectionConstantBufferDesc(sizeof(ViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
            CHECK_HRCMD(
                m_device->CreateBuffer(&viewProjectionConstantBufferDesc, nullptr, m_viewProjectionCBuffer.ReleaseAndGetAddressOf()));

            const D3D11_SUBRESOURCE_DATA vertexBufferData{Geometry::c_cubeVertices};
            const CD3D11_BUFFER_DESC vertexBufferDesc(sizeof(Geometry::c_cubeVertices), D3D11_BIND_VERTEX_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_cubeVertexBuffer.ReleaseAndGetAddressOf()));

            const D3D11_SUBRESOURCE_DATA indexBufferData{Geometry::c_cubeIndices};
            const CD3D11_BUFFER_DESC indexBufferDesc(sizeof(Geometry::c_cubeIndices), D3D11_BIND_INDEX_BUFFER);
            CHECK_HRCMD(m_device->CreateBuffer(&indexBufferDesc, &indexBufferData, m_cubeIndexBuffer.ReleaseAndGetAddressOf()));
        }

        int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const override {
            // List of supported color swapchain formats, in priority order.
            constexpr DXGI_FORMAT SupportedColorSwapchainFormats[] = {
                DXGI_FORMAT_R8G8B8A8_UNORM,
                DXGI_FORMAT_B8G8R8A8_UNORM,
                DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
                DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,
            };

            auto swapchainFormatIt = std::find_first_of(std::begin(SupportedColorSwapchainFormats),
                                                        std::end(SupportedColorSwapchainFormats),
                                                        runtimeFormats.begin(),
                                                        runtimeFormats.end());
            if (swapchainFormatIt == std::end(SupportedColorSwapchainFormats)) {
                THROW("No runtime swapchain format supported for color swapchain");
            }

            return *swapchainFormatIt;
        }

        std::vector<XrSwapchainImageBaseHeader*>
        AllocateSwapchainImageStructs(uint32_t capacity, const XrSwapchainCreateInfo& /*swapchainCreateInfo*/) override {
            // Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
            // Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
            std::vector<XrSwapchainImageD3D11KHR> swapchainImageBuffer(capacity);
            std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
            for (XrSwapchainImageD3D11KHR& image : swapchainImageBuffer) {
                image.type = XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR;
                swapchainImageBase.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
            }

            // Keep the buffer alive by moving it into the list of buffers.
            m_swapchainImageBuffers.push_back(std::move(swapchainImageBuffer));

            return swapchainImageBase;
        }

        ComPtr<ID3D11DepthStencilView> GetDepthStencilView(ID3D11Texture2D* colorTexture) {
            // If a depth-stencil view has already been created for this back-buffer, use it.
            auto depthBufferIt = m_colorToDepthMap.find(colorTexture);
            if (depthBufferIt != m_colorToDepthMap.end()) {
                return depthBufferIt->second;
            }

            // This back-buffer has no cooresponding depth-stencil texture, so create one with matching dimensions.
            D3D11_TEXTURE2D_DESC colorDesc;
            colorTexture->GetDesc(&colorDesc);

            D3D11_TEXTURE2D_DESC depthDesc{};
            depthDesc.Width = colorDesc.Width;
            depthDesc.Height = colorDesc.Height;
            depthDesc.ArraySize = colorDesc.ArraySize;
            depthDesc.MipLevels = 1;
            depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
            depthDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
            depthDesc.SampleDesc.Count = 1;
            ComPtr<ID3D11Texture2D> depthTexture;
            CHECK_HRCMD(m_device->CreateTexture2D(&depthDesc, nullptr, depthTexture.ReleaseAndGetAddressOf()));

            // Create and cache the depth stencil view.
            ComPtr<ID3D11DepthStencilView> depthStencilView;
            CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D32_FLOAT);
            CHECK_HRCMD(m_device->CreateDepthStencilView(depthTexture.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf()));
            depthBufferIt = m_colorToDepthMap.insert(std::make_pair(colorTexture, depthStencilView)).first;

            return depthStencilView;
        }

        void RenderView(const XrCompositionLayerProjectionView& layerView,
                        const XrSwapchainImageBaseHeader* swapchainImage,
                        const XrEnvironmentBlendMode environmentBlendMode,
                        int64_t colorSwapchainFormat,
                        const std::vector<Cube>& cubes) override {
            CHECK(layerView.subImage.imageArrayIndex == 0); // Texture arrays not supported.

            ID3D11Texture2D* const colorTexture = reinterpret_cast<const XrSwapchainImageD3D11KHR*>(swapchainImage)->texture;

            CD3D11_VIEWPORT viewport((float)layerView.subImage.imageRect.offset.x,
                                     (float)layerView.subImage.imageRect.offset.y,
                                     (float)layerView.subImage.imageRect.extent.width,
                                     (float)layerView.subImage.imageRect.extent.height);
            m_deviceContext->RSSetViewports(1, &viewport);

            // Create RenderTargetView with original swapchain format (swapchain is typeless).
            ComPtr<ID3D11RenderTargetView> renderTargetView;
            const CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2D, (DXGI_FORMAT)colorSwapchainFormat);
            CHECK_HRCMD(m_device->CreateRenderTargetView(colorTexture, &renderTargetViewDesc, renderTargetView.ReleaseAndGetAddressOf()));

            const ComPtr<ID3D11DepthStencilView> depthStencilView = GetDepthStencilView(colorTexture);

            // Clear swapchain and depth buffer. NOTE: This will clear the entire render target view, not just the specified view.
            m_deviceContext->ClearRenderTargetView(renderTargetView.Get(),
                                                   environmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE ? DirectX::Colors::DarkSlateGray
                                                                                                            : DirectX::Colors::Transparent);
            m_deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

            ID3D11RenderTargetView* renderTargets[] = {renderTargetView.Get()};
            m_deviceContext->OMSetRenderTargets((UINT)std::size(renderTargets), renderTargets, depthStencilView.Get());

            const DirectX::XMMATRIX spaceToView = XMMatrixInverse(nullptr, LoadXrPose(layerView.pose));
            DirectX::XMMATRIX projectionMatrix = ComposeProjectionMatrix(layerView.fov, {0.05f, 100.0f});

            // Set shaders and constant buffers.
            ViewProjectionConstantBuffer viewProjection;
            XMStoreFloat4x4(&viewProjection.ViewProjection, XMMatrixTranspose(spaceToView * projectionMatrix));
            m_deviceContext->UpdateSubresource(m_viewProjectionCBuffer.Get(), 0, nullptr, &viewProjection, 0, 0);

            ID3D11Buffer* const constantBuffers[] = {m_modelCBuffer.Get(), m_viewProjectionCBuffer.Get()};
            m_deviceContext->VSSetConstantBuffers(0, (UINT)std::size(constantBuffers), constantBuffers);
            m_deviceContext->VSSetShader(m_vertexShader.Get(), nullptr, 0);
            m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

            // Set cube primitive data.
            const UINT strides[] = {sizeof(Geometry::Vertex)};
            const UINT offsets[] = {0};
            ID3D11Buffer* vertexBuffers[] = {m_cubeVertexBuffer.Get()};
            m_deviceContext->IASetVertexBuffers(0, (UINT)std::size(vertexBuffers), vertexBuffers, strides, offsets);
            m_deviceContext->IASetIndexBuffer(m_cubeIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
            m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_deviceContext->IASetInputLayout(m_inputLayout.Get());

            // Render each cube
            for (const Cube& cube : cubes) {
                // Compute and update the model transform.
                ModelConstantBuffer model;
                XMStoreFloat4x4(&model.Model,
                    DirectX::XMMatrixTranspose(DirectX::XMMatrixScaling(cube.Scale.x, cube.Scale.y, cube.Scale.z) * LoadXrPose(cube.Pose)));
                m_deviceContext->UpdateSubresource(m_modelCBuffer.Get(), 0, nullptr, &model, 0, 0);

                // Draw the cube.
                m_deviceContext->DrawIndexed((UINT)std::size(Geometry::c_cubeIndices), 0, 0);
            }
        }

    private:
        ComPtr<ID3D11Device> m_device;
        ComPtr<ID3D11DeviceContext> m_deviceContext;
        XrGraphicsBindingD3D11KHR m_xrGraphicsBindingD3D11{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
        std::list<std::vector<XrSwapchainImageD3D11KHR>> m_swapchainImageBuffers;
        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader> m_pixelShader;
        ComPtr<ID3D11InputLayout> m_inputLayout;
        ComPtr<ID3D11Buffer> m_modelCBuffer;
        ComPtr<ID3D11Buffer> m_viewProjectionCBuffer;
        ComPtr<ID3D11Buffer> m_cubeVertexBuffer;
        ComPtr<ID3D11Buffer> m_cubeIndexBuffer;

        // Map color buffer to associated depth buffer. This map is populated on demand.
        std::map<ID3D11Texture2D*, ComPtr<ID3D11DepthStencilView>> m_colorToDepthMap;
    };
} // namespace

namespace xr {
    std::unique_ptr<IGraphicsPlugin> CreateCubeGraphics() {
        return std::make_unique<CubeGraphics>();
    }
} // namespace xr
