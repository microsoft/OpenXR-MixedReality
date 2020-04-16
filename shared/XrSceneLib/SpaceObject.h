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

#include "SceneObject.h"

class SpaceObject : public SceneObject {
public:
    SpaceObject(SceneContext& sceneContext, std::unique_ptr<xr::SpaceHandle> space, bool hideWhenPoseInvalid = true);

    void Update(const FrameTime& frameTime) override;

private:
    SceneContext& m_sceneContext;
    std::unique_ptr<xr::SpaceHandle> m_space;
    const bool m_hideWhenPoseInvalid;
};
