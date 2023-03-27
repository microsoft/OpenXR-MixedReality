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

    //
    // This sample displays the state of head tracking and controller/hand tracking into HMD view in front of user.
    // The head tracking state is done by observing the VIEW reference space and
    // the controller/hand tracking state is done by observing the left/right hand grip pose space.
    // https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XrSpaceLocationFlagBits
    //
    struct TrackingStateScene : public engine::Scene {
        TrackingStateScene(engine::Context& context)
            : Scene(context) {
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("tracking_state_action_set", "Tracking state action set");
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

            if (context.Extensions.SupportsHandInteractionMSFT) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                  {
                                                                      {m_gripSpaceAction, "/user/hand/left/input/grip/pose"},
                                                                      {m_gripSpaceAction, "/user/hand/right/input/grip/pose"},
                                                                  });
            }

            if (context.Extensions.SupportsHandInteractionEXT) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/ext/hand_interaction_ext",
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

            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                spaceCreateInfo.subactionPath = side == xr::Side::Left ? m_context.Instance.LeftHandPath : m_context.Instance.RightHandPath;
                CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &spaceCreateInfo, m_gripSpace[side].Put(xrDestroySpace)));

                const float offset = side == xr::Side::Left ? -0.5f : 0.5f;
                AddTextBlock(m_handTextBlock[side], Pose::Translation({offset, 0, -1.0f}), {0.4f, 0.3f}, {256, 256});
            }

            // Add text block for head/view reference space
            {
                AddTextBlock(m_headTextBlock, Pose::Translation({0, 0, -1.2f}), {0.4f, 0.3f}, {256, 256});

                XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
                createInfo.poseInReferenceSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_context.Session.Handle, &createInfo, m_viewSpace.Put(xrDestroySpace)));
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            XrSpaceLocation location = {XR_TYPE_SPACE_LOCATION};
            XrSpaceVelocity velocity = {XR_TYPE_SPACE_VELOCITY};
            location.next = &velocity;

            // Update each hand's grip pose and tracking state
            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                CHECK_XRCMD(xrLocateSpace(m_gripSpace[side].Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &location));

                fmt::memory_buffer buffer;
                fmt::format_to(fmt::appender(buffer),
                               "{} hand tracking\n{}/input/grip/pose\n\n",
                               side == xr::Side::Left ? "Left" : "Right",
                               side == xr::Side::Left ? "/user/hand/left" : "/user/hand/right");
                FormatTrackingState(buffer, location.locationFlags, velocity.velocityFlags);

                XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                getInfo.action = m_gripSpaceAction;
                getInfo.subactionPath = side == xr::Side::Left ? m_context.Instance.LeftHandPath : m_context.Instance.RightHandPath;
                XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
                CHECK_XRCMD(xrGetActionStatePose(m_context.Session.Handle, &getInfo, &poseState));

                fmt::format_to(fmt::appender(buffer), "poseState.IsActive={}\n", poseState.isActive);
                UpdateTextBlock(m_handTextBlock[side], fmt::to_string(buffer).c_str());
            }

            // Update VIEW reference space and inspect if head tracking state
            {
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &location));

                fmt::memory_buffer buffer;
                fmt::format_to(fmt::appender(buffer), "Head tracking\na.k.a VIEW reference space\n\n");
                FormatTrackingState(buffer, location.locationFlags, velocity.velocityFlags);
                UpdateTextBlock(m_headTextBlock, fmt::to_string(buffer).c_str());
            }
        }

        // return 0 and 1 to print shorter string inline
        template <typename FlagsT>
        uint32_t TestBit(FlagsT flags, FlagsT bit) {
            return (flags & bit) == bit ? 1 : 0;
        }

        void FormatTrackingState(fmt::memory_buffer& buffer, XrSpaceLocationFlags locationFlags, XrSpaceVelocityFlags velocityFlags) {
            fmt::format_to(fmt::appender(buffer),
                           "Pose Tracked: p={}, o={}\n",
                           TestBit(locationFlags, XR_SPACE_LOCATION_POSITION_TRACKED_BIT),
                           TestBit(locationFlags, XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT));
            fmt::format_to(fmt::appender(buffer),
                           "Pose Valid : p={}, o={}\n",
                           TestBit(locationFlags, XR_SPACE_LOCATION_POSITION_VALID_BIT),
                           TestBit(locationFlags, XR_SPACE_LOCATION_ORIENTATION_VALID_BIT));
            fmt::format_to(fmt::appender(buffer),
                           "Velocity Valid: lv={}, av={}\n",
                           TestBit(velocityFlags, XR_SPACE_VELOCITY_LINEAR_VALID_BIT),
                           TestBit(velocityFlags, XR_SPACE_VELOCITY_ANGULAR_VALID_BIT));
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
            textBlock.TextTexture->Draw(textBlock.Text);

            const auto material = textBlock.TextTexture->CreatePbrMaterial(m_context.PbrResources);
            textBlock.Object = AddObject(engine::CreateQuad(m_context.PbrResources, {size.width, size.height}, material));
            textBlock.Object->Pose() = center;
        }

        void UpdateTextBlock(TextBlock& textBlock, const char* text) {
            if (strcmp(textBlock.Text.c_str(), text) != 0) {
                textBlock.Text = text;
                textBlock.TextTexture->Draw(textBlock.Text);
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
