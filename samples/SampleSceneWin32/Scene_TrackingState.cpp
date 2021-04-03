// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrUtility/XrToString.h>
#include <XrUtility/XrSide.h>
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/TextTexture.h>
#include <XrSceneLib/Scene.h>

using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    struct TrackingStateScene : public engine::Scene {
        TrackingStateScene(engine::Context& context)
            : Scene(context) {
            xr::ActionSet& actionSet = ActionContext().CreateActionSet("tracking_state_action_set", "Tracking state action set");
            const std::vector<std::string> subactionPathBothHands = {"/user/hand/right", "/user/hand/left"};
            m_gripSpaceAction = actionSet.CreateAction("grip_pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT, subactionPathBothHands);

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                                                              {
                                                                  {m_gripSpaceAction, "/user/hand/left/input/grip/pose"},
                                                                  {m_gripSpaceAction, "/user/hand/right/input/grip/pose"},
                                                              });

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                                                              {
                                                                  {m_gripSpaceAction, "/user/hand/left/input/grip/pose"},
                                                                  {m_gripSpaceAction, "/user/hand/right/input/grip/pose"},
                                                              });

            if (context.Extensions.SupportsHandInteraction) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                  {
                                                                      {m_gripSpaceAction, "/user/hand/left/input/grip/pose"},
                                                                      {m_gripSpaceAction, "/user/hand/right/input/grip/pose"},
                                                                  });
            }

            if (context.Extensions.SupportsHPMixedRealityController) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/hp/mixed_reality_controller",
                                                                  {
                                                                      {m_gripSpaceAction, "/user/hand/left/input/grip/pose"},
                                                                      {m_gripSpaceAction, "/user/hand/right/input/grip/pose"},
                                                                  });
            }

            XrActionSpaceCreateInfo spaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            spaceCreateInfo.poseInActionSpace = Pose::Identity();
            spaceCreateInfo.action = m_gripSpaceAction;
            spaceCreateInfo.poseInActionSpace = Pose::Identity();

            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                spaceCreateInfo.subactionPath = xr::StringToPath(m_context.Instance.Handle, xr::Side::UserPath[side]);
                CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &spaceCreateInfo, m_gripSpace[side].Put()));

                const float offset = side == xr::Side::Left ? -0.5f : 0.5f;
                AddTextBlock(m_handTextBlock[side], Pose::Translation({offset, 0, -1.0f}), {0.4f, 0.3f}, {256, 256});
            }

            // Add text block for head/view reference space
            {
                AddTextBlock(m_headTextBlock, Pose::Translation({0, 0, -1.2f}), {0.4f, 0.3f}, {256, 256});

                XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
                createInfo.poseInReferenceSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_context.Session.Handle, &createInfo, m_viewSpace.Put()));
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            // Update each hand's grip pose and tracking state
            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                XrSpaceLocation gripInAppSpace = {XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &gripInAppSpace));

                XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                getInfo.action = m_gripSpaceAction;
                getInfo.subactionPath = xr::StringToPath(m_context.Instance.Handle, xr::Side::UserPath[side]);
                XrActionStatePose state{XR_TYPE_ACTION_STATE_POSE};
                CHECK_XRCMD(xrGetActionStatePose(m_context.Session.Handle, &getInfo, &state));

                fmt::memory_buffer buffer;
                fmt::format_to(buffer, "{} hand tracking\n{}/input/grip/pose\n\n", xr::Side::Name[side], xr::Side::UserPath[side]);
                fmt::format_to(buffer, "IsValid={}\n", Pose::IsPoseValid(gripInAppSpace));
                fmt::format_to(buffer, "IsTracked={}\n", Pose::IsPoseTracked(gripInAppSpace));
                fmt::format_to(buffer, "IsActive={}\n", state.isActive);
                UpdateTextBlock(m_handTextBlock[side], buffer.data());
            }

            // Update VIEW reference space and inspect if head tracking state
            {
                XrSpaceLocation viewInAppSpace = {XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &viewInAppSpace));

                fmt::memory_buffer buffer;
                fmt::format_to(buffer, "Head tracking\na.k.a VIEW reference space\n\n");
                fmt::format_to(buffer, "IsValid={}\n", Pose::IsPoseValid(viewInAppSpace));
                fmt::format_to(buffer, "IsTracked={}\n", Pose::IsPoseTracked(viewInAppSpace));
                UpdateTextBlock(m_headTextBlock, buffer.data());
            }
        }

    private:
        struct TextBlock {
            std::string Text;
            std::unique_ptr<engine::TextTexture> TextTexture;
            std::shared_ptr<engine::PbrModelObject> Object;
        };

        void AddTextBlock(TextBlock& textBlock, const XrPosef& center, const XrExtent2Df& size, const XrExtent2Di& pixelSize) {
            uint32_t pixelWidth = pixelSize.width;
            uint32_t pixelHeight = (uint32_t)std::floor(size.height * pixelWidth / size.width); // Keep texture aspect ratio

            textBlock.TextTexture = std::make_unique<engine::TextTexture>(m_context, GetTextInfo(pixelWidth, pixelHeight));
            textBlock.TextTexture->Draw(xr::utf8_to_wide(textBlock.Text).c_str());

            const auto material = textBlock.TextTexture->CreatePbrMaterial(m_context.PbrResources);
            textBlock.Object = AddObject(engine::CreateQuad(m_context.PbrResources, {size.width, size.height}, material));
            textBlock.Object->Pose() = center;
        }

        void UpdateTextBlock(TextBlock& textBlock, const char* text) {
            if (strcmp(textBlock.Text.c_str(), text) != 0) {
                textBlock.Text = text;
                textBlock.TextTexture->Draw(xr::utf8_to_wide(textBlock.Text).c_str());
                const auto material = textBlock.TextTexture->CreatePbrMaterial(m_context.PbrResources);
                textBlock.Object->GetModel()->GetPrimitive(0).SetMaterial(material);
            }
        }

        static engine::TextTextureInfo GetTextInfo(uint32_t pixelWidth, uint32_t pixelHeight) {
            engine::TextTextureInfo textInfo(pixelWidth, pixelHeight); // pixels
            textInfo.Margin = 5;                                       // pixels
            textInfo.FontSize = 16.0f;                                 // DPI
            textInfo.Foreground = Pbr::RGBA::White;
            textInfo.Background = Pbr::FromSRGB(DirectX::Colors::DarkSlateBlue);
            textInfo.TextAlignment = DWRITE_TEXT_ALIGNMENT_LEADING;
            textInfo.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;
            return textInfo;
        }

    private:
        xr::SpaceHandle m_viewSpace;
        xr::SpaceHandle m_gripSpace[xr::Side::Count];
        XrAction m_gripSpaceAction{};

        TextBlock m_headTextBlock;
        TextBlock m_handTextBlock[xr::Side::Count];
    };

} // namespace

std::unique_ptr<engine::Scene> TryCreateTrackingStateScene(engine::Context& context) {
    return std::make_unique<TrackingStateScene>(context);
}
