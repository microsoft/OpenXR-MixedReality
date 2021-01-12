// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

