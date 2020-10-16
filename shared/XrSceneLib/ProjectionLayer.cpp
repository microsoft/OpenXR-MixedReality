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

#include <XrUtility/XrMath.h>
#include <XrUtility/XrEnumerate.h>
#include <SampleShared/DxUtility.h>
#include <SampleShared/Trace.h>

#include "ProjectionLayer.h"
#include "CompositionLayers.h"
#include "Scene.h"
#include "Context.h"

using namespace DirectX;

engine::ProjectionLayer::ProjectionLayer(const xr::SessionContext& sessionContext) {
    auto primaryViewConfiguraionType = sessionContext.PrimaryViewConfigurationType;
    auto colorSwapchainFormat = sessionContext.SupportedColorSwapchainFormats[0];
    auto depthSwapchainFormat = sessionContext.SupportedDepthSwapchainFormats[0];

    m_defaultViewConfigurationType = primaryViewConfiguraionType;
    m_viewConfigComponents[primaryViewConfiguraionType].PendingConfig.ColorSwapchainFormat = colorSwapchainFormat;
    m_viewConfigComponents[primaryViewConfiguraionType].PendingConfig.DepthSwapchainFormat = depthSwapchainFormat;

    for (const XrViewConfigurationType type : sessionContext.EnabledSecondaryViewConfigurationTypes) {
        m_viewConfigComponents[type].PendingConfig.ColorSwapchainFormat = colorSwapchainFormat;
        m_viewConfigComponents[type].PendingConfig.DepthSwapchainFormat = depthSwapchainFormat;

        if (type == XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT) {
            m_viewConfigComponents[type].PendingConfig.LayerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        }
    }
}

