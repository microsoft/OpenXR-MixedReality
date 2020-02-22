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
    ApplyRigidbodyPhysics(frameTime.Elapsed);
}

void SceneObject::Render(SceneContext& sceneContext) const {
}

void SceneObject::ApplyRigidbodyPhysics(std::chrono::duration<float> duration) {
    using namespace DirectX;

    if (!Physics.Enabled) {
        return;
    }

    const float dt = duration.count();

    const auto position = xr::math::LoadXrVector3(m_pose.position);
    const auto orientation = xr::math::LoadXrQuaternion(m_pose.orientation);
    const auto linearVelocity = XMLoadFloat3(&Physics.LinearVelocity);
    const auto angularVelocity = XMLoadFloat3(&Physics.AngularVelocity);
    const auto linearAcceleration = XMLoadFloat3(&Physics.LinearAcceleration);
    const auto angularAcceleration = XMLoadFloat3(&Physics.AngularAcceleration);

    XMStoreFloat3(&Physics.LinearVelocity, linearVelocity + XMVectorScale(linearAcceleration, dt));
    XMStoreFloat3(&Physics.AngularVelocity, angularVelocity + XMVectorScale(angularAcceleration, dt));

    xr::math::StoreXrVector3(&m_pose.position, position + XMVectorScale(linearVelocity, dt));

    // To implicit app space
    auto adjustedAngularVelocity = XMVector3Rotate(angularVelocity, XMQuaternionInverse(orientation));

    auto angle = XMVectorGetX(XMVector3Length(adjustedAngularVelocity));
    if (angle > 0.0f) {
        xr::math::StoreXrQuaternion(&m_pose.orientation,
                                    XMQuaternionMultiply(XMQuaternionRotationAxis(adjustedAngularVelocity, angle * dt), orientation));
    }
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
