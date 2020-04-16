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
#include "ObjectMotion.h"

enum class SceneObjectState { InitializePending, Initialized, RemovePending };

class SceneObject {
public:
    virtual ~SceneObject() = default;
    SceneObjectState State;
    Motion Motion;

public:
    void SetParent(std::shared_ptr<SceneObject> parent) {
        m_parent = std::move(parent);
    }

    void SetVisible(bool visible) {
        m_isVisible = visible;
    }
    bool IsVisible() const {
        return State == SceneObjectState::Initialized && m_isVisible && (m_parent ? m_parent->IsVisible() : true);
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
    bool m_isVisible{true};

    XrPosef m_pose = xr::math::Pose::Identity();
    XrVector3f m_scale = {1, 1, 1};

    std::shared_ptr<SceneObject> m_parent;

    // Local transform is relative to parent
    // Only recompute when transform is changed.
    mutable DirectX::XMFLOAT4X4 m_localTransform;
    mutable bool m_localTransformDirty{true};
};

inline std::shared_ptr<SceneObject> CreateSceneObject() {
    return std::make_shared<SceneObject>();
}
