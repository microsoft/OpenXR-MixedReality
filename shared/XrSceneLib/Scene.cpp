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
#include "Scene.h"

using namespace DirectX;

namespace {
    template <typename T>
    void AddPendingObjects(std::vector<std::shared_ptr<T>>* objects, std::vector<std::shared_ptr<T>>&& uninitializedObjects) {
        for (auto& object : uninitializedObjects) {
            if (object->State == engine::ObjectState::InitializePending) {
                object->State = engine::ObjectState::Initialized;
                objects->push_back(std::move(object));
            }
        }
    }

    template <typename T>
    void RemoveDestroyedObjects(std::vector<std::shared_ptr<T>>* objects) {
        auto newEnd = std::remove_if(
            objects->begin(), objects->end(), [](auto&& object) { return object->State == engine::ObjectState::RemovePending; });

        objects->erase(newEnd, objects->end());
    }

    template <typename T>
    void UpdateObjects(std::vector<std::shared_ptr<T>> const& objects, engine::Context& context, engine::FrameTime const& frameTime) {
        for (const auto& object : objects) {
            object->Update(context, frameTime);
        }
    }

    template <typename T>
    void RenderObjects(std::vector<std::shared_ptr<T>> const& objects, engine::Context& context, uint32_t viewIndex) {
        for (const auto& object : objects) {
            if (object->IsVisibleForViewIndex(viewIndex)) {
                object->Render(context);
            }
        }
    }
} // namespace

engine::Scene::Scene(engine::Context& context)
    : m_context(context)
    , m_actionContext(context.Instance.Handle) {
}

void engine::Scene::Update(const engine::FrameTime& frameTime) {
    std::unique_lock lk(m_uninitializedMutex);
    std::vector uninitializedObjects = std::move(m_uninitializedObjects);
    std::vector uninitializedQuadLayerObjects = std::move(m_uninitializedQuadLayerObjects);
    lk.unlock();

    AddPendingObjects(&m_objects, std::move(uninitializedObjects));
    AddPendingObjects(&m_quadLayerObjects, std::move(uninitializedQuadLayerObjects));

    RemoveDestroyedObjects(&m_objects);
    RemoveDestroyedObjects(&m_quadLayerObjects);

    UpdateObjects(m_objects, m_context, frameTime);
    UpdateObjects(m_quadLayerObjects, m_context, frameTime);

    OnUpdate(frameTime);
}

void engine::Scene::BeforeRender(const FrameTime& frameTime) {
    OnBeforeRender(frameTime);
}

void engine::Scene::Render(const FrameTime& frameTime, uint32_t viewIndex) {
    RenderObjects(m_objects, m_context, viewIndex);
    RenderObjects(m_quadLayerObjects, m_context, viewIndex);
}
