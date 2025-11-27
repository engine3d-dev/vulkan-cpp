# Demo 5 -- Swapchains

This part of the demo is the most crucial and quite code-heavy to setup. There are quite a bit of steps to creating the swapchain.

Vulkan does not have a concept of "default framebuffer", therefore it requires an infrastructure known as a `swapchain`. Which must be explicitly created in Vulkan.

Swapchain is essentially a queue of images that are waiting to get displayed to the screen. This is a requirement if you want you application to render anything to the screen.

General purpose of the swapchain is to synchronize the presentation of these images with the refresh rate of the screen.

In the vulkan tutorial, they mention you should check for suitability with the swapchain.

In vulkan-cpp, the only part you as the application developer need to worry about is what extensions you would like to enable for setting up the swapchain.

This demo will show how to setup the following to get a background color to the screen:
* Setting up vulkan images
* Setting up command buffers
* Setting up framebuffers
* Setting up renderpass
