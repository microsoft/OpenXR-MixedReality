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
            if (object->State == SceneObjectState::InitializePending) {
                object->State = SceneObjectState::Initialized;
                objects->push_back(std::move(object));
            }
        }
    }

    template <typename T>
    void RemoveDestroyedObjects(std::vector<std::shared_ptr<T>>* objects) {
        auto newEnd = std::remove_if(
            objects->begin(), objects->end(), [](auto&& object) { return object->State == SceneObjectState::RemovePending; });

        objects->erase(newEnd, objects->end());
    }

    template <typename T>
    void UpdateObjects(std::vector<std::shared_ptr<T>> const& objects, FrameTime const& frameTime) {
        for (const auto& object : objects) {
            object->Update(frameTime);
        }
    }

    template <typename T>
    void RenderObjects(std::vector<std::shared_ptr<T>> const& objects, SceneContext& sceneContext) {
        for (const auto& object : objects) {
            object->Render(sceneContext);
        }
    }
} // namespace

Scene::Scene(SceneContext& sceneContext)
    : m_sceneContext(sceneContext)
    , m_actionContext(sceneContext.Instance.Handle) {
}

void Scene::Update(const FrameTime& frameTime) {
    std::unique_lock lk(m_uninitializedMutex);
    std::vector uninitializedSceneObjects = std::move(m_uninitializedSceneObjects);
    std::vector uninitializedQuadLayerObjects = std::move(m_uninitializedQuadLayerObjects);
    lk.unlock();
    AddPendingObjects(&m_sceneObjects, std::move(uninitializedSceneObjects));
    AddPendingObjects(&m_quadLayerObjects, std::move(uninitializedQuadLayerObjects));

    RemoveDestroyedObjects(&m_sceneObjects);
    RemoveDestroyedObjects(&m_quadLayerObjects);

    UpdateObjects(m_sceneObjects, frameTime);
    UpdateObjects(m_quadLayerObjects, frameTime);

    OnUpdate(frameTime);
}

void Scene::Render(const FrameTime& frameTime) {
    RenderObjects(m_sceneObjects, m_sceneContext);
    RenderObjects(m_quadLayerObjects, m_sceneContext);

    OnRender(frameTime);
}
