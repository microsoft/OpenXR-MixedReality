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

#include <XrUtility/XrStereoView.h>
#include <XrUtility/XrHandle.h>
#include <XrUtility/XrMath.h>
#include <SampleShared/DxUtility.h>

struct ProjectionLayerConfig {
    // FovScale = (TargetFov / HmdFov)
    XrFovf FovScale = {1.0f, 1.0f, 1.0f, 1.0f};
    xr::math::NearFar NearFar = {1000, 0.02f};
    XrPosef ViewPoseOffsets[xr::StereoView::Count] = {{{0, 0, 0, 1}, {0, 0, 0}}, {{0, 0, 0, 1}, {0, 0, 0}}};
    // SwapchainSizeScale applied on recommended image rect
    XrExtent2Df SwapchainSizeScale = {1, 1};
    XrExtent2Df SwapchainFovScale = {1, 1};
    // ViewportSizeScale applied on swapchain image size with SwapchainSizeScale applied
    XrExtent2Df ViewportSizeScale = {1, 1};
    // ViewportOffset relative to (0, 0) of the swapchain
    XrOffset2Di ViewportOffset = {0, 0};
    uint32_t SwapchainSampleCount = 1;
    bool DoubleWideMode = false;
    bool IgnoreDepthLayer = false;
    bool ContentProtected = false;
    bool ForceReset = false;
    bool VisibilityMaskEnabled = false;
    bool YFlipViewAxis = false;
    bool FrontFaceCounterClockwise = false;
    DXGI_FORMAT ColorSwapchainFormat{};
    DXGI_FORMAT DepthSwapchainFormat{};
    // Layer's Color/Depth Offset/Scale swapchain image size with SwapchainSizeScale applied
    XrOffset2Di ColorLayerOffset = {0, 0};
    XrOffset2Di DepthLayerOffset = {0, 0};
    XrExtent2Df ColorLayerSizeScale = {1, 1};
    XrExtent2Df DepthLayerSizeScale = {1, 1};
    XrCompositionLayerFlags LayerFlags = 0;
    std::array<uint32_t, xr::StereoView::Count> ColorImageArrayIndices = {xr::StereoView::Left, xr::StereoView::Right};
    std::array<uint32_t, xr::StereoView::Count> DepthImageArrayIndices = {xr::StereoView::Left, xr::StereoView::Right};
    float minDepth = 0.0f;
    float maxDepth = 1.0f;
};

struct IVisibilityMask;
class ProjectionLayer {
public:

public:
    ProjectionLayer()
        : m_ensureSupportSwapchainFormat(nullptr){};
    ProjectionLayer(std::function<void(DXGI_FORMAT, bool /*isDepth*/)> ensureSupportSwapchainFormat,
                    DXGI_FORMAT colorSwapchainFormat,
                    DXGI_FORMAT depthSwapchainFormat,
                    XrViewConfigurationType primaryViewConfiguraionType,
                    const std::vector<XrViewConfigurationType>& secondaryViewConfiguraionTypes);
    ProjectionLayer(ProjectionLayer&&) = default;

    void SetPause(bool pause) {
        m_pause = pause;
    }

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

    void PrepareRendering(const SceneContext* sceneContext,
                          XrViewConfigurationType viewConfigType,
                          const std::vector<XrViewConfigurationView>& viewConfigViews,
                          bool canCreateSwapchain);

    bool Render(SceneContext* sceneContext,
                const FrameTime& frameTime,
                XrSpace layerSpace,
                const std::vector<XrView>& Views,
                const std::vector<std::unique_ptr<Scene>>& activeScenes,
                const IVisibilityMask* visibilityMask,
                XrViewConfigurationType viewConfig);

private:
    ProjectionLayer(const ProjectionLayer&) = delete;
    ProjectionLayer& operator=(const ProjectionLayer&) = delete;
    ProjectionLayer& operator=(ProjectionLayer&&) = default;

    struct ViewConfigComponent {
        ProjectionLayerConfig CurrentConfig;
        ProjectionLayerConfig PendingConfig;

        XrSpace LayerSpace{XR_NULL_HANDLE};
        std::vector<XrCompositionLayerProjectionView> ProjectionViews; // Pre-allocated and reused for each frame.
        std::vector<XrCompositionLayerDepthInfoKHR> DepthElements;     // Pre-allocated and reused for each frame.
        std::vector<D3D11_VIEWPORT> Viewports;

        XrRect2Di LayerColorImageRect[xr::StereoView::Count];
        XrRect2Di LayerDepthImageRect[xr::StereoView::Count];

        sample::dx::SwapchainD3D11 ColorSwapchain;
        sample::dx::SwapchainD3D11 DepthSwapchain;
    };
    std::unordered_map<XrViewConfigurationType, ViewConfigComponent> m_viewConfigComponents;
    XrViewConfigurationType m_defaultViewConfigurationType;

    bool m_pause = false;

    winrt::com_ptr<ID3D11DepthStencilState> m_noDepthWithStencilWrite;
    winrt::com_ptr<ID3D11DepthStencilState> m_forwardZWithStencilTest;
    winrt::com_ptr<ID3D11DepthStencilState> m_reversedZDepthNoStencilTest;
    winrt::com_ptr<ID3D11DepthStencilState> m_reversedZDepthWithStencilTest;

    std::function<void(DXGI_FORMAT format, bool isDepth)> m_ensureSupportSwapchainFormat;
};

class ProjectionLayers {
public:
    ProjectionLayers(std::function<std::unique_ptr<ProjectionLayer>()> createLayerFunction)
        : m_createLayerFunction(createLayerFunction) {
    }

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

    void Resize(uint32_t size, bool forceReset = false) {
        std::lock_guard lock(m_mutex);
        if (forceReset || m_projectionLayers.size() != size) {
            m_projectionLayers.clear();
            m_projectionLayers.reserve(size);
            for (uint32_t i = 0; i < size; i++) {
                m_projectionLayers.emplace_back(m_createLayerFunction());
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
    std::function<std::unique_ptr<ProjectionLayer>()> m_createLayerFunction;
};

