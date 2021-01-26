# native-camera-vulkan

<p align="center">
	<img src="doc/res/cpp-logo.svg"> <img src="doc/res/vulkan-logo.svg"> <img src="doc/res/android-logo.svg">
</p>

> Native android application that showcases camera preview mapping on a spinning 3D cube.

Native android application developed with the use of Vulkan API that performs real-time camera preview mapping on a spinning 3D cube. The aim of the project is to provide with an elaborate example that showcases various useful, according to the author, basic-to-intermediate developing techniques for the creation of Vulkan-powered native android applications that make use of multiple hardware facilities.

## Table of contents

- [Project Description](#project-description)
	+ [Pre-requisites](#pre-requisites)
	+ [Application Architecture](#application-architecture)
		* [Application Loop](#application-loop)
		* [Vulkan Context](#vulkan-context)
		* [Graphics Pipeline](#graphics-pipeline)
	+ [Vulkan C++ API and NDK](#vulkan-c-api-and-ndk)
	+ ["*Java-less*" Native Application](#java-less-native-application)
	+ [Camera Preview via External Buffers](#camera-preview-via-external-buffers)
- [Build Instructions](#build-instructions)
- [References](#references)
- [Contributing](#contributing)
- [Donations](#donations)
- [License](#license)

## Project Description
[![To TOC](doc/res/toc.svg)](#table-of-contents)

This side-project started as an effort to refresh and update my C++ and 3D graphics programming skills. The initial purpose was the study of the latest C++ standards (C++17 and "C++20") as well as the study of Vulkan library. After an introductory review, it became apparent that Vulkan is mainly used by C rather than C++ developers although the Vulkan C++ library (https://github.com/KhronosGroup/Vulkan-Hpp) is constantly gaining popularity. Also the latest developments on mobile graphics and camera hardware along the fact that Vulkan runs on Android devices, drove me down the path of exploring the possibilities of building system applications for Android (outside the JVM context) in order to take advantage of the Vulkan graphics and compute capabilities as well as other third-party system libraries (e.g. OpenCV, dlib, TensorFlow) for my domains of interest. These domains being Computer Vision, Computer Graphics and Augmented Reality. 

By further studying and working on this idea, I also discovered that there aren't much resources out there on how to integrate the camera into a 3D native Android application using "immediate memory mapping" (external buffers). So I've decided to build a single-threaded (well if you don't count the JVM context threads) Vulkan sample that showcases the following:

- Usage of C++17, Vulkan Hpp and the latest NDK (see [Pre-requisites](#pre-requisites)) for building a native Android 3D application.

- Management of Android user permissions outside the JVM context (i.e. without the use of JNI calls).

- Design of a clean program loop that manages user input, sensors (e.g. accelerometer, camera) and screen output.

- Design of a basic Vulkan context for 3D rendering.

- Integration of camera data ([Camera2 API](https://developer.android.com/ndk/reference/group/camera)) via the communication of external hardware buffers to the Vulkan context.

In the references section, I list all sources that guided me in realizing this project.

### Pre-requisites

The codebase of the project targets intermediate-level C++ programmers with some knowledge of the C++17 standard. The project is made in Android Studio with the aid of CMake. Thus, knowledge of this IDE and building tools (CMake, Gradle, AVD & SDK Managers) is required. Finally, native-camera-vulkan makes use of some external to NDK binary and header libraries, so the developer must configure the development environment in such a way (acquiring needed libraries, configure locations etc.), as to meet the following. 

<table>
<tr><td colspan="2" align="center" width="900">Development Tools</td></tr>
<tr><td><a href="https://developer.android.com/studio">Android Studio</a></td><td>v4.1.2</td></tr>
<tr><td><a href="https://developer.android.com/ndk">NDK</a></td><td>v22.0.7026061</td></tr>
<tr><td><a href="https://developer.android.com/studio/releases/platform-tools">SDK Platform</a></td><td>v30.3</td></tr>
<tr><td><a href="https://developer.android.com/studio/releases/build-tools">SDK Build-Tools</a></td><td>v30.0.3</td></tr>
<tr><td colspan="2" align="center">External (to AS) Development Tools</td></tr>
<tr><td><a href="https://cmake.org/">CMake</a></td><td>v3.19.2</td></tr>    
<tr><td colspan="2" align="center">External (to NDK) Libraries</td></tr>
<tr><td><a href="https://github.com/KhronosGroup/Vulkan-Headers">Vulkan</a></td><td>v1.2.162</td></tr>
<tr><td><a href="https://github.com/KhronosGroup/Vulkan-Hpp">Vulkan HPP</a></td><td>v1.2.162</td></tr>
<tr><td><a href="https://github.com/KhronosGroup/Vulkan-ValidationLayers">Validation Layers</a></td><td>v1.2.162</td></tr>
<tr><td><a href="https://www.boost.org/">Boost</a></td><td>v1.74.0</td></tr>
<tr><td><a href="https://github.com/g-truc/glm">GLM</a></td><td>v0.9.9.8</td></tr>
<tr><td><a href="https://github.com/nothings/stb">STB</a></td><td>v2.26 (stb_image)</td></tr>    
</table>



For more information see [Vulkan C++ API and NDK](#vulkan-c-api-and-ndk) and [Build Instructions](#build-instructions).

### Application Architecture

### Vulkan C++ API and NDK

The Android NDK used in this project (see [Pre-requisites](#pre-requisites)), ships with an outdated version of the Vulkan headers (v1.2.121). These headers do not correspond to a Vulkan HPP library version. So it is decided that for the purposes of the development, a newer version of the libraries as well as of the validation layers will be used (v1.2.162). In order to do so, besides including the newer headers in the project, the corresponding binaries of the validation layers had to also be introduced as dependencies in Gradle, see [Build-Instructions](#build-instructions) for more details.

### "*Java-less*" Native Application
### Camera Preview via External Buffers
## Build Instructions
[![To TOC](doc/res/toc.svg)](#table-of-contents)

## References
[![To TOC](doc/res/toc.svg)](#table-of-contents)

## Contributing
[![To TOC](doc/res/toc.svg)](#table-of-contents)
## Donations
[![To TOC](doc/res/toc.svg)](#table-of-contents)
## License
[![To TOC](doc/res/toc.svg)](#table-of-contents)