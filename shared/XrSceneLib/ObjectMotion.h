// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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
