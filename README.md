# OpenXR Samples for Mixed Reality Developers

This repository contains OpenXR code samples tailored for developers who are familiar with and using the Visual Studio toolchain, e.g. HoloLens 2 developers.

These OpenXR samples use C++17 and D3D11. The same source code works across UWP applications running on HoloLens 2 and Win32 applications running on Windows Desktop with Windows Mixed Reality immersive headsets.

# Prepare, build and run the samples

- Understand [what is OpenXR](https://docs.microsoft.com/windows/mixed-reality/openxr#what-is-openxr) and [why OpenXR](https://docs.microsoft.com/windows/mixed-reality/openxr#why-openxr).  Read the [latest OpenXR 1.0 spec (HTML)](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html) and the [latest openxr.h header file (Khronos GitHub)](https://github.com/KhronosGroup/OpenXR-SDK/blob/master/include/openxr/openxr.h).

- Update to [Windows 10 May 2019 Update (1903) or later](https://www.microsoft.com/software-download/windows10) and [Visual Studio 2017 or 2019](https://visualstudio.microsoft.com/downloads/).  If you will be deploying to a HoloLens 2, you should install [Visual Studio 2019 16.2 or later](https://visualstudio.microsoft.com/downloads/).

- Prepare a [HoloLens 2 device](https://docs.microsoft.com/windows/mixed-reality/openxr#getting-started-with-openxr-for-hololens-2) or [Windows Mixed Reality device](https://docs.microsoft.com/windows/mixed-reality/openxr#getting-started-with-openxr-for-windows-mixed-reality-headsets).

- Clone the samples repo: `git clone https://github.com/microsoft/OpenXR-MixedReality.git`

    - The repo has been renamed from `OpenXR-SDK-VisualStudio` since 03/06/2020.  If you cloned the repo before this, redirect your local repo using this command:
     `git remote set-url origin https://github.com/microsoft/OpenXR-MixedReality.git`

- Open the `BasicXrApp.sln` or `Samples.sln` file in Visual Studio. F5 to build and run the sample. You typically choose ARM64 platform when running on HoloLens 2 devices, or choose x64 platform when running on a Windows Desktop PC with the HoloLens 2 Emulator or a Windows Mixed Reality immersive headset (or simulator).

- The core OpenXR API usage patterns can be found in the [OpenXRProgram.cpp](https://github.com/microsoft/OpenXR-MixedReality/blob/master/samples/BasicXrApp/OpenXrProgram.cpp) file. The Run() function at the beginning captures a typical OpenXR app code flow for session initialization, event handling, the frame loop and input actions.

# OpenXR app best practices for HoloLens 2

The [BasicXrApp](https://github.com/microsoft/OpenXR-MixedReality/tree/master/samples/BasicXrApp) demonstrates the best practices for an OpenXR app to achieve full frame rate and low latency.

For more detailed information on getting the best visual quality and performance on HoloLens 2, see the [best practices for HoloLens 2](https://aka.ms/openxr-best).

# OpenXR preview extensions

The [openxr_preview](https://github.com/microsoft/OpenXR-MixedReality/tree/master/openxr_preview) folder contains a set of [preview header files](https://github.com/microsoft/OpenXR-MixedReality/tree/master/openxr_preview/include/openxr) containing the following preview OpenXR extensions:

1. [XR_MSFT_hand_tracking_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_hand_tracking_preview)
1. [XR_MSFT_hand_tracking_mesh_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_hand_tracking_mesh_preview)
1. [XR_MSFT_secondary_view_configuration_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_secondary_view_configuration_preview)
1. [XR_MSFT_first_person_observer_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_first_person_observer_preview)
1. [XR_MSFT_spatial_graph_bridge_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_spatial_graph_bridge_preview)
1. [XR_MSFT_controller_model_preview](https://microsoft.github.io/OpenXR-MixedReality/openxr_preview/specs/openxr.html#XR_MSFT_controller_model_preview)

For sample code demonstrating how to use the preview extensions above, see the [SampleSceneUwp](https://github.com/microsoft/OpenXR-MixedReality/tree/master/samples/SampleSceneUwp), [ThreeCubes](https://github.com/microsoft/OpenXR-MixedReality/tree/master/samples/ThreeCubesUwp) and [XrSceneLib](https://github.com/microsoft/OpenXR-MixedReality/tree/master/samples/XrSceneLib) preview extensions.  Please file feedback on these preview extensions as [GitHub issues](https://github.com/microsoft/OpenXR-MixedReality/issues).  We are planning to incorporate your feedback and finalize these extensions as vendor extensions (MSFT) or cross-vendor extensions (EXT) in the central Khronos OpenXR [headers](https://github.com/KhronosGroup/OpenXR-SDK/tree/master/include/openxr) and [spec](https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html) over the coming months.

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
