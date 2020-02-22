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
#pragma once

#include <openxr/openxr.h>

#include <dxgi.h>
#include <d2d1_2.h>
#include <dwrite_2.h>

#include "SceneObject.h"

// Manages a texture which can be drawn to.
class TextTexture {
public:
    TextTexture(SceneContext* sceneContext,
                uint32_t width,
                uint32_t height,
                const wchar_t* fontName = L"Segoe UI",
                float fontSize = 18,
                Pbr::RGBAColor color = Pbr::RGBA::White);

    void Draw(const wchar_t* text);
    ID3D11Texture2D* Texture() const;
    std::shared_ptr<Pbr::Material> CreatePbrMaterial(const Pbr::Resources& pbrResources) const;

private:
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
