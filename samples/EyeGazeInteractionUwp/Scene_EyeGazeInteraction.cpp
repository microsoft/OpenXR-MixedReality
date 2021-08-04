// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"
#include <XrSceneLib/PbrModelObject.h>
#include <XrSceneLib/Scene.h>

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

//
// This sample shows the usage of eye gaze interaction extension.
// When the user is looking at a ball layed out in a circle around, the ball will rotate.
//
namespace {
    constexpr float objectDiameter = 0.1f; // In meters
    constexpr float layoutRadius = 1.5f;   // In meters
    constexpr int numberOfObjects = 36;    // Number of objects for a full circle.

    bool IsLookingAtObject(const XrPosef& gazeInAppSpace, const XrPosef& objectInAppSpace, float objectRadius) {
        const XrPosef appSpaceInGaze = Pose::Invert(gazeInAppSpace);
        const XrPosef objectInGaze = objectInAppSpace * appSpaceInGaze;
        const float projectionOnZ = Normalize(objectInGaze.position).z;
        const float distance = Length(objectInGaze.position);
        const float angleTolerance = std::atan2(objectRadius, distance);
        return projectionOnZ < -std::cos(angleTolerance);
    }

    struct EyeGazeInteractionScene : public engine::Scene {
        EyeGazeInteractionScene(engine::Context& context)
            : Scene(context) {
            const bool supportsEyeGazeAction =
                context.Extensions.SupportsEyeGazeInteraction && context.System.EyeGazeInteractionProperties.supportsEyeGazeInteraction;
            if (supportsEyeGazeAction) {
                sample::ActionSet& actionSet =
                    ActionContext().CreateActionSet("eye_gaze_interaction_scene_actions", "Eye Gaze Interaction Scene Actions");

                auto gazeAction = actionSet.CreateAction("gaze_action", "Gaze Action", XR_ACTION_TYPE_POSE_INPUT, {});

                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/ext/eye_gaze_interaction",
                                                                  {
                                                                      {gazeAction, "/user/eyes_ext/input/gaze_ext/pose"},
                                                                  });

                XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
                createInfo.action = gazeAction;
                createInfo.poseInActionSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateActionSpace(m_context.Session.Handle, &createInfo, m_gazeSpace.Put()));
            } else {
                // Use VIEW reference space to simulate eye gaze when the system doesn't support
                XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
                createInfo.poseInReferenceSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_context.Session.Handle, &createInfo, m_gazeSpace.Put()));
            }

            m_gazeObject = AddObject(engine::CreateObject());

            // Draw a small axis object 2 meters in front to visualize the orientation of gaze
            m_gazeLookAtAxis = AddObject(engine::CreateAxis(m_context.PbrResources, 0.05f));
            m_gazeLookAtAxis->Pose() = Pose::Translation({0, 0, -2});
            m_gazeLookAtAxis->SetParent(m_gazeObject);

            auto randomColor = []() -> Pbr::RGBAColor {
                constexpr float normalize = 1.0f / RAND_MAX;
                return {rand() * normalize, rand() * normalize, rand() * normalize, 1};
            };

            // Draw a circle of objects around the user, and they will rotate when user is looking at them.
            const float angleDistance = XM_2PI / numberOfObjects;
            for (int i = 0; i < numberOfObjects; i++) {
                auto object = AddObject(engine::CreateSphere(m_context.PbrResources, objectDiameter, 3, randomColor()));
                object->Pose().position.x = layoutRadius * std::sin(i * angleDistance);
                object->Pose().position.z = layoutRadius * std::cos(i * angleDistance);
                object->Motion.SetRotation({0, 0, 1}, XM_2PI); // Rotate around per second
                m_lookAtObjects.emplace_back(std::move(object));
            }
        }

        void OnUpdate(const engine::FrameTime& frameTime) override {
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_gazeSpace.Get(), m_context.AppSpace, frameTime.PredictedDisplayTime, &location));

            if (Pose::IsPoseValid(location)) {
                m_gazeObject->SetVisible(true);
                m_gazeObject->Pose() = location.pose;
                for (auto& object : m_lookAtObjects) {
                    XrPosef objectInAppSpace;
                    StoreXrPose(&objectInAppSpace, object->WorldTransform());
                    object->Motion.Enabled = IsLookingAtObject(location.pose, objectInAppSpace, objectDiameter / 2);
                }
            } else {
                m_gazeObject->SetVisible(false);
                for (auto& object : m_lookAtObjects) {
                    object->Motion.Enabled = false;
                }
            }
        }

    private:
        xr::SpaceHandle m_gazeSpace;
        std::shared_ptr<engine::Object> m_gazeObject;
        std::shared_ptr<engine::Object> m_gazeLookAtAxis;
        std::vector<std::shared_ptr<engine::Object>> m_lookAtObjects;
    };
} // namespace

std::unique_ptr<engine::Scene> TryCreateEyeGazeInteractionScene(engine::Context& context) {
    return std::make_unique<EyeGazeInteractionScene>(context);
}
