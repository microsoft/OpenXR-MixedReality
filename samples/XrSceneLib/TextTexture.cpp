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
#include "pbr/PbrMaterial.h"
#include "TextTexture.h"

namespace {
    constexpr DXGI_FORMAT TextFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
}

TextTexture::TextTexture(
    SceneContext* sceneContext, uint32_t width, uint32_t height, const wchar_t* fontName, float fontSize, Pbr::RGBAColor color) {
    const winrt::com_ptr<ID3D11Device> device = sceneContext->Device;
    const winrt::com_ptr<ID3D11DeviceContext> context = sceneContext->DeviceContext;

    D2D1_FACTORY_OPTIONS options{};
    CHECK_HRCMD(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, winrt::guid_of<ID2D1Factory2>(), &options, m_d2dFactory.put_void()));
    CHECK_HRCMD(DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED, winrt::guid_of<IDWriteFactory2>(), reinterpret_cast<IUnknown**>(m_dwriteFactory.put_void())));

    const winrt::com_ptr<IDXGIDevice> dxgiDevice = device.as<IDXGIDevice>();
    CHECK_HRCMD(m_d2dFactory->CreateDevice(dxgiDevice.get(), m_d2dDevice.put()));
    CHECK_HRCMD(m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_d2dContext.put()));

    //
    // Create text format.
    //
    CHECK_HRCMD(m_dwriteFactory->CreateTextFormat(fontName,
                                                  nullptr,
                                                  DWRITE_FONT_WEIGHT_NORMAL,
                                                  DWRITE_FONT_STYLE_NORMAL,
                                                  DWRITE_FONT_STRETCH_NORMAL,
                                                  fontSize,
                                                  L"en-US",
                                                  m_textFormat.put()));
    CHECK_HRCMD(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    CHECK_HRCMD(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

    CHECK_HRCMD(m_d2dFactory->CreateDrawingStateBlock(m_stateBlock.put()));

    //
    // Set up 2D rendering modes.
    //
    const D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                                                             D2D1::PixelFormat(TextFormat, D2D1_ALPHA_MODE_PREMULTIPLIED));

    const auto texDesc = CD3D11_TEXTURE2D_DESC(
        TextFormat, width, height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, 0);
    CHECK_HRCMD(device->CreateTexture2D(&texDesc, nullptr, m_textDWriteTexture.put()));

    winrt::com_ptr<IDXGISurface> dxgiPerfBuffer = m_textDWriteTexture.as<IDXGISurface>();
    CHECK_HRCMD(m_d2dContext->CreateBitmapFromDxgiSurface(dxgiPerfBuffer.get(), &bitmapProperties, m_d2dTargetBitmap.put()));

    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
    m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

    CHECK_HRCMD(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(color.x, color.y, color.z, color.w), m_brush.put()));
}

void TextTexture::Draw(const wchar_t* text) {
    m_d2dContext->SaveDrawingState(m_stateBlock.get());

    const D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    m_d2dContext->BeginDraw();
    m_d2dContext->Clear(0);
    m_d2dContext->DrawText(text,
                           static_cast<UINT32>(wcslen(text)),
                           m_textFormat.get(),
                           D2D1::RectF(0.0f, 0.0f, renderTargetSize.width, renderTargetSize.height),
                           m_brush.get());

    m_d2dContext->EndDraw();

    m_d2dContext->RestoreDrawingState(m_stateBlock.get());
}

ID3D11Texture2D* TextTexture::Texture() const {
    return m_textDWriteTexture.get();
}

std::shared_ptr<Pbr::Material> TextTexture::CreatePbrMaterial(const Pbr::Resources& pbrResources) const {
    auto material = Pbr::Material::CreateFlat(pbrResources, Pbr::RGBA::White);

    winrt::com_ptr<ID3D11ShaderResourceView> textSrv;
    CHECK_HRCMD(pbrResources.GetDevice()->CreateShaderResourceView(Texture(), nullptr, textSrv.put()));

    material->SetTexture(Pbr::ShaderSlots::BaseColor, textSrv.get());
    material->SetAlphaBlended(true);

    return material;
}
