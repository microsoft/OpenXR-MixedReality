// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrUtility/XrToString.h>
#include <XrUtility/XrSide.h>
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/SpaceObject.h>
#include <XrSceneLib/TextTexture.h>
#include <XrSceneLib/Scene.h>

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {

    constexpr char const* UserHandPath[xr::Side::Count] = {{"/user/hand/left"}, {"/user/hand/right"}};

    struct InteractionProfiles {
        static constexpr char const* SimpleController = "/interaction_profiles/khr/simple_controller";
        static constexpr char const* MotionController = "/interaction_profiles/microsoft/motion_controller";
        static constexpr char const* TouchController = "/interaction_profiles/oculus/touch_controller";
        static constexpr char const* HPMixedRealityController = "/interaction_profiles/hp/mixed_reality_controller";
        static constexpr char const* HandInteraction = "/interaction_profiles/microsoft/hand_interaction";
        static constexpr char const* SamsungController = "/interaction_profiles/samsung/odyssey_controller";
    };

    constexpr char const* aimPoseActionName[xr::Side::Count] = {"left_aim", "right_aim"};
    constexpr char const* gripPoseActionName[xr::Side::Count] = {"left_grip", "right_grip"};

    //
    // This sample visualizes the current interaction profile and its controller components
    // as a list of slide bars to their latest values. It also visualizes aim and grip pose.
    //
    struct ControllerActionsScene : public engine::Scene {
        ControllerActionsScene(engine::Context& context)
            : Scene(context)
            , m_actions(CreateActions(ActionContext(), "controller_actions_scene_actionset")) {
            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                ControllerData& controllerData = m_controllerData[side];
                controllerData.side = side;
                controllerData.userPathString = UserHandPath[side];
                controllerData.userPath = xr::StringToPath(m_context.Instance.Handle, UserHandPath[side]);

                InitializeSuggestBindings(side, ActionContext(), m_context.Extensions, m_actions);

                // Initialize objects attached to aim pose
                {
                    XrAction aimAction = FindAction(m_actions, aimPoseActionName[side]).action;
                    xr::SpaceHandle aimSpace = CreateActionSpace(context.Session.Handle, aimAction);
                    controllerData.aimRoot = AddObject(engine::CreateSpaceObject(std::move(aimSpace)));
                    controllerData.aimRoot->SetVisible(false);

                    auto aimRay = AddObject(engine::CreateCube(m_context.PbrResources, {0.001f, 0.001f, 2.f}, Pbr::RGBA::White));
                    aimRay->Pose() = xr::math::Pose::Translation({0, 0, -1.f});
                    aimRay->SetParent(controllerData.aimRoot);

                    auto axis = AddObject(engine::CreateAxis(m_context.PbrResources, 0.05f, 0.001f));
                    axis->SetParent(controllerData.aimRoot);
                }

                // Initialize objects attached to grip pose
                {
                    XrAction gripAction = FindAction(m_actions, gripPoseActionName[side]).action;
                    xr::SpaceHandle gripSpace = CreateActionSpace(context.Session.Handle, gripAction);
                    controllerData.gripRoot = AddObject(engine::CreateSpaceObject(std::move(gripSpace)));

                    auto axis = AddObject(engine::CreateAxis(m_context.PbrResources, 0.05f, 0.001f));
                    axis->SetParent(controllerData.gripRoot);
                }

                m_interactionProfilesDirty = true;
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            if (m_interactionProfilesDirty.exchange(false)) {
                for (auto side : {xr::Side::Left, xr::Side::Right}) {
                    ControllerData& controllerData = m_controllerData[side];
                    XrInteractionProfileState state{XR_TYPE_INTERACTION_PROFILE_STATE};
                    CHECK_XRCMD(xrGetCurrentInteractionProfile(m_context.Session.Handle, controllerData.userPath, &state));

                    if (controllerData.interactionProfilePath != state.interactionProfile) {
                        InteractionProfileChanged(*this, m_context, controllerData, m_actions, side, state.interactionProfile);
                    }
                }
            }

            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                // Update the value and visual for each controller component
                for (auto& component : m_controllerData[side].components) {
                    UpdateComponentValueVisuals(m_context, xr::StringToPath(m_context.Instance.Handle, UserHandPath[side]), component);
                }
            }
        }

        void OnEvent(const XrEventDataBuffer& eventData [[maybe_unused]]) override {
            if (auto* internactionProfileChanged = xr::event_cast<XrEventDataInteractionProfileChanged>(&eventData)) {
                m_interactionProfilesDirty = true;

                XrInteractionProfileState state{XR_TYPE_INTERACTION_PROFILE_STATE};
                CHECK_XRCMD(xrGetCurrentInteractionProfile(m_context.Session.Handle, m_context.Instance.LeftHandPath, &state));
                std::string leftPath = state.interactionProfile == XR_NULL_PATH
                                           ? "NULL"
                                           : xr::PathToString(m_context.Instance.Handle, state.interactionProfile);

                CHECK_XRCMD(xrGetCurrentInteractionProfile(m_context.Session.Handle, m_context.Instance.RightHandPath, &state));
                std::string rightPath = state.interactionProfile == XR_NULL_PATH
                                            ? "NULL"
                                            : xr::PathToString(m_context.Instance.Handle, state.interactionProfile);

                sample::Trace("Interaction profile is changed.\n\tLeft: {}\n\tRight:{}\n", leftPath.c_str(), rightPath.c_str());
            }
        }

    private:
        struct ActionBinding {
            const char* interactionProfile{}; // e.g. /interaction_profiles/khr/simple_controller
            const char* componetPath{};       // e.g. select/click, will be prefixed by /top_level_user_path/iput later.
            const char* subactionPath{};      // e.g. /user/hand/left, /user/hand/right or nullptr which means add both subaction paths.
        };

        struct ActionInfo {
            XrAction action;
            std::string actionName;
            XrActionType actionType;

            std::vector<ActionBinding> actionBindings;
        };

        struct ComponentData {
            const ActionInfo& actionInfo;

            float actionValue;
            bool isActive;
            std::string text;
            std::shared_ptr<engine::Object> placementObject;
            std::shared_ptr<engine::PbrModelObject> valueObject;
        };

        struct ControllerData {
            uint32_t side;
            std::string userPathString;
            XrPath userPath;

            XrPath interactionProfilePath;
            std::string interactionProfileName;

            std::string text;
            std::shared_ptr<engine::Object> textObject;

            std::vector<ComponentData> components;

            std::shared_ptr<engine::Object> gripRoot;
            std::shared_ptr<engine::Object> aimRoot;
        };

        const std::vector<ActionInfo> m_actions;
        ControllerData m_controllerData[xr::Side::Count]{};
        std::atomic<bool> m_interactionProfilesDirty{false};

    private:
        static std::vector<ActionInfo> CreateActions(sample::ActionContext& actionContext, const char* actionSetName) {
            sample::ActionSet& actionSet = actionContext.CreateActionSet(actionSetName, actionSetName);
            std::vector<ActionInfo> actions{};

            const std::vector<std::string> bothHands = {UserHandPath[xr::Side::Left], UserHandPath[xr::Side::Right]};
            auto addAction = [&](const char* actionName, XrActionType actionType, std::vector<ActionBinding> bindings) {
                ActionInfo actionInfo;
                actionInfo.actionName = actionName;
                actionInfo.actionType = actionType;
                actionInfo.action = actionSet.CreateAction(actionName, actionName, actionType, bothHands);
                actionInfo.actionBindings = std::move(bindings);
                actions.emplace_back(actionInfo);
            };

            addAction("select",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::SimpleController, "select/click", nullptr},
                      });

            addAction("trigger_select_value",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "trigger/value", nullptr},
                          {InteractionProfiles::MotionController, "trigger/value", nullptr},
                          {InteractionProfiles::SamsungController, "trigger/value", nullptr},
                          {InteractionProfiles::HandInteraction, "select/value", nullptr},
                          {InteractionProfiles::TouchController, "trigger/value", nullptr},
                      });
            addAction("squeeze",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "squeeze/value", nullptr},
                          {InteractionProfiles::MotionController, "squeeze/click", nullptr},
                          {InteractionProfiles::SamsungController, "squeeze/click", nullptr},
                          {InteractionProfiles::HandInteraction, "squeeze/value", nullptr},
                          {InteractionProfiles::TouchController, "squeeze/value", nullptr},
                      });
            addAction("thumbstick_x",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "thumbstick/x", nullptr},
                          {InteractionProfiles::MotionController, "thumbstick/x", nullptr},
                          {InteractionProfiles::SamsungController, "thumbstick/x", nullptr},
                          {InteractionProfiles::TouchController, "thumbstick/x", nullptr},
                      });
            addAction("thumbstick_y",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "thumbstick/y", nullptr},
                          {InteractionProfiles::MotionController, "thumbstick/y", nullptr},
                          {InteractionProfiles::SamsungController, "thumbstick/y", nullptr},
                          {InteractionProfiles::TouchController, "thumbstick/y", nullptr},
                      });
            addAction("thumbstick_click",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "thumbstick/click", nullptr},
                          {InteractionProfiles::MotionController, "thumbstick/click", nullptr},
                          {InteractionProfiles::SamsungController, "thumbstick/click", nullptr},
                          {InteractionProfiles::TouchController, "thumbstick/click", nullptr},
                      });
            addAction("trackpad_x",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::MotionController, "trackpad/x", nullptr},
                          {InteractionProfiles::SamsungController, "trackpad/x", nullptr},
                      });
            addAction("trackpad_y",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::MotionController, "trackpad/y", nullptr},
                          {InteractionProfiles::SamsungController, "trackpad/y", nullptr},
                      });
            addAction("trackpad_touch",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::MotionController, "trackpad/touch", nullptr},
                          {InteractionProfiles::SamsungController, "trackpad/touch", nullptr},
                      });
            addAction("trackpad_click",
                      XR_ACTION_TYPE_FLOAT_INPUT,
                      {
                          {InteractionProfiles::MotionController, "trackpad/click", nullptr},
                          {InteractionProfiles::SamsungController, "trackpad/click", nullptr},
                      });
            addAction("a",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "a/click", UserHandPath[xr::Side::Right]},
                          {InteractionProfiles::TouchController, "a/click", UserHandPath[xr::Side::Right]},
                      });
            addAction("b",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "b/click", UserHandPath[xr::Side::Right]},
                          {InteractionProfiles::TouchController, "b/click", UserHandPath[xr::Side::Right]},
                      });
            addAction("x",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "x/click", UserHandPath[xr::Side::Left]},
                          {InteractionProfiles::TouchController, "x/click", UserHandPath[xr::Side::Left]},
                      });
            addAction("y",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "y/click", UserHandPath[xr::Side::Left]},
                          {InteractionProfiles::TouchController, "y/click", UserHandPath[xr::Side::Left]},
                      });
            addAction("menu",
                      XR_ACTION_TYPE_BOOLEAN_INPUT,
                      {
                          {InteractionProfiles::HPMixedRealityController, "menu/click", nullptr},
                          {InteractionProfiles::MotionController, "menu/click", nullptr},
                          {InteractionProfiles::SamsungController, "menu/click", nullptr},
                          {InteractionProfiles::SimpleController, "menu/click", nullptr},
                          {InteractionProfiles::TouchController, "menu/click", UserHandPath[xr::Side::Left]},
                      });

            for (auto side : {xr::Side::Left, xr::Side::Right}) {
                addAction(aimPoseActionName[side],
                          XR_ACTION_TYPE_POSE_INPUT,
                          {
                              {InteractionProfiles::SimpleController, "aim/pose", UserHandPath[side]},
                              {InteractionProfiles::HPMixedRealityController, "aim/pose", UserHandPath[side]},
                              {InteractionProfiles::MotionController, "aim/pose", UserHandPath[side]},
                              {InteractionProfiles::SamsungController, "aim/pose", UserHandPath[side]},
                              {InteractionProfiles::HandInteraction, "aim/pose", UserHandPath[side]},
                              {InteractionProfiles::TouchController, "aim/pose", UserHandPath[side]},
                          });

                addAction(gripPoseActionName[side],
                          XR_ACTION_TYPE_POSE_INPUT,
                          {
                              {InteractionProfiles::SimpleController, "grip/pose", UserHandPath[side]},
                              {InteractionProfiles::HPMixedRealityController, "grip/pose", UserHandPath[side]},
                              {InteractionProfiles::MotionController, "grip/pose", UserHandPath[side]},
                              {InteractionProfiles::SamsungController, "grip/pose", UserHandPath[side]},
                              {InteractionProfiles::HandInteraction, "grip/pose", UserHandPath[side]},
                              {InteractionProfiles::TouchController, "grip/pose", UserHandPath[side]},
                          });
            }

            return actions;
        }

        static const ActionInfo& FindAction(const std::vector<ActionInfo>& actions, const std::string& actionName) {
            const auto& it =
                std::find_if(actions.begin(), actions.end(), [&](const ActionInfo& action) { return action.actionName == actionName; });
            assert(it != actions.end());
            return *it;
        }

        static xr::SpaceHandle CreateActionSpace(XrSession session, XrAction action) {
            xr::SpaceHandle space;
            XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            createInfo.action = action;
            createInfo.poseInActionSpace = xr::math::Pose::Identity();
            createInfo.subactionPath = XR_NULL_PATH;
            CHECK_XRCMD(xrCreateActionSpace(session, &createInfo, space.Put(xrDestroySpace)));
            return space;
        }

        static void InitializeSuggestBindings(uint32_t side,
                                              sample::ActionContext& actionContext,
                                              const xr::ExtensionContext& extensions,
                                              const std::vector<ActionInfo>& actions) {
            const std::string subactionPath = UserHandPath[side];

            std::map<std::string, std::vector<sample::ActionContext::ActionBinding>> suggestedBindings;
            for (const auto& actionInfo : actions) {
                XrAction action = actionInfo.action;
                for (const auto& [profilePath, componentPath, subactionPath] : actionInfo.actionBindings) {
                    if (subactionPath == nullptr) {
                        suggestedBindings[profilePath].push_back({action, std::string() + "/user/hand/left/input/" + componentPath});
                        suggestedBindings[profilePath].push_back({action, std::string() + "/user/hand/right/input/" + componentPath});
                    } else {
                        suggestedBindings[profilePath].push_back({action, std::string() + subactionPath + "/input/" + componentPath});
                    }
                }
            }

            actionContext.SuggestInteractionProfileBindings(InteractionProfiles::SimpleController,
                                                            suggestedBindings[InteractionProfiles::SimpleController]);
            actionContext.SuggestInteractionProfileBindings(InteractionProfiles::MotionController,
                                                            suggestedBindings[InteractionProfiles::MotionController]);
            actionContext.SuggestInteractionProfileBindings(InteractionProfiles::TouchController,
                                                            suggestedBindings[InteractionProfiles::TouchController]);

            if (extensions.SupportsHPMixedRealityController) {
                actionContext.SuggestInteractionProfileBindings(InteractionProfiles::HPMixedRealityController,
                                                                suggestedBindings[InteractionProfiles::HPMixedRealityController]);
            }

            if (extensions.SupportsHandInteraction) {
                actionContext.SuggestInteractionProfileBindings(InteractionProfiles::HandInteraction,
                                                                suggestedBindings[InteractionProfiles::HandInteraction]);
            }

            if (extensions.SupportsSamsungOdysseyController) {
                actionContext.SuggestInteractionProfileBindings(InteractionProfiles::SamsungController,
                                                                suggestedBindings[InteractionProfiles::SamsungController]);
            }
        }

        // Update the visualization of controller components when interaction profile is changed.
        static void InteractionProfileChanged(engine::Scene& scene,
                                              engine::Context& context,
                                              ControllerData& controllerData,
                                              const std::vector<ActionInfo>& actions,
                                              uint32_t side,
                                              XrPath interactionProfilePath) {
            controllerData.interactionProfilePath = interactionProfilePath;
            const bool hasInteractionProfile = controllerData.interactionProfilePath != XR_NULL_PATH;
            controllerData.interactionProfileName = hasInteractionProfile
                                                        ? xr::PathToString(context.Instance.Handle, controllerData.interactionProfilePath)
                                                        : "No interaction profile";

            // Remove previous components
            for (auto& object : controllerData.components) {
                scene.RemoveObject(object.placementObject);
                scene.RemoveObject(object.valueObject);
            }
            controllerData.components.clear();
            scene.RemoveObject(controllerData.textObject);
            controllerData.textObject = nullptr;

            // Insert components of the current interaction profile
            if (hasInteractionProfile) {
                for (const auto& actionInfo : actions) {
                    auto it =
                        std::find_if(actionInfo.actionBindings.begin(), actionInfo.actionBindings.end(), [&](const ActionBinding& binding) {
                            return strcmp(binding.interactionProfile, controllerData.interactionProfileName.c_str()) == 0 &&
                                   (binding.subactionPath == nullptr || strcmp(binding.subactionPath, UserHandPath[side]) == 0);
                        });

                    if (it == actionInfo.actionBindings.end()) {
                        continue;
                    }

                    ComponentData component{actionInfo};
                    component.text = it->componetPath;

                    component.placementObject = engine::CreateObject();
                    component.placementObject->SetParent(controllerData.gripRoot);
                    const float x = side == xr::Side::Left ? -0.1f : 0.1f;
                    const float y = 0.0f;
                    const float z = -0.073f + controllerData.components.size() * (0.0096f * 1.47f);
                    component.placementObject->Pose() = xr::math::Pose::Translation({x, y, z});

                    component.valueObject = engine::CreateCube(context.PbrResources, {1, 1, 1}, Pbr::RGBA::White);
                    component.valueObject->SetParent(component.placementObject);
                    controllerData.components.emplace_back(std::move(component));
                }
            }

            auto ConcatNames = [](const std::set<std::string>& names) -> std::string {
                if (names.size() == 0) {
                    return "No Binding";
                }

                std::string finalNames;
                uint32_t i = 0;
                for (auto& name : names) {
                    finalNames += name;
                    if (i < names.size() - 1) {
                        finalNames += "\n";
                    }
                    i++;
                }

                return finalNames;
            };

            auto GetControllerString = [&ConcatNames](XrSession session, XrAction action) -> std::string {
                std::string interactionProfileString =
                    ConcatNames(xr::QueryActionLocalizedName(session, action, XR_INPUT_SOURCE_LOCALIZED_NAME_INTERACTION_PROFILE_BIT));
                std::string userPathString =
                    ConcatNames(xr::QueryActionLocalizedName(session, action, XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT));
                return fmt::format("{}\n{}", interactionProfileString.c_str(), userPathString.c_str());
            };

            auto GetComponentString = [&ConcatNames](XrSession session, XrAction action) -> std::string {
                std::string userPath =
                    ConcatNames(xr::QueryActionLocalizedName(session, action, XR_INPUT_SOURCE_LOCALIZED_NAME_USER_PATH_BIT));
                std::string component =
                    ConcatNames(xr::QueryActionLocalizedName(session, action, XR_INPUT_SOURCE_LOCALIZED_NAME_COMPONENT_BIT));
                return userPath + ", " + component;
            };

            // Print interaction profile information in text
            if (controllerData.components.size() > 0) {
                fmt::memory_buffer buffer;
                std::string controllerLocalizedString =
                    GetControllerString(context.Session.Handle, controllerData.components[0].actionInfo.action);
                fmt::format_to(buffer, "{}\n{}\n", controllerData.interactionProfileName.c_str(), controllerLocalizedString.c_str());
                if (hasInteractionProfile) {
                    for (const auto& component : controllerData.components) {
                        std::string componentString = GetComponentString(context.Session.Handle, component.actionInfo.action);
                        fmt::format_to(buffer, "\n{}:\n{}\n", component.text.c_str(), componentString.c_str());
                        scene.AddObject(component.placementObject);
                        scene.AddObject(component.valueObject);
                    }
                }
                sample::Trace(fmt::to_string(buffer));

                // Draw text object for the new interaction profile and components
                controllerData.text = fmt::to_string(buffer);
                controllerData.textObject = scene.AddObject(CreateTextObject(context, controllerData.side, controllerData.text));
                controllerData.textObject->SetParent(controllerData.gripRoot);
                const float offset = controllerData.side == xr::Side::Left ? -0.05f : 0.05f;
                controllerData.textObject->Pose() = {{-0.707f, 0, 0, 0.707f}, {offset, -0.01f, 0}};
                controllerData.textObject->Scale() = {0.1f, 0.1f, 0.1f};
            }
        }

        static std::shared_ptr<engine::Object> CreateTextObject(engine::Context& context, uint32_t side, std::string_view text) {
            constexpr uint32_t width = 480, height = 960;
            engine::TextTextureInfo textInfo{width, height}; // pixels
            textInfo.FontSize = 18;
            textInfo.Foreground = Pbr::RGBA::White;
            textInfo.Background = Pbr::FromSRGB(Colors::DarkSlateGray);
            textInfo.Margin = 5; // pixels
            textInfo.TextAlignment = side == xr::Side::Left ? DWRITE_TEXT_ALIGNMENT_LEADING : DWRITE_TEXT_ALIGNMENT_TRAILING;
            textInfo.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT_NEAR;

            auto textTexture = std::make_unique<engine::TextTexture>(context, textInfo);
            textTexture->Draw(xr::utf8_to_wide(text).c_str());
            const auto& material = textTexture->CreatePbrMaterial(context.PbrResources);

            // The new quad always have width of 1 meter
            const float quadHeight = float(height) / float(width);
            return engine::CreateQuad(context.PbrResources, {1, quadHeight}, material);
        }

        static void UpdateComponentValueVisuals(engine::Context& context, XrPath subactionPath, ComponentData& component) {
            XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            getInfo.action = component.actionInfo.action;
            getInfo.subactionPath = subactionPath;
            if (component.actionInfo.actionType == XR_ACTION_TYPE_BOOLEAN_INPUT) {
                XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
                CHECK_XRCMD(xrGetActionStateBoolean(context.Session.Handle, &getInfo, &state));
                component.actionValue = (float)state.currentState;
                component.isActive = state.isActive;
            } else if (component.actionInfo.actionType == XR_ACTION_TYPE_FLOAT_INPUT) {
                XrActionStateFloat state{XR_TYPE_ACTION_STATE_FLOAT};
                CHECK_XRCMD(xrGetActionStateFloat(context.Session.Handle, &getInfo, &state));
                component.actionValue = state.currentState;
                component.isActive = state.isActive;
            } else if (component.actionInfo.actionType == XR_ACTION_TYPE_POSE_INPUT) {
                XrActionStatePose state{XR_TYPE_ACTION_STATE_POSE};
                CHECK_XRCMD(xrGetActionStatePose(context.Session.Handle, &getInfo, &state));
                component.actionValue = state.isActive ? 1.f : 0.f;
                component.isActive = state.isActive;
            } else {
                assert(false); // Not sure how to visualize this action type yet.
            }

            // Update component visual to show a slide bar for the current value.
            const float u = 0.0021f;
            const float s = (1 + 10.f * component.actionValue) * u;
            const float p = (1 + 5.0f * component.actionValue) * u;
            component.valueObject->Pose() = xr::math::Pose::Translation({p, 0, 0});
            component.valueObject->Scale() = {s, u, u};
            component.valueObject->SetFillMode(component.isActive ? Pbr::FillMode::Solid : Pbr::FillMode::Wireframe);
        }
    };
} // namespace

std::unique_ptr<engine::Scene> TryCreateControllerActionsScene(engine::Context& context) {
    return std::make_unique<ControllerActionsScene>(context);
}
