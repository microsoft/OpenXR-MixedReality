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
#include "ObjectMotion.h"

using engine::Motion;

void Motion::SetGravity(float gravitationalAcceleration) {
    LinearAcceleration = {0, -gravitationalAcceleration, 0};
}

void Motion::SetRotation(const XrVector3f& axis, float radiansPerSecond) {
    AngularVelocity = {axis.x * radiansPerSecond, axis.y * radiansPerSecond, axis.z * radiansPerSecond};
}

void Motion::SetVelocity(const XrSpaceVelocity& velocity) {
    LinearVelocity = (velocity.velocityFlags & XR_SPACE_VELOCITY_LINEAR_VALID_BIT) ? xr::math::cast(velocity.linearVelocity)
                                                                                   : DirectX::XMFLOAT3{0, 0, 0};
    AngularVelocity = (velocity.velocityFlags & XR_SPACE_VELOCITY_ANGULAR_VALID_BIT) ? xr::math::cast(velocity.angularVelocity)
                                                                                     : DirectX::XMFLOAT3{0, 0, 0};
}

void Motion::UpdateMotionAndPose(XrPosef& pose, std::chrono::duration<float> durationInSeconds) {
    using namespace DirectX;

    if (!Enabled) {
        return;
    }

    const float dt = durationInSeconds.count();

    const auto position = xr::math::LoadXrVector3(pose.position);
    const auto orientation = xr::math::LoadXrQuaternion(pose.orientation);
    const auto linearVelocity = XMLoadFloat3(&LinearVelocity);
    const auto angularVelocity = XMLoadFloat3(&AngularVelocity);
    const auto linearAcceleration = XMLoadFloat3(&LinearAcceleration);
    const auto angularAcceleration = XMLoadFloat3(&AngularAcceleration);

    XMStoreFloat3(&LinearVelocity, linearVelocity + XMVectorScale(linearAcceleration, dt));
    XMStoreFloat3(&AngularVelocity, angularVelocity + XMVectorScale(angularAcceleration, dt));

    xr::math::StoreXrVector3(&pose.position, position + XMVectorScale(linearVelocity, dt));

    // Convert angularVelocity from object space to world space
    auto angularVelocityInWorld = XMVector3Rotate(angularVelocity, XMQuaternionInverse(orientation));

    auto angle = XMVectorGetX(XMVector3Length(angularVelocityInWorld));
    if (angle > 0.0f) {
        xr::math::StoreXrQuaternion(&pose.orientation,
                                    XMQuaternionMultiply(XMQuaternionRotationAxis(angularVelocityInWorld, angle * dt), orientation));
    }
}
