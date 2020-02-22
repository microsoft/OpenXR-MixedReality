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

#include "TextQuadObject.h"

constexpr uint32_t TextWidth = 256;
constexpr uint32_t TextHeight = 256;

TextQuadObject::TextQuadObject(Scene* scene, XrSpace space, SceneContext* sceneContext, std::wstring text)
    : m_scene(scene)
    , m_text(std::move(text))
    , m_sceneContext(sceneContext)
    , m_textTexture(sceneContext, TextWidth, TextHeight, L"Consolas", 18.0f) {
    D3D11_TEXTURE2D_DESC textDesc;
    m_textTexture.Texture()->GetDesc(&textDesc);

    m_textSwapchain =
        sample::dx::CreateSwapchainD3D11(m_sceneContext->Session,
                                         textDesc.Format,
                                         TextWidth,
                                         TextHeight,
                                         /*arrayLength*/ 1,
                                         /*sampleCount*/ 1,
                                         /*createFlags*/ (m_contentProtectionEnabled ? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0),
                                         /*usage Flags*/ XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT);

    PrepareTextSwapchain(space);
}

void TextQuadObject::Update(const FrameTime& frameTime) {
    if (m_textChangeRequested) {
        // m_quad is removed from render loop, but m_quad is a shared_ptr so
        // we are still holding it until PrepareTextSwapchain() call
        XrSpace lastTextQuadSpace = m_quad->Space;

        PrepareTextSwapchain(lastTextQuadSpace);
        m_textChangeRequested = false;
    }

    m_quad->SetVisible(IsVisible());
}

void TextQuadObject::SetText(const std::wstring& text) {
    if (m_text != text) {
        m_textChangeRequested = true;

        // Queue text quad removal; it will be removed before next call to Update()
        m_scene->RemoveSceneObject(m_quad);
        m_text = text;
    }
}

void TextQuadObject::SetContentProtectionEnabled(bool enabled) {
    if (m_contentProtectionEnabled != enabled) {
        m_textChangeRequested = true;

        // Queue text quad removal; it will be removed before next call to Update()
        m_scene->RemoveSceneObject(m_quad);
        m_contentProtectionEnabled = enabled;
    }
}

void TextQuadObject::SetBackgroundColor(const XrColor4f& color) {
    m_backgroundColor = color;
}

void TextQuadObject::PrepareTextSwapchain(const XrSpace textSpace) {
    XrSwapchainSubImage textImageData{};
    textImageData.swapchain = m_textSwapchain.Handle.Get();
    textImageData.imageRect.extent = {TextWidth, TextHeight};

    m_quad = m_scene->AddQuadLayerObject(MakeQuadLayerObject(textSpace, textImageData));
    m_quad->Scale() = {0.2f, 0.2f, 0.2f};
    m_quad->CompositionLayerFlags |= XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;


    // Render text to the swapchain
    {
        uint32_t index;

        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        CHECK_XRCMD(xrAcquireSwapchainImage(m_textSwapchain.Handle.Get(), &acquireInfo, &index));

        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        CHECK_XRCMD(xrWaitSwapchainImage(m_textSwapchain.Handle.Get(), &waitInfo));

        m_textTexture.Draw(m_text.c_str());

        m_sceneContext->DeviceContext->CopyResource(m_textSwapchain.Images[index].texture, m_textTexture.Texture());

        const XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        CHECK_XRCMD(xrReleaseSwapchainImage(m_textSwapchain.Handle.Get(), &releaseInfo));
    }
}

