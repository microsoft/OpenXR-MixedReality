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
#include "Scene.h"
#include "ProjectionLayer.h"
#include "CompositionLayers.h"
#include "SceneContext.h"
#include "VisibilityMask.h"

#include <SampleShared/DxUtility.h>
#include <SampleShared/Trace.h>
#include <XrUtility/XrMath.h>

using namespace DirectX;

ProjectionLayer::ProjectionLayer(std::function<void(DXGI_FORMAT, bool /*isDepth*/)> ensureSupportSwapchainFormat,
                                 DXGI_FORMAT colorSwapchainFormat,
                                 DXGI_FORMAT depthSwapchainFormat,
                                 XrViewConfigurationType primaryViewConfiguraionType,
                                 const std::vector<XrViewConfigurationType>& secondaryViewConfiguraionTypes)
    : m_ensureSupportSwapchainFormat(ensureSupportSwapchainFormat) {
    m_defaultViewConfigurationType = primaryViewConfiguraionType;
    m_viewConfigComponents[primaryViewConfiguraionType].PendingConfig.ColorSwapchainFormat = colorSwapchainFormat;
    m_viewConfigComponents[primaryViewConfiguraionType].PendingConfig.DepthSwapchainFormat = depthSwapchainFormat;

    for (const XrViewConfigurationType type : secondaryViewConfiguraionTypes) {
        m_viewConfigComponents[type].PendingConfig.ColorSwapchainFormat = colorSwapchainFormat;
        m_viewConfigComponents[type].PendingConfig.DepthSwapchainFormat = depthSwapchainFormat;

        if (type == XR_VIEW_CONFIGURATION_TYPE_SECONDARY_MONO_FIRST_PERSON_OBSERVER_MSFT) {
            m_viewConfigComponents[type].PendingConfig.LayerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        }
    }
}

