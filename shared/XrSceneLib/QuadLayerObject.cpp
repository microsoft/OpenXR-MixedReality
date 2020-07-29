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
#include "QuadLayerObject.h"
#include "CompositionLayers.h"

using namespace DirectX;

std::shared_ptr<engine::QuadLayerObject> engine::CreateQuadLayerObject(XrSpace space, XrSwapchainSubImage image) {
    auto result = std::make_shared<engine::QuadLayerObject>();
    result->Image = std::move(image);
    result->Space = space;
    return result;
}

void engine::AppendQuadLayer(engine::CompositionLayers& layers, engine::QuadLayerObject* quad) {
    XrCompositionLayerQuad& quadLayer = layers.AddQuadLayer();
    quadLayer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
    quadLayer.subImage = quad->Image;
    quadLayer.space = quad->Space;
    quadLayer.layerFlags = quad->CompositionLayerFlags;
    quadLayer.eyeVisibility = quad->EyeVisibility;


    XMVECTOR scale, position, orientation;
    if (!DirectX::XMMatrixDecompose(&scale, &orientation, &position, quad->WorldTransform())) {
        throw std::runtime_error("Failed to decompose quad layer world transform");
    }

    xr::math::StoreXrQuaternion(&quadLayer.pose.orientation, orientation);
    xr::math::StoreXrVector3(&quadLayer.pose.position, position);

    xr::math::StoreXrExtent(&quadLayer.size, scale); // Use x and y but ignore z.
};

