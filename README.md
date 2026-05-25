# Vulkan-CPP

Custom Vulkan abstraction layer with native C++20 modules support using LLVM.

[![✅CI](https://github.com/engine3d-dev/vulkan-cpp/actions/workflows/ci.yml/badge.svg)](https://github.com/engine3d-dev/vulkan-cpp/actions/workflows/ci.yml)
[![GitHub stars](https://img.shields.io/github/stars/engine3d-dev/vulkan-cpp.svg)](https://github.com/engine3d-dev/vulkan-cpp/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/engine3d-dev/vulkan-cpp.svg)](https://github.com/engine3d-dev/vulkan-cpp/network)
[![GitHub issues](https://img.shields.io/github/issues/engine3d-dev/vulkan-cpp.svg)](https://github.com/engine3d-dev/vulkan-cpp/issues)

> [!TIP]
> vulkan-cpp assumes you have some knowledge of graphics APIs such as OpenGL or Direct3D

## Getting Started

Before buidling the demos, make sure to check the [getting started](https://engine3d-dev.github.io/0.1/getting_started) page beforehand.

## Building Demos

These demos are isolated and attempt at following the Vulkan tutorial, specifically to using the vulkan-cpp APIs. Building the demo is quite easy, assuming you already setup the development environment from the getting started page linked above.

Here is how to build any of the demos:

> [!NOTE]
> `-s build_type=Debug` to build in debug mode

```bash
conan atlas build demos/<demo-dir> -s build_type=Debug
```

Example for building demo 6:

```bash
conan atlas build demos/6-graphics-pipeline -s build_type=Debug
```

## Running the Demos

The build directory will be located in the demo that you built.

Executable path will be as the following:

```bash
./demo/<demo-dir>/build/Debug/<executable>
```

For demo 6 this is where the executable is located:

```bash
./demo/6-graphics-pipeline/build/Debug/graphics-pipeline
```