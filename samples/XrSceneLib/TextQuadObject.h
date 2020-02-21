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

#include <SampleShared/DxUtility.h>

#include "TextTexture.h"
#include "Scene.h"
#include "SceneObject.h"

class TextQuadObject : public SceneObject {
public:
    TextQuadObject(Scene* scene, XrSpace space, SceneContext* sceneContext, std::wstring text);
    void Update(const FrameTime& frameTime) override;
    void SetText(const std::wstring& text);
    void SetBackgroundColor(const XrColor4f& color);

    void SetContentProtectionEnabled(bool enabled);

    std::shared_ptr<QuadLayerObject> Quad() const {
        return m_quad;
    }

private:
    void PrepareTextSwapchain(XrSpace textSpace);

private:
    Scene* const m_scene;
    SceneContext* const m_sceneContext;

    std::wstring m_text;
    bool m_textChangeRequested{false};
    bool m_contentProtectionEnabled{false};
    XrColor4f m_backgroundColor{0.0f, 0.0f, 0.0f, 0.0f};

    TextTexture m_textTexture;

    std::shared_ptr<QuadLayerObject> m_quad;

    sample::dx::SwapchainD3D11 m_textSwapchain;
};
