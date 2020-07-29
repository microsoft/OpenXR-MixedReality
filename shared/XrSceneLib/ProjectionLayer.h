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

#include <functional>
#include <XrUtility/XrStereoView.h>
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrMath.h>
#include <SampleShared/DxUtility.h>
#include "Context.h"
#include "FrameTime.h"

namespace engine {

    struct ProjectionLayerConfig {
        XrCompositionLayerFlags LayerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        DXGI_FORMAT ColorSwapchainFormat{};
        DXGI_FORMAT DepthSwapchainFormat{};
        xr::math::NearFar NearFar = {1000, 0.02f};
        XrExtent2Df SwapchainSizeScale = {1, 1}; // SwapchainSizeScale is applied to recommended image rect
        XrExtent2Df SwapchainFovScale = {1, 1};  // SwapchainFovScale is applied to recommended view fov
        XrExtent2Df ViewportSizeScale = {1, 1};  // ViewportSizeScale is applied after SwapchainSizeScale applied
        XrOffset2Di ViewportOffset = {0, 0};     // ViewportOffset is relative to (0, 0) of the swapchain
        uint32_t SwapchainSampleCount = 1;
        bool DoubleWideMode = false;
        bool SubmitDepthInfo = true;
        bool ContentProtected = false;
        bool ForceReset = false;
        DirectX::XMFLOAT4 ClearColor = {0, 0, 0, 0}; // Transparent
    };

    struct Scene;

    class ProjectionLayer {
    public:
        explicit ProjectionLayer(const xr::SessionContext& sessionContext);
        ProjectionLayer(ProjectionLayer&&) = default;
        ProjectionLayer(const ProjectionLayer&) = delete;

        ProjectionLayerConfig& Config(std::optional<XrViewConfigurationType> viewConfig = std::nullopt) {
            return m_viewConfigComponents.at(viewConfig.value_or(m_defaultViewConfigurationType)).PendingConfig;
        }

        const ProjectionLayerConfig& Config(std::optional<XrViewConfigurationType> viewConfig = std::nullopt) const {
            return m_viewConfigComponents.at(viewConfig.value_or(m_defaultViewConfigurationType)).PendingConfig;
        }

        const std::vector<XrCompositionLayerProjectionView>&
        ProjectionViews(std::optional<XrViewConfigurationType> viewConfig = std::nullopt) const {
            return m_viewConfigComponents.at(viewConfig.value_or(m_defaultViewConfigurationType)).ProjectionViews;
        }

        const XrSpace LayerSpace(std::optional<XrViewConfigurationType> viewConfig = std::nullopt) const {
            return m_viewConfigComponents.at(viewConfig.value_or(m_defaultViewConfigurationType)).LayerSpace;
        }

        void PrepareRendering(const Context& context,
                              XrViewConfigurationType viewConfigType,
                              const std::vector<XrViewConfigurationView>& viewConfigViews);

        bool Render(Context& context,
                    const engine::FrameTime& frameTime,
                    XrSpace layerSpace,
                    const std::vector<XrView>& Views,
                    const std::vector<std::unique_ptr<Scene>>& activeScenes,
                    XrViewConfigurationType viewConfig);

    private:
        struct ViewConfigComponent {
            ProjectionLayerConfig CurrentConfig;
            ProjectionLayerConfig PendingConfig;

            XrSpace LayerSpace{XR_NULL_HANDLE};
            std::vector<XrCompositionLayerProjectionView> ProjectionViews; // Pre-allocated and reused for each frame.
            std::vector<XrCompositionLayerDepthInfoKHR> DepthInfo;         // Pre-allocated and reused for each frame.
            std::vector<D3D11_VIEWPORT> Viewports;

            XrRect2Di LayerColorImageRect[xr::StereoView::Count];
            XrRect2Di LayerDepthImageRect[xr::StereoView::Count];

            sample::dx::SwapchainD3D11 ColorSwapchain;
            sample::dx::SwapchainD3D11 DepthSwapchain;
        };
        std::unordered_map<XrViewConfigurationType, ViewConfigComponent> m_viewConfigComponents;
        XrViewConfigurationType m_defaultViewConfigurationType;

        winrt::com_ptr<ID3D11DepthStencilState> m_reversedZDepthNoStencilTest;
    };

    class ProjectionLayers {
    public:
        ProjectionLayers() = default;
        ProjectionLayers(ProjectionLayers&&) = default;
        ProjectionLayers(const ProjectionLayers&) = delete;

        ~ProjectionLayers() {
            std::lock_guard lock(m_mutex);
            m_projectionLayers.clear();
        }

        uint32_t Size() const {
            std::lock_guard lock(m_mutex);
            return (uint32_t)m_projectionLayers.size();
        }

        ProjectionLayer& At(uint32_t index) {
            std::lock_guard lock(m_mutex);
            return *m_projectionLayers.at(index);
        }

        void Resize(uint32_t size, Context& context, bool forceReset = false) {
            std::lock_guard lock(m_mutex);
            if (forceReset || m_projectionLayers.size() != size) {
                m_projectionLayers.clear();
                m_projectionLayers.reserve(size);
                for (uint32_t i = 0; i < size; i++) {
                    m_projectionLayers.emplace_back(std::make_unique<ProjectionLayer>(context.Session));
                    m_projectionLayers.back()->Config().ForceReset = true;
                }
            }
        }

        void ForEachLayerWithLock(std::function<void(ProjectionLayer&)> function) {
            std::lock_guard lock(m_mutex);
            for (std::unique_ptr<ProjectionLayer>& layer : m_projectionLayers) {
                function(*layer);
            }
        }

    private:
        mutable std::mutex m_mutex;
        std::vector<std::unique_ptr<ProjectionLayer>> m_projectionLayers;
    };
} // namespace engine

