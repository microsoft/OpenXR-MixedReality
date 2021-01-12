// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "Object.h"
#include "Context.h"

namespace engine {

    enum class LayerGrouping {
        Underlay, // Behind all projection layers
        Overlay   // In front of all projection layers
    };

    struct QuadLayerObject : public Object {
        XrSwapchainSubImage Image;

        XrSpace Space{XR_NULL_HANDLE};
        XrCompositionLayerFlags CompositionLayerFlags{};
        XrEyeVisibility EyeVisibility{XR_EYE_VISIBILITY_BOTH};
        LayerGrouping LayerGroup = LayerGrouping::Overlay;
    };

    std::shared_ptr<QuadLayerObject> CreateQuadLayerObject(XrSpace space, XrSwapchainSubImage image);

} // namespace engine

