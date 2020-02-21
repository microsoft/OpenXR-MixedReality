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

struct IVisibilityMask {
    virtual ~IVisibilityMask() = default;

    virtual bool RenderMask(uint32_t viewIndex, DirectX::FXMMATRIX viewToProj) const = 0;
    virtual void NotifyMaskChanged(uint32_t viewIndex) = 0;
};

std::unique_ptr<IVisibilityMask> CreateStereoVisibilityMask(SceneContext* sceneContext);
