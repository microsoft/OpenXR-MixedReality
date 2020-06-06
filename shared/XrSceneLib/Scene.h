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

#include <mutex>
#include <XrUtility/XrActionContext.h>

#include "FrameTime.h"
#include "SceneContext.h"
#include "SceneObject.h"
#include "QuadLayerObject.h"

struct Scene {
    virtual ~Scene() = default;
    explicit Scene(SceneContext& sceneContext);

    void Update(const FrameTime& frameTime);
    void Render(const FrameTime& frameTime);

    // Active is true when the scene participates update and render loop.
    bool IsActive() const {
        return m_isActive;
    }
    void SetActive(bool active) {
        m_isActive = active;
    }
    void ToggleActive() {
        SetActive(!IsActive());
    }

    void NotifyEvent(const XrEventDataBuffer& eventData) {
        OnEvent(eventData);
    }

#pragma region Scene objects will be rendered into projection layers
    template <typename T>
    std::shared_ptr<T> AddSceneObject(const std::shared_ptr<T>& sceneObject) {
        sceneObject->State = SceneObjectState::InitializePending;
        std::lock_guard guard(m_uninitializedMutex);
        m_uninitializedSceneObjects.push_back(sceneObject);
        return sceneObject;
    }

    template <typename T>
    void RemoveSceneObject(const std::shared_ptr<T>& sceneObject) {
        sceneObject->State = SceneObjectState::RemovePending;
    }

    const std::vector<std::shared_ptr<SceneObject>>& GetSceneObjects() const {
        return m_sceneObjects;
    }

#pragma endregion

#pragma region Quad layer objects will be rendered into quad layers, and will not affect projection layers
    std::shared_ptr<QuadLayerObject> AddQuadLayerObject(const std::shared_ptr<QuadLayerObject>& sceneObject) {
        sceneObject->State = SceneObjectState::InitializePending;
        std::lock_guard guard(m_uninitializedMutex);
        m_uninitializedQuadLayerObjects.push_back(sceneObject);
        return sceneObject;
    }

    const std::vector<std::shared_ptr<QuadLayerObject>>& GetQuadLayerObjects() const {
        return m_quadLayerObjects;
    }
#pragma endregion

    xr::ActionContext& ActionContext() {
        return m_actionContext;
    }

protected:
    SceneContext& m_sceneContext;

    virtual void OnUpdate(const FrameTime& frameTime [[maybe_unused]]) {
    }
    virtual void OnRender(const FrameTime& frameTime [[maybe_unused]]) const {
    }
    virtual void OnEvent(const XrEventDataBuffer& eventData [[maybe_unused]]) {
    }

private:
    xr::ActionContext m_actionContext;

    std::atomic<bool> m_isActive{true};

    std::vector<std::shared_ptr<SceneObject>> m_sceneObjects;
    std::vector<std::shared_ptr<QuadLayerObject>> m_quadLayerObjects;

    mutable std::mutex m_uninitializedMutex;
    std::vector<std::shared_ptr<SceneObject>> m_uninitializedSceneObjects;
    std::vector<std::shared_ptr<QuadLayerObject>> m_uninitializedQuadLayerObjects;
};
