# Vulkan-CPP

Custom Vulkan abstraction layer with native C++20 modules support using LLVM.

> [!TIP]
> vulkan-cpp assumes you have some knowledge of graphics APIs such as OpenGL or Direct3D

## Getting Started

Before buidling the demos, make sure to check the [getting started](https://engine3d-dev.github.io/0.1/getting_started) page beforehand.

## Building Demos

These demos are isolated and follow the Vulkan tutorial, just using the vulkan-cpp APIs. Building the demo is quite easy, assuming you already setup the development environment in the getting started page.

To build any demo, do the following:

> [!NOTE]
> `-s build_type=Debug` to build in debug mode is optional

```bash
conan atlas build demos/<demo-dir> -s build_type=Debug
```

For example, to build demo 6:

```bash
conan atlas build demos/6-graphics-pipeline -s build_type=Debug
```

## Running the Demos

The build directory will be located in the demo that you built.

Executable path will be as the following:

```bash
./demo/<demo-dir>/build/Debug/<executable>
```

It is because we built demo 6, it would be:

```bash
./demo/6-graphics-pipeline/build/Debug/graphics-pipeline
```