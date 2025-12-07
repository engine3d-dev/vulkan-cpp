# Vulkan-CPP

Modern abstraction layer for Vulkan using C++23 to simplify and modernize development for graphical applications.

![NOTE]
> vulkan-cpp assumes you have some knowledge of computer graphics and API's such as with OpenGL or Direct3D

# Why another abstraction around Vulkan?

I chose to make this vulkan abstraction for developing graphics applications using Vulkan much simpler. This involved providing ways for specifying operations that had quite a bit of boilerplate being done in raw Vulkan.

Examples of these are renderpass attachments and setting up descriptor set handles. These take up extroadinary amount of code to implement. Though there are not always a one size fit all situation.


## How to Build

For building our projects we use Conan, the C++ package manager to manage our dependencies and build vulkan-cpp demos.

Required to start at the [getting started](https://engine3d-dev.github.io/0.1/getting_started) page for setting up the development environment.

## Example Demos

The demos are meant to reflect closely enough to the Vulkan tutorial documentation site. Where they show you how to learn how to use Vulkan.

These demos are supposed to enable you in learning more about how vulkan-cpp works, and how you can effectively build a renderer using the current API's of vulkan-cpp.

## Shader Samples

These are shader samples used by the specific demos that you see referenced below. Which demo utilizes those specific shader samples.

* sample 1 -- used by demo 6 and 7
* sample 2 -- used by demo 8
* sample 3 -- used by demo 9 (descriptors + camera uniforms)
* sample 4 -- used by demo 10 (textures)