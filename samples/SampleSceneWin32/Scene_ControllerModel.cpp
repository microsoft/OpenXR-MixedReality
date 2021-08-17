// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/Scene.h>
#include <XrSceneLib/ControllerObject.h>

namespace {
    struct ControllerModelScene : public engine::Scene {
        ControllerModelScene(engine::Context& context)
            : Scene(context)
            , m_leftController(context.Instance.LeftHandPath)
            , m_rightController(context.Instance.RightHandPath) {
            sample::ActionSet& actionSet = ActionContext().CreateActionSet("controller_model_action_set", "Controller Model Action Set");

            const std::vector<std::string> subactionPathBothHands = {"/user/hand/right", "/user/hand/left"};
            m_gripPoseAction = actionSet.CreateAction("grip_pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT, subactionPathBothHands);
            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                                                              {
                                                                  {m_gripPoseAction, "/user/hand/right/input/grip"},
                                                                  {m_gripPoseAction, "/user/hand/left/input/grip"},
                                                              });

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/oculus/touch_controller",
                                                              {
                                                                  {m_gripPoseAction, "/user/hand/right/input/grip"},
                                                                  {m_gripPoseAction, "/user/hand/left/input/grip"},
                                                              });

            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/khr/simple_controller",
                                                              {
                                                                  {m_gripPoseAction, "/user/hand/right/input/grip"},
                                                                  {m_gripPoseAction, "/user/hand/left/input/grip"},
                                                              });

            if (context.Extensions.SupportsHPMixedRealityController) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/hp/mixed_reality_controller",
                                                                  {
                                                                      {m_gripPoseAction, "/user/hand/right/input/grip"},
                                                                      {m_gripPoseAction, "/user/hand/left/input/grip"},
                                                                  });
            }

            XrActionSpaceCreateInfo actionSpaceCreateInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            actionSpaceCreateInfo.poseInActionSpace = xr::math::Pose::Identity();

            for (ControllerData& controller : {std::ref(m_leftController), std::ref(m_rightController)}) {
                actionSpaceCreateInfo.subactionPath = controller.UserPath;
                actionSpaceCreateInfo.action = m_gripPoseAction;
                CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &actionSpaceCreateInfo, controller.GripSpace.Put()));

                // Controller objects are created with empty model.  It will be loaded when available.
                controller.Object = AddObject(CreateControllerObject(m_context, controller.UserPath));
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            for (ControllerData& controller : {std::ref(m_leftController), std::ref(m_rightController)}) {
                // Update the grip pose and place the controller model to it.
                XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(controller.GripSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &location));
                if (xr::math::Pose::IsPoseValid(location)) {
                    controller.Object->SetVisible(true);
                    controller.Object->Pose() = location.pose;

                    controller.Object->SetFillMode(xr::math::Pose::IsPoseTracked(location) ? Pbr::FillMode::Solid
                                                                                           : Pbr::FillMode::Wireframe);
                } else {
                    controller.Object->SetVisible(false);
                }
            }
        }

    private:
        struct ControllerData {
            const XrPath UserPath;
            xr::SpaceHandle GripSpace{};
            std::shared_ptr<engine::PbrModelObject> Object;

            explicit ControllerData(XrPath userPath)
                : UserPath(userPath) {
            }
        };
        XrAction m_gripPoseAction{XR_NULL_HANDLE};
        ControllerData m_leftController, m_rightController;
    };

} // namespace

std::unique_ptr<engine::Scene> TryCreateControllerModelScene(engine::Context& context) {
    return context.Extensions.SupportsControllerModel ? std::make_unique<ControllerModelScene>(context) : nullptr;
}
