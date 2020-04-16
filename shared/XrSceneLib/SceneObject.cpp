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
#include "SceneObject.h"

void SceneObject::Update(const FrameTime& frameTime) {
    Motion.UpdateMotionAndPose(Pose(), frameTime.Elapsed);
}

void SceneObject::Render(SceneContext& sceneContext) const {
}

DirectX::XMMATRIX SceneObject::LocalTransform() const {
    if (!m_localTransformDirty) {
        return DirectX::XMLoadFloat4x4(&m_localTransform);
    }

    const DirectX::XMMATRIX modelScale = DirectX::XMMatrixScalingFromVector(xr::math::LoadXrVector3(m_scale));
    const DirectX::XMMATRIX modelRotation = DirectX::XMMatrixRotationQuaternion(xr::math::LoadXrQuaternion(m_pose.orientation));
    const DirectX::XMMATRIX modelTranslation = DirectX::XMMatrixTranslationFromVector(xr::math::LoadXrVector3(m_pose.position));
    const DirectX::XMMATRIX localTransform = modelScale * modelRotation * modelTranslation;

    DirectX::XMStoreFloat4x4(&m_localTransform, localTransform);
    m_localTransformDirty = false;

    return localTransform;
}

DirectX::XMMATRIX SceneObject::WorldTransform() const {
    return m_parent ? XMMatrixMultiply(LocalTransform(), m_parent->WorldTransform()) : LocalTransform();
}
