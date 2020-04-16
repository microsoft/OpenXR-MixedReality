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

    bool IsLookingAtObject(const XrPosef& gazeInScene, const XrPosef& objectInScene, float objectRadius) {
        const XrPosef sceneInGaze = Pose::Invert(gazeInScene);
        const XrPosef objectInGaze = objectInScene * sceneInGaze;
        const float projectionOnZ = Normalize(objectInGaze.position).z;
        const float distance = Length(objectInGaze.position);
        const float angleTolerance = std::atan2(objectRadius, distance);
        return projectionOnZ < -std::cos(angleTolerance);
    }

    struct EyeGazeInteractionScene : public Scene {
        EyeGazeInteractionScene(SceneContext& sceneContext)
            : Scene(sceneContext) {
            const bool supportsEyeGazeAction = sceneContext.Extensions.SupportsEyeGazeInteraction &&
                                               sceneContext.System.EyeGazeInteractionProperties.supportsEyeGazeInteraction;
            if (supportsEyeGazeAction) {
                xr::ActionSet& actionSet =
                    ActionContext().CreateActionSet("eye_gaze_interaction_scene_actions", "Eye Gaze Interaction Scene Actions");

                auto gazeAction = actionSet.CreateAction("gaze_action", "Gaze Action", XR_ACTION_TYPE_POSE_INPUT, {});

                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/ext/eye_gaze_interaction",
                                                                  {
                                                                      {gazeAction, "/user/eyes_ext/input/gaze_ext/pose"},
                                                                  });

                XrActionSpaceCreateInfo createInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
                createInfo.action = gazeAction;
                createInfo.poseInActionSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateActionSpace(m_sceneContext.Session, &createInfo, m_gazeSpace.Put()));
            } else {
                // Use VIEW reference space to simulate eye gaze when the system doesn't support
                XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
                createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
                createInfo.poseInReferenceSpace = Pose::Identity();
                CHECK_XRCMD(xrCreateReferenceSpace(m_sceneContext.Session, &createInfo, m_gazeSpace.Put()));
            }

            m_gazeObject = AddSceneObject(CreateSceneObject());

            // Draw a small axis object 2 meters in front to visualize the orientation of gaze
            m_gazeLookAtAxis = AddSceneObject(CreateAxis(m_sceneContext.PbrResources, 0.05f));
            m_gazeLookAtAxis->Pose() = Pose::Translation({0, 0, -2});
            m_gazeLookAtAxis->SetParent(m_gazeObject);

            auto randomColor = []() -> Pbr::RGBAColor {
                constexpr float normalize = 1.0f / RAND_MAX;
                return {rand() * normalize, rand() * normalize, rand() * normalize, 1};
            };

            // Draw a circle of objects around the user, and they will rotate when user is looking at them.
            const float angleDistance = XM_2PI / numberOfObjects;
            for (int i = 0; i < numberOfObjects; i++) {
                auto object = AddSceneObject(CreateSphere(m_sceneContext.PbrResources, objectDiameter, 3, randomColor()));
                object->Pose().position.x = layoutRadius * std::sin(i * angleDistance);
                object->Pose().position.z = layoutRadius * std::cos(i * angleDistance);
                object->Motion.SetRotation({0, 0, 1}, XM_2PI); // Rotate around per second
                m_lookAtObjects.emplace_back(std::move(object));
            }
        }

        void OnUpdate(const FrameTime& frameTime) override {
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_gazeSpace.Get(), m_sceneContext.SceneSpace, frameTime.PredictedDisplayTime, &location));

            if (Pose::IsPoseValid(location)) {
                m_gazeObject->SetVisible(true);
                m_gazeObject->Pose() = location.pose;
                for (auto& object : m_lookAtObjects) {
                    XrPosef objectInScene;
                    StoreXrPose(&objectInScene, object->WorldTransform());
                    object->Motion.Enabled = IsLookingAtObject(location.pose, objectInScene, objectDiameter / 2);
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
        std::shared_ptr<SceneObject> m_gazeObject;
        std::shared_ptr<SceneObject> m_gazeLookAtAxis;
        std::vector<std::shared_ptr<SceneObject>> m_lookAtObjects;
    };
} // namespace

std::unique_ptr<Scene> TryCreateEyeGazeInteractionScene(SceneContext& sceneContext) {
    return std::make_unique<EyeGazeInteractionScene>(sceneContext);
}
