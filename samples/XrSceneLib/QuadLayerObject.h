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
#include "SceneContext.h"

enum class LayerGrouping {
    Underlay, // Behind all projection layers
    Overlay   // In front of all projection layers
};

struct QuadLayerObject : public SceneObject {
    XrSwapchainSubImage Image;

    XrSpace Space{XR_NULL_HANDLE};
    XrCompositionLayerFlags CompositionLayerFlags{};
    XrEyeVisibility EyeVisibility{XR_EYE_VISIBILITY_BOTH};
    LayerGrouping LayerGroup = LayerGrouping::Overlay;
};

std::shared_ptr<QuadLayerObject> MakeQuadLayerObject(XrSpace space, XrSwapchainSubImage image);

