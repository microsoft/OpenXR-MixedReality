// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <pbr/PbrMaterial.h>
#include "TextTexture.h"

namespace {
    constexpr DXGI_FORMAT TextFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
}

engine::TextTexture::TextTexture(Context& context, TextTextureInfo textInfo)
    : m_textInfo(std::move(textInfo)) {
    const winrt::com_ptr<ID3D11Device> device = context.Device;
    const winrt::com_ptr<ID3D11DeviceContext> deviceContext = context.DeviceContext;

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
    CHECK_HRCMD(m_dwriteFactory->CreateTextFormat(m_textInfo.FontName,
                                                  nullptr,
                                                  DWRITE_FONT_WEIGHT_NORMAL,
                                                  DWRITE_FONT_STYLE_NORMAL,
                                                  DWRITE_FONT_STRETCH_NORMAL,
                                                  m_textInfo.FontSize,
                                                  L"en-US",
                                                  m_textFormat.put()));
    CHECK_HRCMD(m_textFormat->SetTextAlignment(m_textInfo.TextAlignment));
    CHECK_HRCMD(m_textFormat->SetParagraphAlignment(m_textInfo.ParagraphAlignment));

    CHECK_HRCMD(m_d2dFactory->CreateDrawingStateBlock(m_stateBlock.put()));

    //
    // Set up 2D rendering modes.
    //
    const D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
                                                                             D2D1::PixelFormat(TextFormat, D2D1_ALPHA_MODE_PREMULTIPLIED));

    const auto texDesc = CD3D11_TEXTURE2D_DESC(TextFormat,
                                               m_textInfo.Width,
                                               m_textInfo.Height,
                                               1,
                                               1,
                                               D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
                                               D3D11_USAGE_DEFAULT,
                                               0,
                                               1,
                                               0,
                                               0);
    CHECK_HRCMD(device->CreateTexture2D(&texDesc, nullptr, m_textDWriteTexture.put()));

    winrt::com_ptr<IDXGISurface> dxgiPerfBuffer = m_textDWriteTexture.as<IDXGISurface>();
    CHECK_HRCMD(m_d2dContext->CreateBitmapFromDxgiSurface(dxgiPerfBuffer.get(), &bitmapProperties, m_d2dTargetBitmap.put()));

    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
    m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    m_d2dContext->SetTransform(D2D1::Matrix3x2F::Identity());

    const auto& foreground = m_textInfo.Foreground;
    const D2D1::ColorF brushColor(foreground.x, foreground.y, foreground.z, foreground.w);
    CHECK_HRCMD(m_d2dContext->CreateSolidColorBrush(brushColor, m_brush.put()));
}

void engine::TextTexture::Draw(std::string_view text) {
    m_d2dContext->SaveDrawingState(m_stateBlock.get());

    const D2D1_SIZE_F renderTargetSize = m_d2dContext->GetSize();
    m_d2dContext->BeginDraw();

    const auto& background = m_textInfo.Background;
    m_d2dContext->Clear(D2D1::ColorF(background.x, background.y, background.z, background.w));

    if (!text.empty()) {
        const auto& margin = m_textInfo.Margin;
        std::wstring wtext = xr::utf8_to_wide(text);
        m_d2dContext->DrawText(wtext.c_str(),
                               (UINT32)wtext.size(),
                               m_textFormat.get(),
                               D2D1::RectF(m_textInfo.Margin,
                                           m_textInfo.Margin,
                                           renderTargetSize.width - m_textInfo.Margin * 2,
                                           renderTargetSize.height - m_textInfo.Margin * 2),
                               m_brush.get());
    }

    m_d2dContext->EndDraw();

    m_d2dContext->RestoreDrawingState(m_stateBlock.get());
}

ID3D11Texture2D* engine::TextTexture::Texture() const {
    return m_textDWriteTexture.get();
}

std::shared_ptr<Pbr::Material> engine::TextTexture::CreatePbrMaterial(const Pbr::Resources& pbrResources) const {
    auto material = Pbr::Material::CreateFlat(pbrResources, Pbr::RGBA::White);

    winrt::com_ptr<ID3D11ShaderResourceView> textSrv;
    CHECK_HRCMD(pbrResources.GetDevice()->CreateShaderResourceView(Texture(), nullptr, textSrv.put()));

    material->SetTexture(Pbr::ShaderSlots::BaseColor, textSrv.get());
    material->SetAlphaBlended(true);

    return material;
}
