// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    //
    // This sample displays a simple orbit in front of the user using local reference space
    // and shows how to pause the animation when the session lost focus while it continues rendering.
    // Also demos Gaze-Select interaction, that the user can air tap to move the orbit in front of the user.
    //
    struct OrbitScene : public engine::Scene {
        OrbitScene(engine::Context& context)
            : Scene(context) {
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("orbit_scene_actions", "Orbit Scene Actions");

            m_selectAction = actionSet.CreateAction("select_action", "Select Action", XR_ACTION_TYPE_BOOLEAN_INPUT, {});

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                                                              {
                                                                  {m_selectAction, "/user/hand/right/input/select"},
                                                                  {m_selectAction, "/user/hand/left/input/select"},
                                                              });

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                                                              {
                                                                  {m_selectAction, "/user/hand/right/input/trigger"},
                                                                  {m_selectAction, "/user/hand/left/input/trigger"},
                                                              });

            if (context.Extensions.SupportsHandInteraction) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                  {
                                                                      {m_selectAction, "/user/hand/right/input/select"},
                                                                      {m_selectAction, "/user/hand/left/input/select"},
                                                                  });
            }

            m_sun = AddObject(engine::CreateSphere(m_context.PbrResources, 0.5f, 20, Pbr::FromSRGB(Colors::OrangeRed)));
            m_sun->SetVisible(false); // invisible until tracking is valid and placement succeeded.

            m_earth = AddObject(engine::CreateSphere(m_context.PbrResources, 0.1f, 20, Pbr::FromSRGB(Colors::SeaGreen)));
            m_earth->SetParent(m_sun);

            XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            createInfo.poseInReferenceSpace = Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(m_context.Session.Handle, &createInfo, m_viewSpace.Put()));
        }

        void OnEvent(const XrEventDataBuffer& eventData [[maybe_unused]]) override {
            if (auto* internactionProfileChanged = xr::event_cast<XrEventDataInteractionProfileChanged>(&eventData)) {
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

        void OnUpdate(const engine::FrameTime& frameTime) override {
            XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
            XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            getInfo.action = m_selectAction;
            CHECK_XRCMD(xrGetActionStateBoolean(m_context.Session.Handle, &getInfo, &state));
            const bool isSelectPressed = state.isActive && state.changedSinceLastSync && state.currentState;
            const bool firstUpdate = !m_sun->IsVisible();

            if (firstUpdate || isSelectPressed) {
                const XrTime time = state.isActive ? state.lastChangeTime : frameTime.PredictedDisplayTime;
                XrSpaceLocation viewInAppSpace = {XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_context.AppSpace, time, &viewInAppSpace));

                if (Pose::IsPoseValid(viewInAppSpace)) {
                    // Project the forward of the view to the scene's horizontal plane
                    const XrPosef oneMeterInFrontInView = {{0, 0, 0, 1}, {0, 0, -1}};
                    const XrPosef oneMeterInFrontInAppSpace = oneMeterInFrontInView * viewInAppSpace.pose;
                    const XrVector3f viewForwardInAppSpace = oneMeterInFrontInAppSpace.position - viewInAppSpace.pose.position;
                    const XrVector3f viewForwardInGravity = Dot(viewForwardInAppSpace, {0, -1, 0}) * XrVector3f{0, -1, 0};
                    const XrVector3f eyeLevelForwardInAppSpace = Normalize(viewForwardInAppSpace - viewForwardInGravity);

                    // Put the sun 2 meters in front of the user at eye level
                    const XrVector3f sunInAppSpace = viewInAppSpace.pose.position + 2.f * eyeLevelForwardInAppSpace;
                    m_targetPoseInAppSpace = Pose::LookAt(sunInAppSpace, eyeLevelForwardInAppSpace, {0, 1, 0});

                    if (firstUpdate) {
                        m_sun->SetVisible(true);
                        m_sun->Pose() = m_targetPoseInAppSpace;
                    }
                }
            }

            // Slowly ease the sun to the target location
            m_sun->Pose() = Pose::Slerp(m_sun->Pose(), m_targetPoseInAppSpace, 0.05f);

            // Animate the earth orbiting the sun, and pause when app lost focus.
            if (frameTime.IsSessionFocused) {
                const float angle = frameTime.TotalElapsedSeconds * XM_PI; // half circle a second

                XrVector3f earthPosition;
                earthPosition.x = 0.6f * sin(angle);
                earthPosition.y = 0.0f;
                earthPosition.z = 0.6f * cos(angle);
                m_earth->Pose().position = earthPosition;
            }
        }

    private:
        XrAction m_selectAction{XR_NULL_HANDLE};
        XrPosef m_targetPoseInAppSpace = Pose::Identity();
        std::shared_ptr<engine::PbrModelObject> m_sun;
        std::shared_ptr<engine::PbrModelObject> m_earth;
        xr::SpaceHandle m_viewSpace;
    };
} // namespace

std::unique_ptr<engine::Scene> TryCreateOrbitScene(engine::Context& context) {
    return std::make_unique<OrbitScene>(context);
}
