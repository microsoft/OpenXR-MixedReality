// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

namespace engine {
    struct QuadLayerObject;
    class ProjectionLayer;
    class CompositionLayers;

    void AppendQuadLayer(CompositionLayers& layers, QuadLayerObject* quad);
    void AppendProjectionLayer(CompositionLayers& layers, ProjectionLayer* layer, XrViewConfigurationType type);

    class CompositionLayers {
    public:

        XrCompositionLayerQuad& AddQuadLayer() {
            XrCompositionLayerQuad& quadLayer = m_quadLayers.emplace_back();
            quadLayer.type = XR_TYPE_COMPOSITION_LAYER_QUAD;
            m_compositionLayers.push_back(reinterpret_cast<const XrCompositionLayerBaseHeader*>(&quadLayer));
            return m_quadLayers.back();
        }

        XrCompositionLayerProjection& AddProjectionLayer(XrCompositionLayerFlags layerFlags) {
            XrCompositionLayerProjection& projectionLayer = m_projectionLayers.emplace_back();
            projectionLayer.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION;
            projectionLayer.layerFlags = layerFlags;
            m_compositionLayers.push_back(reinterpret_cast<const XrCompositionLayerBaseHeader*>(&projectionLayer));
            return m_projectionLayers.back();
        }

        uint32_t LayerCount() const {
            return (uint32_t)m_compositionLayers.size();
        }

        const XrCompositionLayerBaseHeader* const* LayerData() const {
            return m_compositionLayers.data();
        }

    private:
        std::list<XrCompositionLayerQuad> m_quadLayers;
        std::list<XrCompositionLayerProjection> m_projectionLayers;
        std::vector<XrCompositionLayerBaseHeader const*> m_compositionLayers;
    };
} // namespace engine

