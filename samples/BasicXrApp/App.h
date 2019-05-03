//*********************************************************//
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

namespace xr {
    struct Cube {
        XrSpace Space{XR_NULL_HANDLE};
        XrVector3f Scale{0.1f, 0.1f, 0.1f};

        XrBool32 PoseValid{false};
        XrPosef Pose = xr::math::Pose::Identity();
    };

    struct IOpenXrProgram {
        virtual ~IOpenXrProgram() = default;
        virtual void Run() = 0;
    };

    struct IGraphicsPlugin {
        virtual ~IGraphicsPlugin() = default;

        // Create an instance of this graphics api for the provided instance and systemId.
        virtual void InitializeDevice(XrInstance instance, XrSystemId systemId) = 0;

        // Select the preferred swapchain format from the list of available formats.
        virtual int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const = 0;

        // Get the graphics binding header for session creation.
        virtual const XrGraphicsBindingD3D11KHR* GetGraphicsBinding() const = 0;

        // Allocate space for the swapchain image structures. These are different for each graphics API. The returned
        // pointers are valid for the lifetime of the graphics plugin.
        virtual std::vector<XrSwapchainImageBaseHeader*>
        AllocateSwapchainImageStructs(uint32_t capacity, const XrSwapchainCreateInfo& swapchainCreateInfo) = 0;

        // Render to a swapchain image for a projection view.
        virtual void RenderView(const XrCompositionLayerProjectionView& layerView,
                                const XrSwapchainImageBaseHeader* swapchainImage,
                                const XrEnvironmentBlendMode environmentBlendMode,
                                int64_t colorSwapchainFormat,
                                const std::vector<Cube>& cubes) = 0;
    };

    std::unique_ptr<IGraphicsPlugin> CreateCubeGraphics();
    std::unique_ptr<IOpenXrProgram> CreateOpenXrProgram(std::string applicationName, std::unique_ptr<IGraphicsPlugin> graphicsPlugin);

} // namespace xr
