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
#include <Scene.h>
#include <ControllerObject.h>

using namespace DirectX;
using namespace std::literals::chrono_literals;

namespace {
    struct ControllerModeScene : Scene {
        ControllerModeScene(SceneContext& sceneContext)
            : Scene(sceneContext) {
            xr::ActionSet& actionSet = ActionContext().CreateActionSet("controller_model_action_set", "Controller Model Action Set");

            const std::vector<std::string> subactionPathBothHands = {"/user/hand/right", "/user/hand/left"};
            XrAction gripPoseAction = actionSet.CreateAction("grip_pose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT, subactionPathBothHands);
            ActionContext().SuggestInteractionProfileBindings("/interaction_profiles/microsoft/motion_controller",
                                                              {
                                                                  {gripPoseAction, "/user/hand/right/input/grip"},
                                                                  {gripPoseAction, "/user/hand/left/input/grip"},
                                                              });

            // Controller objects are created with empty model.  It will be loaded when available.
            m_leftController = AddSceneObject(CreateControllerObject(m_sceneContext, gripPoseAction, m_sceneContext.LeftHand));
            m_rightController = AddSceneObject(CreateControllerObject(m_sceneContext, gripPoseAction, m_sceneContext.RightHand));
        }

    private:
        std::shared_ptr<SceneObject> m_leftController, m_rightController;
    };

} // namespace

std::unique_ptr<Scene> TryCreateControllerModelScene(SceneContext& sceneContext) {
    return sceneContext.Extensions.SupportsControllerModel ? std::make_unique<ControllerModeScene>(sceneContext) : nullptr;
}
