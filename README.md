# OpenXR Samples for Visual Studio Developers

This repository contains a few OpenXR code samples tailored for developers who are familiar with and using Visual Studio tool chain, e.g. HoloLens 2 developers.

These OpenXR samples are using C++17 and D3D11. The same source code works across UWP applications running on HoloLens 2 and Win32 applications running on Windows Desktop with the Mixed Reality headset.

# Prepare, build and run the samples

- Understand [what is OpenXR](https://docs.microsoft.com/en-us/windows/mixed-reality/openxr#what-is-openxr) and [why OpenXR](https://docs.microsoft.com/en-us/windows/mixed-reality/openxr#why-openxr).  Read [latest OpenXR 1.0 spec (HTML)](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html) and [latest openxr.h header file (Khronos GitHub)](https://github.com/KhronosGroup/OpenXR-SDK/blob/master/include/openxr/openxr.h).

- Prepare [Windows 10 May 2019 Update (1903)](https://www.microsoft.com/en-us/software-download/windows10) and [Visual Studio 2019 latest version](https://visualstudio.microsoft.com/downloads/).

- Prepare [HoloLens 2 device](https://docs.microsoft.com/en-us/windows/mixed-reality/openxr#getting-started-with-openxr-for-hololens-2) or [Windows Mixed Reality device](https://docs.microsoft.com/en-us/windows/mixed-reality/openxr#getting-started-with-openxr-for-windows-mixed-reality-headsets).

- Clone this sample repo: `git clone https://github.com/microsoft/OpenXR-SDK-VisualStudio.git`

- Open the sample_uwp.sln or sample_win32.sln file in Visual Studio. F5 to build and run the sample. You typically choose ARM64 platform when running on HoloLens 2 devices, or choose x64 platform when running on a Windows Desktop PC with Mixed Reality headset or the HoloLens 2 Emulator.

# Understand OpenXR samples and best practices for HoloLens 2

## A typical code flow

All OpenXR API usage patterns can be found in [OpenXRProgram.cpp](https://github.com/microsoft/OpenXR-SDK-VisualStudio/blob/master/samples/BasicXrApp/OpenXrProgram.cpp) file. The Run() function at the beginning captures a typical OpenXR app code flow from initialization to the event and rendering loop.

## Select a pixel format

Always enumerate supported pixel formats using `xrEnumerateSwapchainFormats`, and choose the first color and depth pixel format from the runtime that the app supports, because that's what the runtime prefers. Note, on HoloLens 2, `DXGI_FORMAT_B8G8R8A8_UNORM_SRGB` and `DXGI_FORMAT_D16_UNORM` is typically the first choice to achieve better rendering performance. This preference can be different on VR headsets running on a Desktop PC.  
  
**Performance Warning:** Using a format other than the primary texture format will result in runtime post-processing which comes at a significant performance penalty.

## Gamma-correct rendering

Although this applies to all OpenXR runtimes, care must be taken to ensure the rendering pipeline is gamma-correct. When rendering to a swapchain, the render-target view format should match the swapchain format (e.g. DXGI_FORMAT_B8G8R8A8_UNORM_SRGB for both the swapchain format and the render-target view). The exception is if the app's rendering pipeline does a manual sRGB conversion in shader code, in which case the app should request an sRGB swapchain format but use the linear format for the render-target view (e.g. request DXGI_FORMAT_B8G8R8A8_UNORM_SRGB as the swapchain format but use DXGI_FORMAT_B8G8R8A8_UNORM as the render-target view) to prevent content from being double-gamma corrected.

## Use a single projection layer

HoloLens 2 has limited GPU power for applications to render content. Always using a single projection layer can help the application's framerate, hologram stability and visual quality.  
  
**Performance Warning:** Using more than a single projection layer will result in runtime post-processing which comes at a significant performance penalty.

## Render with texture array and VPRT

Create one `xrSwapchain` for both left and right eye using `arraySize=2` for color swapchain, and one for depth. Use a shader with VPRT and instanced draw calls for stereoscopic rendering to minimize GPU load. This also enables the runtime's optimization to achieve the best performance on HoloLens 2.
Alternatives to using a texture array, such as double-wide rendering or a separate swapchain per eye, will result in runtime post-processing which comes at a significant performance penalty.

## Render with recommended rendering parameters and frame timing

Always render with the recommended view configuration width/height, and always use `xrLocateViews` API to query for the recommended view pose, fov, and other rendering parameters before rendering. Always use the `XrFrameEndInfo.predictedDisplayTime` from the latest `xrWaitFrame` call when querying for poses and views. This allow HoloLens to adjust rendering and optimize visual quality for the person who is wearing the HoloLens.

## Submit depth buffer for projection layers

Always use `XR_KHR_composition_layer_depth_extension` and submit the depth buffer together with the projection layer when submitting a frame to `xrEndFrame`.   This can help hologram stability by enabling the hardware depth reprojection on HoloLens 2.

## Choose a reasonable depth range

Prefer a narrower depth range to scope the virtual content to help hologram stability on HoloLens. For example, the OpenXrProgram.cpp sample is using 0.1 to 20 meters. Use [reversed-Z](https://developer.nvidia.com/content/depth-precision-visualized) for a more uniformed depth resolution.  Note, on HoloLens 2, using the preferred `DXGI_FORMAT_D16_UNORM` depth format can help achieve better frame rate and performance.  Therefore above practice for depth range is more important.

## Prepare for different environment blend modes

Always enumerate supported environment blend mode using `xrEnumerateEnvironmentBlendModes` API, and prepare rendering content accordingly. For example, for a system with `XR_ENVIRONMENT_BLEND_MODE_ADDITIVE` such as the HoloLens, the app should use transparent as clear color, but for a system with `XR_ENVIRONMENT_BLEND_MODE_OPAQUE`, the app should use some opaque color as background.

## Choose unbounded space as application's root space

An application typically uses an intermediate space to connect views, actions and holograms together. Use `XR_REFERENCE_SPACE_TYPE_UNBOUNDED_MSFT` when the extension is supported to avoid undesired hologram drift when the user moves afar (e.g. 5 meters away) from where the app starts. Use `XR_REFERENCE_SPACE_TYPE_LOCAL` as a fallback if the unbounded space extension doesn't exist.

## Associate hologram with spatial anchor

Always use a distinct spatial anchor through `xrCreateSpatialAnchorSpaceMSFT` extension for any hologram that's locked into the environment. This allows the hologram stay put even if user walked away to other rooms and comes back. The spatial anchor can remember and locate the hologram where it was created.

## Support mixed reality capture

Although HoloLens 2's primary display uses additive environment blending, when the user initiates mixed reality capture, the app's rendering content might be alpha blended with the environment video stream. To achieve the best visual quality in mixed reality capture videos, it's better to set the `XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT` in the projection layer's `layerFlags`.  
  
**Performance Warning:** Omitting the `XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT` flag on the single projection layer will result in runtime post-processing which comes at a significant performance penalty.

# Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.microsoft.com.

When you submit a pull request, a CLA-bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., label, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
