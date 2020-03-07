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

#include "SceneContext.h"
#include "SceneObject.h"
#include "QuadLayerObject.h"

#include <optional>

namespace scenes::priorities {
    constexpr uint32_t Default = 0;
    constexpr uint32_t ControllerRendering = 0;
    constexpr uint32_t Menu = 1;
} // namespace scenes::priorities

struct Scene {
    virtual ~Scene() = default;
    Scene(SceneContext* sceneContext, std::wstring sceneName, bool defaultActive);

#pragma region Scene objects will be rendered into projection layers
    template <typename T>
    std::shared_ptr<T> AddSceneObject(const std::shared_ptr<T>& sceneObject) {
        sceneObject->State = SceneObjectState::InitializePending;
        m_uninitializedSceneObjects.push_back(sceneObject);
        return sceneObject;
    }

    template <typename T>
    void RemoveSceneObject(const std::shared_ptr<T>& sceneObject) {
        sceneObject->State = SceneObjectState::RemovePending;
    }

    std::vector<std::shared_ptr<SceneObject>> GetSceneObjects() const {
        return m_sceneObjects;
    }
#pragma endregion

#pragma region Quad layer objects will be rendered into quad layers, and will not affect projection layers
    std::shared_ptr<QuadLayerObject> AddQuadLayerObject(const std::shared_ptr<QuadLayerObject>& sceneObject) {
        sceneObject->State = SceneObjectState::InitializePending;
        m_uninitializedQuadLayerObjects.push_back(sceneObject);
        return sceneObject;
    }

    std::vector<std::shared_ptr<QuadLayerObject>> GetQuadLayerObjects() const {
        return m_quadLayerObjects;
    }

    bool HasSceneObjects() const {
        return !m_sceneObjects.empty();
    }
#pragma endregion

    void Update(const FrameTime& frameTime);
    void Render(const FrameTime& frameTime) const;

    void NotifyInteractionProfileChangedEvent() {
        OnInteractionProfileChanged();
    }
    void NotifySpaceChangingEvent(XrReferenceSpaceType referenceSpaceType, XrTime changeTime, std::optional<XrPosef> pose) {
        OnSpaceChanging(referenceSpaceType, changeTime, pose);
    }
    void NotifyMenuButtonPressed() {
        OnMenuButtonPressed();
    }

    virtual DirectX::XMFLOAT4 ColorScale() const {
        return {1.0f, 1.0f, 1.0f, 1.0f};
    }

    bool IsActive() const {
        return m_isActive;
    }
    void SetActive(bool active) {
        if (m_isActive != active) {
            OnActiveChanged(active);
        }
        m_isActive = active;
    }

    bool ActiveByDefault() const {
        return m_defaultActive;
    }

    const std::wstring& Name() const {
        return m_sceneName;
    }

    const std::wstring& Description() const {
        return m_sceneDescription;
    }

protected:
    ::SceneContext* const m_sceneContext;

    virtual void OnUpdate(const FrameTime& frameTime [[maybe_unused]]) {
    }
    virtual void OnRender(const FrameTime& frameTime [[maybe_unused]]) const {
    }
    virtual void OnInteractionProfileChanged() {
    }
    virtual void OnSpaceChanging(XrReferenceSpaceType referenceSpaceType [[maybe_unused]],
                                 XrTime changeTime [[maybe_unused]],
                                 const std::optional<XrPosef>& pose [[maybe_unused]]) {
    }
    virtual void OnActiveChanged(bool toActive [[maybe_unused]]) {
    }
    virtual void OnMenuButtonPressed() {
        SetActive(!IsActive());
    }

    void SetSceneDescription(std::wstring description) {
        m_sceneDescription = std::move(description);
    }

private:
    const bool m_defaultActive{false};
    const std::wstring m_sceneName;
    std::wstring m_sceneDescription;

    std::atomic<bool> m_isActive{true};

    std::vector<std::shared_ptr<SceneObject>> m_sceneObjects;
    std::vector<std::shared_ptr<SceneObject>> m_uninitializedSceneObjects;

    std::vector<std::shared_ptr<QuadLayerObject>> m_quadLayerObjects;
    std::vector<std::shared_ptr<QuadLayerObject>> m_uninitializedQuadLayerObjects;
};
