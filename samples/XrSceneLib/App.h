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

#include "Scene.h"
#include "SceneContext.h"
#include "ProjectionLayer.h"

class XrApp {
public:
    virtual ~XrApp() = default;

    virtual SceneContext* SceneContext() const = 0;

    virtual void AddScene(std::unique_ptr<Scene> scene) = 0;
    virtual const std::vector<std::unique_ptr<Scene>>& Scenes() const = 0;

    virtual void Run() = 0;
    virtual void Stop() = 0;

    virtual ProjectionLayers& ProjectionLayers() = 0;

};

std::unique_ptr<XrApp> CreateXrApp(const xr::NameVersion& appInfo,
                                   const std::vector<const char*>& extensions,
                                   bool singleThreadedGraphics = false);

