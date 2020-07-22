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

#pragma once

namespace engine {

    struct Motion {
        bool Enabled{false};
        DirectX::XMFLOAT3 LinearVelocity{};
        DirectX::XMFLOAT3 LinearAcceleration{};
        DirectX::XMFLOAT3 AngularVelocity{};
        DirectX::XMFLOAT3 AngularAcceleration{};

        void SetGravity(float gravitationalAcceleration = 9.8f);
        void SetVelocity(const XrSpaceVelocity& velocity);
        void SetRotation(const XrVector3f& axis, float radiansPerSecond);
        void UpdateMotionAndPose(XrPosef& pose, std::chrono::duration<float> durationInSeconds);
    };
} // namespace engine