void ProjectionLayer::PrepareRendering(const SceneContext* sceneContext,
                                       XrViewConfigurationType viewConfigType,
                                       const std::vector<XrViewConfigurationView>& viewConfigViews,
                                       bool canCreateSwapchain) {
    ViewConfigComponent& viewConfigComponent = m_viewConfigComponents.at(viewConfigType);
    ProjectionLayerConfig& layerPendingConfig = viewConfigComponent.PendingConfig;
    ProjectionLayerConfig& layerCurrentConfig = viewConfigComponent.CurrentConfig;

    bool shouldResetSwapchain = layerPendingConfig.ForceReset || layerCurrentConfig.DoubleWideMode != layerPendingConfig.DoubleWideMode ||
                                layerCurrentConfig.SwapchainSampleCount != layerPendingConfig.SwapchainSampleCount ||
                                layerCurrentConfig.SwapchainSizeScale.width != layerPendingConfig.SwapchainSizeScale.width ||
                                layerCurrentConfig.SwapchainSizeScale.height != layerPendingConfig.SwapchainSizeScale.height ||
                                layerCurrentConfig.ContentProtected != layerPendingConfig.ContentProtected;

    if (layerCurrentConfig.ColorSwapchainFormat != layerPendingConfig.ColorSwapchainFormat) {
        m_ensureSupportSwapchainFormat(layerPendingConfig.ColorSwapchainFormat, false /*isDepth*/);
        shouldResetSwapchain = true;
    }

    if (layerCurrentConfig.DepthSwapchainFormat != layerPendingConfig.DepthSwapchainFormat) {
        m_ensureSupportSwapchainFormat(layerPendingConfig.DepthSwapchainFormat, true /*isDepth*/);
        shouldResetSwapchain = true;
    }

    if (!viewConfigComponent.ColorSwapchain.Handle || !viewConfigComponent.DepthSwapchain.Handle) {
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

    uint32_t swapchainImageWidth =
        static_cast<uint32_t>(std::ceil(recommendedImageRectWidth * layerCurrentConfig.SwapchainSizeScale.width));
    uint32_t swapchainImageHeight =
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
        const XrOffset2Di colorImageRectForDoubleWide = {doubleWideOffsetX + layerCurrentConfig.ColorLayerOffset.x,
                                                         layerCurrentConfig.ColorLayerOffset.y};
        viewConfigComponent.LayerColorImageRect[viewIndex] = {
            layerCurrentConfig.DoubleWideMode ? colorImageRectForDoubleWide : layerCurrentConfig.ColorLayerOffset,
            {static_cast<int32_t>(std::ceil(swapchainImageWidth * layerCurrentConfig.ColorLayerSizeScale.width)),
             static_cast<int32_t>(std::ceil(swapchainImageHeight * layerCurrentConfig.ColorLayerSizeScale.height))}};

        const XrOffset2Di depthImageRectForDoubleWide = {doubleWideOffsetX + layerCurrentConfig.DepthLayerOffset.x,
                                                         layerCurrentConfig.DepthLayerOffset.y};
        viewConfigComponent.LayerDepthImageRect[viewIndex] = {
            layerCurrentConfig.DoubleWideMode ? depthImageRectForDoubleWide : layerCurrentConfig.DepthLayerOffset,
            {static_cast<int32_t>(std::ceil(swapchainImageWidth * layerCurrentConfig.DepthLayerSizeScale.width)),
             static_cast<int32_t>(std::ceil(swapchainImageHeight * layerCurrentConfig.DepthLayerSizeScale.height))}};
    }

    if (!shouldResetSwapchain || !canCreateSwapchain) {
        return;
    }

    const uint32_t wideScale = layerCurrentConfig.DoubleWideMode ? 2 : 1;
    const uint32_t arrayLength = layerCurrentConfig.DoubleWideMode ? 1 : (uint32_t)viewConfigViews.size();

    // Create color swapchain with recommended properties.
    viewConfigComponent.ColorSwapchain =
        sample::dx::CreateSwapchainD3D11(sceneContext->Session,
                                         layerCurrentConfig.ColorSwapchainFormat,
                                         swapchainImageWidth * wideScale,
                                         swapchainImageHeight,
                                         arrayLength,
                                         swapchainSampleCount,
                                         layerCurrentConfig.ContentProtected ? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0,
                                         XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT,
                                         viewConfigType);

    // Create depth swapchain with recommended properties.
    viewConfigComponent.DepthSwapchain =
        sample::dx::CreateSwapchainD3D11(sceneContext->Session,
                                         layerCurrentConfig.DepthSwapchainFormat,
                                         swapchainImageWidth * wideScale,
                                         swapchainImageHeight,
                                         arrayLength,
                                         swapchainSampleCount,
                                         layerCurrentConfig.ContentProtected ? XR_SWAPCHAIN_CREATE_PROTECTED_CONTENT_BIT : 0,
                                         XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                         viewConfigType);

    {
        CD3D11_DEPTH_STENCIL_DESC depthStencilDesc(CD3D11_DEFAULT{});
        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; // Set stencil buffer to ref val on succeeded pixels
        m_noDepthWithStencilWrite = nullptr;
        CHECK_HRCMD(sceneContext->Device->CreateDepthStencilState(&depthStencilDesc, m_noDepthWithStencilWrite.put()));

        // Pass stencil test if stencil buffer's value == ref value; do not modify stencil buffer
        const D3D11_DEPTH_STENCILOP_DESC stencilTestOp = {
            D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_EQUAL};
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.FrontFace = stencilTestOp;
        m_forwardZWithStencilTest = nullptr;
        CHECK_HRCMD(sceneContext->Device->CreateDepthStencilState(&depthStencilDesc, m_forwardZWithStencilTest.put()));

        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
        m_reversedZDepthNoStencilTest = nullptr;
        CHECK_HRCMD(sceneContext->Device->CreateDepthStencilState(&depthStencilDesc, m_reversedZDepthNoStencilTest.put()));

        depthStencilDesc.StencilEnable = true;
        m_reversedZDepthWithStencilTest = nullptr;
        CHECK_HRCMD(sceneContext->Device->CreateDepthStencilState(&depthStencilDesc, m_reversedZDepthWithStencilTest.put()));
    }

    viewConfigComponent.ProjectionViews.resize(viewConfigViews.size());
    viewConfigComponent.DepthElements.resize(viewConfigViews.size());
}

