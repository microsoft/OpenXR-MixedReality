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

namespace {
    //
    // This sample displays a simple orbit in front of the user using local reference space
    // and shows how to pause the animation when the session lost focus while it continues rendering.
    // Also demos Gaze-Select interaction, that the user can air tap to move the orbit in front of the user.
    //
    struct OrbitScene : public Scene {
        OrbitScene(SceneContext& sceneContext)
            : Scene(sceneContext) {
            xr::ActionSet& actionSet = ActionContext().CreateActionSet("orbit_scene_actions", "Orbit Scene Actions");

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

            if (sceneContext.Extensions.SupportsHandInteraction) {
                ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/hand_interaction",
                                                                  {
                                                                      {m_selectAction, "/user/hand/right/input/select"},
                                                                      {m_selectAction, "/user/hand/left/input/select"},
                                                                  });
            }

            m_sun = AddSceneObject(CreateSphere(m_sceneContext.PbrResources, 0.5f, 20, Pbr::FromSRGB(Colors::OrangeRed)));
            m_sun->SetVisible(false); // invisible until tracking is valid and placement succeeded.

            m_earth = AddSceneObject(CreateSphere(m_sceneContext.PbrResources, 0.1f, 20, Pbr::FromSRGB(Colors::SeaGreen)));
            m_earth->SetParent(m_sun);

            XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            createInfo.poseInReferenceSpace = Pose::Identity();
            CHECK_XRCMD(xrCreateReferenceSpace(m_sceneContext.Session, &createInfo, m_viewSpace.Put()));
        }

        void OnUpdate(const FrameTime& frameTime) override {
            XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
            XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            getInfo.action = m_selectAction;
            CHECK_XRCMD(xrGetActionStateBoolean(m_sceneContext.Session, &getInfo, &state));
            const bool isSelectPressed = state.isActive && state.changedSinceLastSync && state.currentState;
            const bool firstUpdate = !m_sun->IsVisible();

            if (firstUpdate || isSelectPressed) {
                const XrTime time = state.isActive ? state.lastChangeTime : frameTime.PredictedDisplayTime;
                XrSpaceLocation viewInScene = {XR_TYPE_SPACE_LOCATION};
                CHECK_XRCMD(xrLocateSpace(m_viewSpace.Get(), m_sceneContext.SceneSpace, time, &viewInScene));

                if (Pose::IsPoseValid(viewInScene)) {
                    // Project the forward of the view to the scene's horizontal plane
                    const XrPosef viewFrontInView = {{0, 0, 0, 1}, {0, 0, -1}};
                    const XrPosef viewFrontInScene = viewFrontInView * viewInScene.pose;
                    const XrVector3f viewForwardInScene = viewFrontInScene.position - viewInScene.pose.position;
                    const XrVector3f viewForwardInGravity = Dot(viewForwardInScene, {0, -1, 0}) * XrVector3f{0, -1, 0};
                    const XrVector3f userForwardInScene = Normalize(viewForwardInScene - viewForwardInGravity);

                    // Put the sun 2 meters in front of the user at eye level
                    const XrVector3f sunInScene = viewInScene.pose.position + 2.f * userForwardInScene;
                    m_targetPoseInScene = Pose::LookAt(sunInScene, userForwardInScene, {0, 1, 0});

                    if (firstUpdate) {
                        m_sun->SetVisible(true);
                        m_sun->Pose() = m_targetPoseInScene;
                    }
                }
            }

            // Slowly ease the sun to the target location
            m_sun->Pose() = Pose::Slerp(m_sun->Pose(), m_targetPoseInScene, 0.05f);

            // Animate the earth orbiting the sun, and pause when app lost focus.
            if (m_sceneContext.SessionState == XR_SESSION_STATE_FOCUSED) {
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
        XrPosef m_targetPoseInScene = Pose::Identity();
        std::shared_ptr<PbrModelObject> m_sun;
        std::shared_ptr<PbrModelObject> m_earth;
        xr::SpaceHandle m_viewSpace;
    };
} // namespace

std::unique_ptr<Scene> TryCreateOrbitScene(SceneContext& sceneContext) {
    return std::make_unique<OrbitScene>(sceneContext);
}
