# Vulkan-CPP

Vulkan abstraction layer, that uses modern C++ at an attempt to simplify the development with vulkan.

# Overview

This does not mean that the instnace, physical, and logical devices are handled for you. You still would need to create those. This abstracion allows for a simpler design as to how those creation get done. Allowing you to focus on the specification of those creation without much of the verbose of the vulkan's API.

This API focuses on simplicity, but providing enough information to still be flexible in your own application. Whether you want to build a device management system that understands your hardware specifications and support multiple GPU's configurations for multi-viewport. To even having a single logical device for simpler use.

That is the purpose as to why I decided to make vulkan-cpp, to focus on usability and simplicity. While introducing minimal code to get something working if possible.

## How to Build

This project uses the C++ conan package manager to manage dependencies. Reference to [getting started](https://engine3d-dev.github.io/0.1/getting_started) to setup the development environment.

## Demos

The demos are meant to act as the tutorial in using vulkan to write basic primitives or use the ones provided by vulkan-cpp.

`vulkan-cpp` still requires you to know how to use vulkan in some capacity. The reason I made this was to help with making some modifications into my own projects with handling specific things with vulkan, I find bug-prone.

These demos also act as a way to show how to utilize vulkan-cpp from rendering a basic triangle to a more complex renderer objects. Such as textures, skybox's, etc.

## Shader Samples

The demos use the shader samples provided. Shader samples are samples that are used by the same official vulkan tutorial guide.

Here are which demos uses the shader samples

* sample 1 -- used by demo 6 and 7
* sample 2 -- used by demo 8
* sample 3 -- used by demo 9 (descriptors + camera data)
* sample 4 -- used by demo 10 (textures)