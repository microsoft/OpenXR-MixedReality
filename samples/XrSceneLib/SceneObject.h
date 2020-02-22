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

#include <XrUtility/XrMath.h>
#include "SceneContext.h"
#include "FrameTime.h"

enum class SceneObjectState { InitializePending, Initialized, RemovePending };

struct PhysicsData {
    bool Enabled{false};
    DirectX::XMFLOAT3 LinearVelocity{0, 0, 0};
    DirectX::XMFLOAT3 LinearAcceleration{0, -9.8f, 0};
    DirectX::XMFLOAT3 AngularVelocity{0, 0, 0};
    DirectX::XMFLOAT3 AngularAcceleration{0, 0, 0};
};

class SceneObject {
public:
    virtual ~SceneObject() = default;
    SceneObjectState State;
    PhysicsData Physics;

public:
    void SetParent(std::shared_ptr<SceneObject> parent) {
        m_parent = std::move(parent);
    }

    void SetVisible(bool visible) {
        m_isVisible = visible;
    }
    bool IsVisible() const {
        return (m_parent ? m_parent->IsVisible() : true) && m_isVisible;
    }

    bool IsRenderable() const {
        return State == SceneObjectState::Initialized && IsVisible();
    }

    const XrPosef& Pose() const {
        return m_pose;
    }
    XrPosef& Pose() {
        m_localTransformDirty = true;
        return m_pose;
    }

    const XrVector3f& Scale() const {
        return m_scale;
    }
    XrVector3f& Scale() {
        m_localTransformDirty = true;
        return m_scale;
    }

    DirectX::XMMATRIX LocalTransform() const;
    DirectX::XMMATRIX WorldTransform() const;

    virtual void Update(const FrameTime& frameTime);
    virtual void Render(SceneContext& sceneContext) const;

private:
    void ApplyRigidbodyPhysics(std::chrono::duration<float> duration);

private:
    bool m_isVisible{true};

    XrPosef m_pose = xr::math::Pose::Identity();
    XrVector3f m_scale = {1, 1, 1};

    std::shared_ptr<SceneObject> m_parent;

    // Local transform is relative to parent
    // Only recompute when transform is changed.
    mutable DirectX::XMFLOAT4X4 m_localTransform;
    mutable bool m_localTransformDirty{true};
};