bool ProjectionLayer::Render(SceneContext* sceneContext,
                             const FrameTime& frameTime,
                             XrSpace layerSpace,
                             const std::vector<XrView>& views,
                             const std::vector<std::unique_ptr<Scene>>& activeScenes,
                             const IVisibilityMask* visibilityMask,
                             XrViewConfigurationType viewConfig) {
    if (m_pause) {
        return true; // submit previous frame.
    }

    ViewConfigComponent& viewConfigComponent = m_viewConfigComponents.at(viewConfig);
    const sample::dx::SwapchainD3D11& colorSwapchain = viewConfigComponent.ColorSwapchain;
    const sample::dx::SwapchainD3D11& depthSwapchain = viewConfigComponent.DepthSwapchain;
    std::vector<XrCompositionLayerProjectionView>& projectionViews = viewConfigComponent.ProjectionViews;
    std::vector<XrCompositionLayerDepthInfoKHR>& depthElements = viewConfigComponent.DepthElements;
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

            XrFovf fov;
            fov.angleLeft = projection.fov.angleLeft * currentConfig.FovScale.angleLeft;
            fov.angleRight = projection.fov.angleRight * currentConfig.FovScale.angleRight;
            fov.angleUp = projection.fov.angleUp * currentConfig.FovScale.angleUp;
            fov.angleDown = projection.fov.angleDown * currentConfig.FovScale.angleDown;

            viewConfigComponent.LayerSpace = layerSpace;
            projectionViews[viewIndex] = {XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
            projectionViews[viewIndex].pose = xr::math::Pose::Multiply(projection.pose, currentConfig.ViewPoseOffsets[viewIndex]);
            projectionViews[viewIndex].fov.angleLeft = fov.angleLeft * currentConfig.SwapchainFovScale.width;
            projectionViews[viewIndex].fov.angleRight = fov.angleRight * currentConfig.SwapchainFovScale.width;
            projectionViews[viewIndex].fov.angleUp = fov.angleUp * currentConfig.SwapchainFovScale.height;
            projectionViews[viewIndex].fov.angleDown = fov.angleDown * currentConfig.SwapchainFovScale.height;
            projectionViews[viewIndex].subImage.swapchain = colorSwapchain.Handle.Get();
            projectionViews[viewIndex].subImage.imageArrayIndex =
                currentConfig.DoubleWideMode ? 0 : currentConfig.ColorImageArrayIndices[viewIndex];
            projectionViews[viewIndex].subImage.imageRect = viewConfigComponent.LayerColorImageRect[viewIndex];

            uint32_t depthImageArrayIndex = currentConfig.DoubleWideMode ? 0 : currentConfig.DepthImageArrayIndices[viewIndex];

            D3D11_VIEWPORT viewport = viewports[viewIndex];
            if (currentConfig.IgnoreDepthLayer) {
                projectionViews[viewIndex].next = nullptr;
            } else {
                depthElements[viewIndex] = {XR_TYPE_COMPOSITION_LAYER_DEPTH_INFO_KHR};
                depthElements[viewIndex].minDepth = viewport.MinDepth = currentConfig.minDepth;
                depthElements[viewIndex].maxDepth = viewport.MaxDepth = currentConfig.maxDepth;
                depthElements[viewIndex].nearZ = currentConfig.NearFar.Near;
                depthElements[viewIndex].farZ = currentConfig.NearFar.Far;
                depthElements[viewIndex].subImage.swapchain = depthSwapchain.Handle.Get();
                depthElements[viewIndex].subImage.imageArrayIndex = depthImageArrayIndex;
                depthElements[viewIndex].subImage.imageRect = viewConfigComponent.LayerDepthImageRect[viewIndex];

                projectionViews[viewIndex].next = &depthElements[viewIndex];
            }

            // Render for this view pose.
            {
                // Set the Viewport.
                sceneContext->DeviceContext->RSSetViewports(1, &viewport);

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
                CHECK_HRCMD(sceneContext->Device->CreateRenderTargetView(
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
                CHECK_HRCMD(sceneContext->Device->CreateDepthStencilView(
                    depthSwapchain.Images[depthSwapchainImageIndex].texture, &depthStencilViewDesc, depthStencilView.put()));

                const bool reversedZ = (currentConfig.NearFar.Near > currentConfig.NearFar.Far);

                // Clear and render to the render target.
                ID3D11RenderTargetView* const renderTargets[] = {renderTargetView.get()};
                sceneContext->DeviceContext->OMSetRenderTargets(1, renderTargets, depthStencilView.get());

                // In double wide mode, the first projection clears the whole RTV and DSV.
                if ((viewIndex == 0) || !currentConfig.DoubleWideMode) {
                    XMVECTORF32 clearColor = sceneContext->PrimaryViewConfigEnvironmentBlendMode == XR_ENVIRONMENT_BLEND_MODE_OPAQUE
                                                 ? DirectX::Colors::CornflowerBlue
                                                 : DirectX::Colors::Transparent;
                    clearColor.v = DirectX::XMColorSRGBToRGB(clearColor.v);
                    sceneContext->DeviceContext->ClearRenderTargetView(renderTargets[0], clearColor);

                    const float clearDepthValue = reversedZ ? 0.f : 1.f;
                    sceneContext->DeviceContext->ClearDepthStencilView(
                        depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, clearDepthValue, 0);
                }

                const DirectX::XMMATRIX projectionMatrix = xr::math::ComposeProjectionMatrix(fov, currentConfig.NearFar);
                const bool visibilityMaskEnabled = currentConfig.VisibilityMaskEnabled;
                const UINT stencilTestRef = 1;

                bool shouldUseStencil = false;
                if (visibilityMask && visibilityMaskEnabled) {
                    sceneContext->DeviceContext->OMSetDepthStencilState(m_noDepthWithStencilWrite.get(), stencilTestRef);
                    shouldUseStencil = visibilityMask->RenderMask(viewIndex, projectionMatrix);
                }

                if (reversedZ) {
                    if (shouldUseStencil) {
                        sceneContext->DeviceContext->OMSetDepthStencilState(m_reversedZDepthWithStencilTest.get(), stencilTestRef);
                    } else {
                        sceneContext->DeviceContext->OMSetDepthStencilState(m_reversedZDepthNoStencilTest.get(), 0);
                    }
                } else {
                    if (shouldUseStencil) {
                        sceneContext->DeviceContext->OMSetDepthStencilState(m_forwardZWithStencilTest.get(), stencilTestRef);
                    } else {
                        sceneContext->DeviceContext->OMSetDepthStencilState(nullptr, 0);
                    }
                }

                // Set state for any objects which use PBR rendering.
                // PBR library expects traditional view transform (world to view).
                DirectX::XMMATRIX worldToViewMatrix = xr::math::LoadInvertedXrPose(projectionViews[viewIndex].pose);

                if (currentConfig.YFlipViewAxis) {
                    DirectX::XMMATRIX yFlip = DirectX::XMMatrixIdentity();
                    yFlip.r[1] = XMVectorSet(0, -1, 0, 0);
                    worldToViewMatrix *= yFlip;
                }

                sceneContext->PbrResources.SetViewProjection(worldToViewMatrix, projectionMatrix);
                sceneContext->PbrResources.Bind(sceneContext->DeviceContext.get());
                sceneContext->PbrResources.SetDepthFuncReversed(reversedZ);
                sceneContext->PbrResources.SetFrontFaceWindingOrder(currentConfig.FrontFaceCounterClockwise
                                                                        ? Pbr::FrontFaceWindingOrder::CounterClockWise
                                                                        : Pbr::FrontFaceWindingOrder::ClockWise);

                // Render all active scenes.
                for (const std::unique_ptr<Scene>& scene : activeScenes) {
                    if (scene->HasSceneObjects() && scene->IsActive()) {
                        submitProjectionLayer = true;
                        scene->Render(frameTime);
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

    sceneContext->PbrResources.UpdateAnimationTime(frameTime.TotalElapsed);

    return submitProjectionLayer;
}

void AppendProjectionLayer(CompositionLayers& layers, const ProjectionLayer* layer, XrViewConfigurationType viewConfig) {
    XrCompositionLayerProjection& projectionLayer = layers.AddProjectionLayer(layer->Config(viewConfig).LayerFlags);
    projectionLayer.space = layer->LayerSpace(viewConfig);
    projectionLayer.viewCount = (uint32_t)layer->ProjectionViews(viewConfig).size();
    projectionLayer.views = layer->ProjectionViews(viewConfig).data();

}

