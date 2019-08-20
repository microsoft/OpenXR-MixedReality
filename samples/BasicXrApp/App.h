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

namespace xr::sample {
    struct Cube {
        XrSpace Space{XR_NULL_HANDLE};
        XrPosef Pose = xr::math::Pose::Identity();
        XrVector3f Scale{0.1f, 0.1f, 0.1f};
    };

    struct IOpenXrProgram {
        virtual ~IOpenXrProgram() = default;
        virtual void Run() = 0;
    };

    struct IGraphicsPluginD3D11 {
        virtual ~IGraphicsPluginD3D11() = default;

        // Create an instance of this graphics api for the provided instance and systemId.
        virtual void InitializeDevice(XrInstance instance, XrSystemId systemId) = 0;

        // List of color pixel formats supported by this app.
        virtual const std::vector<DXGI_FORMAT>& SupportedColorFormats() const = 0;
        virtual const std::vector<DXGI_FORMAT>& SupportedDepthFormats() const = 0;

        // Get the graphics binding header for session creation.
        virtual const XrGraphicsBindingD3D11KHR* GetGraphicsBinding() const = 0;

        // Render to swapchain images for a projection view.
        virtual void RenderView(const XrCompositionLayerProjectionView& layerView,
                                DXGI_FORMAT colorSwapchainFormat,
                                const XrSwapchainImageD3D11KHR& colorSwapchainImage,
                                DXGI_FORMAT depthSwapchainFormat,
                                const XrSwapchainImageD3D11KHR& depthSwapchainImage,
                                const XrEnvironmentBlendMode environmentBlendMode,
                                const xr::math::NearFarDistance& nearFar,
                                const std::vector<Cube>& cubes) = 0;
    };

    std::unique_ptr<IGraphicsPluginD3D11> CreateCubeGraphics();
    std::unique_ptr<IOpenXrProgram> CreateOpenXrProgram(std::string applicationName, std::unique_ptr<IGraphicsPluginD3D11> graphicsPlugin);

} // namespace xr::sample