void engine::ProjectionLayer::PrepareRendering(const Context& context,
                                               XrViewConfigurationType viewConfigType,
                                               const std::vector<XrViewConfigurationView>& viewConfigViews) {
    ViewConfigComponent& viewConfigComponent = m_viewConfigComponents.at(viewConfigType);
    ProjectionLayerConfig& layerPendingConfig = viewConfigComponent.PendingConfig;
    ProjectionLayerConfig& layerCurrentConfig = viewConfigComponent.CurrentConfig;

    bool shouldResetSwapchain = layerPendingConfig.ForceReset || layerCurrentConfig.DoubleWideMode != layerPendingConfig.DoubleWideMode ||
                                layerCurrentConfig.SwapchainSampleCount != layerPendingConfig.SwapchainSampleCount ||
                                layerCurrentConfig.SwapchainSizeScale.width != layerPendingConfig.SwapchainSizeScale.width ||
                                layerCurrentConfig.SwapchainSizeScale.height != layerPendingConfig.SwapchainSizeScale.height ||
                                layerCurrentConfig.ContentProtected != layerPendingConfig.ContentProtected;

    if (!shouldResetSwapchain && layerCurrentConfig.ColorSwapchainFormat != layerPendingConfig.ColorSwapchainFormat) {
        if (!xr::Contains(context.Session.SupportedColorSwapchainFormats, layerPendingConfig.ColorSwapchainFormat)) {
            throw std::runtime_error(
                fmt::format("Unsupported color swapchain format: {}", layerPendingConfig.ColorSwapchainFormat).c_str());
        }
        shouldResetSwapchain = true;
    }

    if (!shouldResetSwapchain && layerCurrentConfig.DepthSwapchainFormat != layerPendingConfig.DepthSwapchainFormat) {
        if (!xr::Contains(context.Session.SupportedDepthSwapchainFormats, layerPendingConfig.DepthSwapchainFormat)) {
            throw std::runtime_error(
                fmt::format("Unsupported depth swapchain format: {}", layerPendingConfig.DepthSwapchainFormat).c_str());
        }
        shouldResetSwapchain = true;
    }

    if (!shouldResetSwapchain && !viewConfigComponent.ColorSwapchain.Handle || !viewConfigComponent.DepthSwapchain.Handle) {
        shouldResetSwapchain = true;
    }

    layerPendingConfig.ForceReset = false;
    layerCurrentConfig = layerPendingConfig;

    // SceneLib only supports identical sized swapchain images for left and right eyes (texture array/double wide).
    // Thus if runtime gives us different image rect sizes for left/right eyes,
    // we use the maximum left/right imageRect extent for recommendedImageRectExtent
    assert(!viewConfigViews.empty());
    uint32_t recommendedImageRectWidth = 0;
    uint32_t recommendedImageRectHeight = 0;
    for (const XrViewConfigurationView& view : viewConfigViews) {
        recommendedImageRectWidth = std::max(recommendedImageRectWidth, view.recommendedImageRectWidth);
        recommendedImageRectHeight = std::max(recommendedImageRectHeight, view.recommendedImageRectHeight);
    }
    assert(recommendedImageRectWidth != 0 && recommendedImageRectHeight != 0);

    const uint32_t swapchainImageWidth =
        static_cast<uint32_t>(std::ceil(recommendedImageRectWidth * layerCurrentConfig.SwapchainSizeScale.width));
    const uint32_t swapchainImageHeight =
        static_cast<uint32_t>(std::ceil(recommendedImageRectHeight * layerCurrentConfig.SwapchainSizeScale.height));

    const uint32_t swapchainSampleCount = layerCurrentConfig.SwapchainSampleCount < 1
                                              ? viewConfigViews[xr::StereoView::Left].recommendedSwapchainSampleCount
                                              : layerCurrentConfig.SwapchainSampleCount;

    viewConfigComponent.Viewports.resize(viewConfigViews.size());

    for (uint32_t viewIndex = 0; viewIndex < (uint32_t)viewConfigViews.size(); viewIndex++) {
        viewConfigComponent.Viewports[viewIndex] = CD3D11_VIEWPORT(
            layerCurrentConfig.DoubleWideMode ? static_cast<float>(swapchainImageWidth * viewIndex + layerCurrentConfig.ViewportOffset.x)
                                              : static_cast<float>(layerCurrentConfig.ViewportOffset.x),
            static_cast<float>(layerCurrentConfig.ViewportOffset.y),
            static_cast<float>(swapchainImageWidth * layerCurrentConfig.ViewportSizeScale.width),
            static_cast<float>(swapchainImageHeight * layerCurrentConfig.ViewportSizeScale.height));

        const int32_t doubleWideOffsetX = static_cast<int32_t>(swapchainImageWidth * viewIndex);
        viewConfigComponent.LayerDepthImageRect[viewIndex] =
            viewConfigComponent.LayerColorImageRect[viewIndex] = {layerCurrentConfig.DoubleWideMode ? doubleWideOffsetX : 0,
                                                                  0,
                                                                  static_cast<int32_t>(std::ceil(swapchainImageWidth)),
                                                                  static_cast<int32_t>(std::ceil(swapchainImageHeight))};
    }

    if (!shouldResetSwapchain) {
        return;
    }

    const uint32_t wideScale = layerCurrentConfig.DoubleWideMode ? 2 : 1;
    const uint32_t arrayLength = layerCurrentConfig.DoubleWideMode ? 1 : (uint32_t)viewConfigViews.size();

    const std::optional<XrViewConfigurationType> viewConfigurationForSwapchain =
        context.Extensions.SupportsSecondaryViewConfiguration ? std::optional{viewConfigType} : std::nullopt;

    // Create color swapchain with recommended properties.
    viewConfigComponent.ColorSwapchain =
        sample::dx::CreateSwapchainD3D11(context.Session.Handle,
                                         layerCurrentConfig.ColorSwapchainFormat,
                                         swapchainImageWidth * wideScale,
                                         swapchainImageHeight,
                                         arrayLength,
                                         swapchainSampleCount,
                                         layerCurrentConfig.ContentProtected ? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0,
                                         XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
                                         viewConfigurationForSwapchain);

    // Create depth swapchain with recommended properties.
    viewConfigComponent.DepthSwapchain =
        sample::dx::CreateSwapchainD3D11(context.Session.Handle,
                                         layerCurrentConfig.DepthSwapchainFormat,
                                         swapchainImageWidth * wideScale,
                                         swapchainImageHeight,
                                         arrayLength,
                                         swapchainSampleCount,
                                         layerCurrentConfig.ContentProtected ? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0,
                                         XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                         viewConfigurationForSwapchain);

    {
        CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(CD3D11_DEFAULT{});
        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
        m_reversedZDepthNoStencilTest = nullptr;
        CHECK_HRCMD(context.Device->CreateDepthStencilState(&depthStencilDesc, m_reversedZDepthNoStencilTest.put()));
    }

    viewConfigComponent.ProjectionViews.resize(viewConfigViews.size());
    viewConfigComponent.DepthInfo.resize(viewConfigViews.size());
}

