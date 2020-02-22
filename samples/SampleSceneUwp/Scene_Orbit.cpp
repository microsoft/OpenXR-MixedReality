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
#include "PbrModelObject.h"
#include "Scene.h"

using namespace DirectX;
using namespace xr::math;
using namespace std::chrono;
using namespace std::chrono_literals;

namespace {
    //
    // This sample displays a simple orbit in front of the user using local reference space
    // and shows how to pause the animation when the session lost focus while it continues rendering.
    //
    struct OrbitScene : public Scene {
        OrbitScene(SceneContext* sceneContext)
            : Scene(sceneContext, L"Orbit Scene", true) {
            XrReferenceSpaceCreateInfo createInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            createInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
            createInfo.poseInReferenceSpace = Pose::Translation({0, 0, -2}); // 2 meters in front
            CHECK_XRCMD(xrCreateReferenceSpace(m_sceneContext->Session, &createInfo, m_centerSpace.Put()));

            m_sun = AddSceneObject(MakeSphere(m_sceneContext->PbrResources, 0.5f, 20, Pbr::FromSRGB(Colors::OrangeRed)));
            m_earth = AddSceneObject(MakeSphere(m_sceneContext->PbrResources, 0.1f, 20, Pbr::FromSRGB(Colors::SeaGreen)));
            m_earth->SetParent(m_sun);
        }

        void OnUpdate(const FrameTime& frameTime) override {
            XrSpaceLocation location = {XR_TYPE_SPACE_LOCATION};
            CHECK_XRCMD(xrLocateSpace(m_centerSpace.Get(), m_sceneContext->SceneSpace, frameTime.PredictedDisplayTime, &location));
            if (Pose::IsPoseValid(location)) {
                m_sun->Pose() = location.pose;
            }

            if (m_sceneContext->SessionState == XR_SESSION_STATE_FOCUSED) {
                const float seconds = duration_cast<duration<float>>(frameTime.TotalElapsed).count();
                const float angle = seconds * XM_PI; // half circle a second

                XrVector3f earthPosition;
                earthPosition.x = 0.6f * sin(angle);
                earthPosition.y = 0.0f;
                earthPosition.z = 0.6f * cos(angle);
                m_earth->Pose().position = earthPosition;
            }
        }

    private:
        xr::SpaceHandle m_centerSpace;
        std::shared_ptr<PbrModelObject> m_sun;
        std::shared_ptr<PbrModelObject> m_earth;
    };
} // namespace

std::unique_ptr<Scene> CreateOrbitScene(SceneContext* sceneContext) {
    return std::make_unique<OrbitScene>(sceneContext);
}
