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
#include "Context.h"
#include "Object.h"
#include "QuadLayerObject.h"

namespace engine {

    struct Scene {
        virtual ~Scene() = default;
        explicit Scene(Context& context);

        Scene() = delete;
        Scene(Scene&&) = delete;
        Scene(const Scene&) = delete;

        void Update(const FrameTime& frameTime);
        void BeforeRender(const FrameTime& frameTime);
        void Render(const FrameTime& frameTime, uint32_t viewIndex);

        // Active is true when the scene participates update and render loop.
        bool IsActive() const {
            return m_isActive;
        }
        void SetActive(bool active) {
            if (m_isActive != active) {
                m_isActive = active;
                OnActiveChanged();
            }
        }
        void ToggleActive() {
            SetActive(!IsActive());
        }

        void NotifyEvent(const XrEventDataBuffer& eventData) {
            OnEvent(eventData);
        }

#pragma region Scene objects will be rendered into projection layers
        template <typename T>
        std::shared_ptr<T> AddObject(const std::shared_ptr<T>& object) {
            object->State = ObjectState::InitializePending;
            std::lock_guard guard(m_uninitializedMutex);
            m_uninitializedObjects.push_back(object);
            return object;
        }

        template <typename T>
        void RemoveObject(const std::shared_ptr<T>& object) {
            if (object) {
                object->State = ObjectState::RemovePending;
            }
        }

        const std::vector<std::shared_ptr<Object>>& GetObjects() const {
            return m_objects;
        }

#pragma endregion

#pragma region Quad layer objects will be rendered into quad layers, and will not affect projection layers
        std::shared_ptr<QuadLayerObject> AddQuadLayerObject(const std::shared_ptr<QuadLayerObject>& object) {
            object->State = ObjectState::InitializePending;
            std::lock_guard guard(m_uninitializedMutex);
            m_uninitializedQuadLayerObjects.push_back(object);
            return object;
        }

        const std::vector<std::shared_ptr<QuadLayerObject>>& GetQuadLayerObjects() const {
            return m_quadLayerObjects;
        }
#pragma endregion

        xr::ActionContext& ActionContext() {
            return m_actionContext;
        }

    protected:
        engine::Context& m_context;

        virtual void OnUpdate(const engine::FrameTime& frameTime [[maybe_unused]]) {
        }
        virtual void OnBeforeRender(const engine::FrameTime& frameTime [[maybe_unused]]) const {
        }
        virtual void OnEvent(const XrEventDataBuffer& eventData [[maybe_unused]]) {
        }
        virtual void OnActiveChanged() {
        }

    private:
        xr::ActionContext m_actionContext;

        std::atomic<bool> m_isActive{true};

        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::shared_ptr<QuadLayerObject>> m_quadLayerObjects;

        mutable std::mutex m_uninitializedMutex;
        std::vector<std::shared_ptr<Object>> m_uninitializedObjects;
        std::vector<std::shared_ptr<QuadLayerObject>> m_uninitializedQuadLayerObjects;
    };

} // namespace engine
