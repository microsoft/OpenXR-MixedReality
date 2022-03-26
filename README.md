# OpenXR Samples for Mixed Reality Developers

This repository contains OpenXR code samples tailored for developers who are familiar with and using the Visual Studio toolchain, e.g. HoloLens 2 developers.

These OpenXR samples use C++17 and D3D11. The same source code works across UWP applications running on HoloLens 2 and Win32 applications running on Windows Desktop with Windows Mixed Reality immersive headsets.

# Prepare, build and run the samples

- Understand [what is OpenXR](https://docs.microsoft.com/windows/mixed-reality/openxr#what-is-openxr)
and [why OpenXR](https://docs.microsoft.com/windows/mixed-reality/openxr#why-openxr).
Read the [latest OpenXR 1.0 spec (HTML)](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html)
and the [latest openxr.h header file (Khronos GitHub)](https://github.com/KhronosGroup/OpenXR-SDK/blob/master/include/openxr/openxr.h).

- Update to [Windows 10 May 2019 Update (1903) or later](https://www.microsoft.com/software-download/windows10)
and [Visual Studio 2017 or 2019](https://visualstudio.microsoft.com/downloads/).
If you will be deploying to a HoloLens 2, you should install [Visual Studio 2019 16.2 or later](https://visualstudio.microsoft.com/downloads/).

- Prepare a [HoloLens 2 device](https://docs.microsoft.com/windows/mixed-reality/openxr#getting-started-with-openxr-for-hololens-2)
or [Windows Mixed Reality device](https://docs.microsoft.com/windows/mixed-reality/openxr#getting-started-with-openxr-for-windows-mixed-reality-headsets).

- Clone the samples repo: `git clone https://github.com/microsoft/OpenXR-MixedReality.git`

- Open the `BasicXrApp.sln` or `Samples.sln` file in Visual Studio. F5 to build and run the sample.
You typically choose ARM64 platform when running on HoloLens 2 devices,
or choose x64 platform when running on a Windows Desktop PC with the HoloLens 2 Emulator or a Windows Mixed Reality immersive headset (or simulator).

# OpenXR preview extensions

The [openxr_preview](https://github.com/microsoft/OpenXR-MixedReality/tree/main/openxr_preview) folder contains a set of [preview header files](https://github.com/microsoft/OpenXR-MixedReality/tree/main/openxr_preview/include/openxr) containing the following OpenXR extensions that are only available [in preview runtime](http://aka.ms/openxr-preview).

1. [XR_MSFT_spatial_graph_bridge](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_spatial_graph_bridge) Revision 2 adds the `XrSpatialGraphNodeBindingMSFT` handle and `xrTryCreateSpatialGraphStaticNodeBindingMSFT` function so that applications can try to get a spatial graph node from an `XrSpace` handle.

Please send feedback on preview extensions and samples at [GitHub issues](https://github.com/microsoft/OpenXR-MixedReality/issues).
We are planning to incorporate your feedback and finalize these extensions as vendor extensions (MSFT) or cross-vendor extensions (EXT)
in the central Khronos OpenXR [headers](https://github.com/KhronosGroup/OpenXR-SDK/tree/master/include/openxr)
and [spec](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html)
over the coming months.

# OpenXR samples and extension usages

- **The core OpenXR API usage patterns** <br/>
can be found in the [BasicXrApp/OpenXRProgram.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/BasicXrApp/OpenXrProgram.cpp) file.
The [Run() function](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/BasicXrApp/OpenXrProgram.cpp#L28)
captures a typical OpenXR app code flow for session initialization, event handling, the frame loop and input actions.

- **Hand tracking** <br/>
The [Scene_HandTracking.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/SampleSceneUwp/Scene_HandTracking.cpp)
demos the usage of [XR_EXT_hand_tracking](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_hand_tracking)
and [XR_MSFT_hand_tracking_mesh](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_hand_tracking_mesh) extensions.

- **Eye tracking** <br/>
The [Scene_EyeGazeInteraction.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/EyeGazeInteractionUwp/Scene_EyeGazeInteraction.cpp) file
demos the usage of [XR_EXT_eye_gaze_interaction](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_EXT_eye_gaze_interaction) extension.

- **Mixed reality capture support** <br/>
Search "secondary" in [XrApp.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/shared/XrSceneLib/XrApp.cpp) file
to understand the usage of [XR_MSFT_secondary_view_configuration](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_secondary_view_configuration)
and [XR_MSFT_first_person_observer](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_first_person_observer) extensions.

- **Render the motion controller model** <br/>
The [ControllerObject.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/shared/XrSceneLib/ControllerObject.cpp) and
[Scene_ControllerModel.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/SampleSceneWin32/Scene_ControllerModel.cpp) files
demos the usage of [XR_MSFT_controller_model](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_controller_model) extension.

- **Holographic window attachment** <br/>
The [SampleSceneUwp/Main.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/SampleSceneUwp/Main.cpp) file demos protocol activation for XR apps,
and using [XR_MSFT_holographic_window_attachment](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_holographic_window_attachment) extension
to interop with CoreWindow in order to support [keyboard inputs](https://docs.microsoft.com/en-us/windows/mixed-reality/keyboard-mouse-and-controller-input-in-directx#subscribe-for-corewindow-input-events)
and [TextEditingContext](https://docs.microsoft.com/en-us/uwp/api/Windows.UI.Text.Core.CoreTextEditContext?view=winrt-19041).

- **Understand the local, unbounded and anchor spaces**<br/>
The [ThreeSpacesUwp](https://github.com/microsoft/OpenXR-MixedReality/blob/main/samples/ThreeSpacesUwp/Scene_ThreeSpaces.cpp) project
demos the usage and differences of [LOCAL](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#reference-spaces),
[UNBOUNDED](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_unbounded_reference_space) reference spaces
and [spatial anchors](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_spatial_anchor).

* The sample code for [XR_MSFT_spatial_graph_bridge](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#XR_MSFT_spatial_graph_bridge) is coming soon ...


# OpenXR app best practices for HoloLens 2

The [BasicXrApp](https://github.com/microsoft/OpenXR-MixedReality/tree/main/samples/BasicXrApp) demonstrates the best practices for an OpenXR app to achieve full frame rate and low latency.

For more detailed information on getting the best visual quality and performance on HoloLens 2, see the [best practices for HoloLens 2](https://aka.ms/openxr-best).

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