bool engine::ProjectionLayer::Render(Context& context,
                                     const engine::FrameTime& frameTime,
                                     XrSpace layerSpace,
                                     const std::vector<XrView>& views,
                                     const std::vector<std::unique_ptr<Scene>>& activeScenes,
                                     XrViewConfigurationType viewConfig) {

    ViewConfigComponent& viewConfigComponent = m_viewConfigComponents.at(viewConfig);
    const sample::dx::SwapchainD3D11& colorSwapchain = viewConfigComponent.ColorSwapchain;
    const sample::dx::SwapchainD3D11& depthSwapchain = viewConfigComponent.DepthSwapchain;
    std::vector<XrCompositionLayerProjectionView>& projectionViews = viewConfigComponent.ProjectionViews;
    std::vector<XrCompositionLayerDepthInfoKHR>& depthInfo = viewConfigComponent.DepthInfo;
    std::vector<D3D11_VIEWPORT>& viewports = viewConfigComponent.Viewports;
    const ProjectionLayerConfig& currentConfig = viewConfigComponent.CurrentConfig;

    bool submitProjectionLayer = false;
    const XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    uint32_t colorSwapchainImageIndex{};
    CHECK_XRCMD(xrAcquireSwapchainImage(colorSwapchain.Handle.Get(), &acquireInfo, &colorSwapchainImageIndex));
    uint32_t depthSwapchainImageIndex{};
    CHECK_XRCMD(xrAcquireSwapchainImage(depthSwapchain.Handle.Get(), &acquireInfo, &depthSwapchainImageIndex));

    XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    waitInfo.timeout = XR_INFINITE_DURATION;
    const XrResult colorSwapchainWait = CHECK_XRCMD(xrWaitSwapchainImage(colorSwapchain.Handle.Get(), &waitInfo));
    const XrResult depthSwapchainWait = CHECK_XRCMD(xrWaitSwapchainImage(depthSwapchain.Handle.Get(), &waitInfo));
    if ((colorSwapchainWait != XR_SUCCESS) || (depthSwapchainWait != XR_SUCCESS)) {
        // Swapchain image timeout, don't submit this multi projection layer
        submitProjectionLayer = false;
    } else {
        const uint32_t viewCount = (uint32_t)views.size();
        for (uint32_t viewIndex = 0; viewIndex < viewCount; viewIndex++) {
            const XrView& projection = views[viewIndex];

            const XrFovf fov = projection.fov;
            const XrPosef viewPose = projection.pose;
            const float normalizedViewportMinDepth = 0;
            const float normalizedViewportMaxDepth = 1;
            const uint32_t colorImageArrayIndex = currentConfig.DoubleWideMode ? 0 : viewIndex;
            const uint32_t depthImageArrayIndex = currentConfig.DoubleWideMode ? 0 : viewIndex;

            viewConfigComponent.LayerSpace = layerSpace;
            projectionViews[viewIndex] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
            projectionViews[viewIndex].pose = viewPose;
            projectionViews[viewIndex].fov.angleLeft = fov.angleLeft * currentConfig.SwapchainFovScale.width;
            projectionViews[viewIndex].fov.angleRight = fov.angleRight * currentConfig.SwapchainFovScale.width;
            projectionViews[viewIndex].fov.angleUp = fov.angleUp * currentConfig.SwapchainFovScale.height;
            projectionViews[viewIndex].fov.angleDown = fov.angleDown * currentConfig.SwapchainFovScale.height;
            projectionViews[viewIndex].subImage.swapchain = colorSwapchain.Handle.Get();
            projectionViews[viewIndex].subImage.imageArrayIndex = colorImageArrayIndex;
            projectionViews[viewIndex].subImage.imageRect = viewConfigComponent.LayerColorImageRect[viewIndex];

            D3D11_VIEWPORT viewport = viewports[viewIndex];
            if (currentConfig.SubmitDepthInfo && context.Extensions.SupportsDepthInfo) {
                depthInfo[viewIndex] = {XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR};
                depthInfo[viewIndex].minDepth = viewport.MinDepth = normalizedViewportMinDepth;
                depthInfo[viewIndex].maxDepth = viewport.MaxDepth = normalizedViewportMaxDepth;
                depthInfo[viewIndex].nearZ = currentConfig.NearFar.Near;
                depthInfo[viewIndex].farZ = currentConfig.NearFar.Far;
                depthInfo[viewIndex].subImage.swapchain = depthSwapchain.Handle.Get();
                depthInfo[viewIndex].subImage.imageArrayIndex = depthImageArrayIndex;
                depthInfo[viewIndex].subImage.imageRect = viewConfigComponent.LayerDepthImageRect[viewIndex];

                projectionViews[viewIndex].next = &depthInfo[viewIndex];
            } else {
                projectionViews[viewIndex].next = nullptr;
            }

            // Render for this view pose.
            {
                // Set the Viewport.
                context.DeviceContext->RSSetViewports(1, &viewport);

                const uint32_t firstArraySliceForColor = projectionViews[viewIndex].subImage.imageArrayIndex;

                // Create a render target view into the appropriate slice of the color texture from this swapchain image.
                // This is a lightweight operation which can be done for each viewport projection.
                winrt::com_ptr<ID3D11RenderTargetView> renderTargetView;
                const CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
                    currentConfig.SwapchainSampleCount > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
                    currentConfig.ColorSwapchainFormat,
                    0 /* mipSlice */,
                    firstArraySliceForColor,
                    1 /* arraySize */);
                CHECK_HRCMD(context.Device->CreateRenderTargetView(
                    colorSwapchain.Images[colorSwapchainImageIndex].texture, &renderTargetViewDesc, renderTargetView.put()));

                const uint32_t firstArraySliceForDepth = depthImageArrayIndex;

                // Create a depth stencil view into the slice of the depth stencil texture array for this swapchain image.
                // This is a lightweight operation which can be done for each viewport projection.
                winrt::com_ptr<ID3D11DepthStencilView> depthStencilView;
                CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
                    currentConfig.SwapchainSampleCount > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
                    currentConfig.DepthSwapchainFormat,
                    0 /* mipSlice */,
                    firstArraySliceForDepth,
                    1 /* arraySize */);
                CHECK_HRCMD(context.Device->CreateDepthStencilView(
                    depthSwapchain.Images[depthSwapchainImageIndex].texture, &depthStencilViewDesc, depthStencilView.put()));

                const bool reversedZ = (currentConfig.NearFar.Near > currentConfig.NearFar.Far);

                // Clear and render to the render target.
                ID3D11RenderTargetView* const renderTargets[] = {renderTargetView.get()};
                context.DeviceContext->OMSetRenderTargets(1, renderTargets, depthStencilView.get());

                // In double wide mode, the first projection clears the whole RTV and DSV.
                if ((viewIndex == 0) || !currentConfig.DoubleWideMode) {
                    context.DeviceContext->ClearRenderTargetView(renderTargets[0], reinterpret_cast<const float*>(&Config().ClearColor));

                    const float clearDepthValue = reversedZ ? 0.f : 1.f;
                    context.DeviceContext->ClearDepthStencilView(
                        depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepthValue, 0);
                }

                const DirectX::XMMATRIX projectionMatrix = xr::math::ComposeProjectionMatrix(fov, currentConfig.NearFar);

                if (reversedZ) {
                    context.DeviceContext->OMSetDepthStencilState(m_reversedZDepthNoStencilTest.get(), 0);
                } else {
                    context.DeviceContext->OMSetDepthStencilState(nullptr, 0);
                }

                // Set state for any objects which use PBR rendering.
                // PBR library expects traditional view transform (world to view).
                DirectX::XMMATRIX worldToViewMatrix = xr::math::LoadInvertedXrPose(projectionViews[viewIndex].pose);

                context.PbrResources.SetViewProjection(worldToViewMatrix, projectionMatrix);
                context.PbrResources.Bind(context.DeviceContext.get());
                context.PbrResources.SetDepthFuncReversed(reversedZ);

                // Render all active scenes.
                for (const std::unique_ptr<Scene>& scene : activeScenes) {
                    if (scene->IsActive() && !std::empty(scene->GetObjects())) {
                        submitProjectionLayer = true;
                        scene->Render(frameTime, viewIndex);
                    }
                }
            }
        }
    }

    // Now that the scene is done writing to the swapchain, it must be released in order to be made available for
    // xrEndFrame.
    const XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
    CHECK_XRCMD(xrReleaseSwapchainImage(colorSwapchain.Handle.Get(), &releaseInfo));
    CHECK_XRCMD(xrReleaseSwapchainImage(depthSwapchain.Handle.Get(), &releaseInfo));

    context.PbrResources.UpdateAnimationTime(frameTime.TotalElapsed);

    return submitProjectionLayer;
}

void engine::AppendProjectionLayer(CompositionLayers& layers, ProjectionLayer* layer, XrViewConfigurationType viewConfig) {
    XrCompositionLayerProjection& projectionLayer = layers.AddProjectionLayer(layer->Config(viewConfig).LayerFlags);
    projectionLayer.space = layer->LayerSpace(viewConfig);
    projectionLayer.viewCount = (uint32_t)layer->ProjectionViews(viewConfig).size();
    projectionLayer.views = layer->ProjectionViews(viewConfig).data();

}

