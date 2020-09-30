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

#include "Object.h"

namespace engine {

    class SpaceObject : public Object {
    public:
        SpaceObject(xr::SpaceHandle space, bool hideWhenPoseInvalid = true);

        void Update(engine::Context& context, const engine::FrameTime& frameTime) override;

    private:
        xr::SpaceHandle m_space;
        const bool m_hideWhenPoseInvalid;
    };
} // namespace engine
