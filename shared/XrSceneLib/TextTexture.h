// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <dxgi.h>
#include <d2d1_2.h>
#include <dwrite_2.h>

#include "pbr/PbrMaterial.h"
#include "Context.h"

namespace engine {

    struct TextTextureInfo {
        TextTextureInfo(uint32_t width, uint32_t height)
            : Width(width)
            , Height(height) {
        }

        uint32_t Width;
        uint32_t Height;
        const wchar_t* FontName = L"Segoe UI";
        float FontSize = 18;
        float Margin = 0;
        Pbr::RGBAColor Foreground = Pbr::RGBA::White;
        Pbr::RGBAColor Background = Pbr::RGBA::Transparent;
        DWRITE_TEXT_ALIGNMENT TextAlignment = DWRITE_TEXT_ALIGNMENT_CENTER;
        DWRITE_PARAGRAPH_ALIGNMENT ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_CENTER;
    };

    // Manages a texture which can be drawn to.
    class TextTexture {
    public:
        TextTexture(Context& context, TextTextureInfo textInfo);

        void Draw(const wchar_t* text);
        ID3D11Texture2D* Texture() const;
        std::shared_ptr<Pbr::Material> CreatePbrMaterial(const Pbr::Resources& pbrResources) const;

    private:
        const TextTextureInfo m_textInfo;
        winrt::com_ptr<ID2D1Factory2> m_d2dFactory;
        winrt::com_ptr<ID2D1Device1> m_d2dDevice;
        winrt::com_ptr<ID2D1DeviceContext1> m_d2dContext;
        winrt::com_ptr<ID2D1Bitmap1> m_d2dTargetBitmap;
        winrt::com_ptr<ID2D1SolidColorBrush> m_brush;
        winrt::com_ptr<ID2D1DrawingStateBlock> m_stateBlock;
        winrt::com_ptr<IDWriteFactory2> m_dwriteFactory;
        winrt::com_ptr<IDWriteTextFormat> m_textFormat;
        winrt::com_ptr<ID3D11Texture2D> m_textDWriteTexture;
    };

} // namespace engine
