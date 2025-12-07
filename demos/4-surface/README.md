# Demo 4 -- Creating a surface

Vulkan is a platform-agnostic API, it cannot interface directly to the windowing system on its own. In this demo, I show an example on how to establish the connectino between Vulkan and the windowing system to display to the screen.

In these demos, I use GLFW. There are already tutorials out there that show how to set Vulkan with other windowing systems such as SDL, etc. If you do not choose GLFW.

To use a windowing system, be sure to enable the WSI (Window System Integration) extension. In this demo, one of the extensions I show how to enable is `VK_KHR_surface`.

The `VK_KHR_surface` is an instance-level extension. This has already been enabled in the previous demos.

In the demo code, I query for the surface extensions using the `glfwGetRequiredInstanceExtensions` API provided by GLFW.

As it also includes other WSI extensions that will be used in the next upcoming demos.

# Creating a Window Surface

The window surface needs to be created right after the instance creation because it does influence the physical device selection.

Reason why the creation of the vulkan surface was postponed later is because it is part of a larger topic of render targets and presentation.

Which would have clutter the the setup needed to get vulkan instantiated.

In vulkan-cpp, it is quite simple to create a vulkan surface.

# Code to Creating a Window Surface

## To setup GLFW

Because vulkan-cpp uses GLFW. This section, I will show how to setup GLFW.

Call the `glfwInit` API to initialize GLFW. It returns a status code if it errored out in initializing GLFW.

```C++
if(!glfwInit()) {
    return -1;
}
```

Basics in setting up the GLFW window is the following:

```C++

int width = 900;
int height = 600;

GLFWwindow* window = glfwCreateWindow(width, height, "Hello Vulkan", nullptr, nullptr);

```

## Creating Vulkan Surface Handle

Creating a vulkan surface after vulkan instance is required. The vulkan-cpp's equivalent, which is `vk::surface`.

Currently, I dont have a way to differentiate between multiple windowing systems as GLFW is the default this `vk::surface` was developed only to expect a `GLFWwindow`.

To create a surface, simply put:

Now, that creating the GLFW window is created

```C++

// assume these steps are done
// ...
vk::instance api_instance(....);

// ...
GLFWwindow* window_handle = glfwCreateWindow(...);

vk::surface window_surface(api_instance, window);

```

## Queue Families

What vulkan-cpp has is two different representation for queue's.

`vk::device_present_queue` - specifically for queues that you will be used for displaying to the screen
`vk::device_queue` - used for submitting tasks such as command buffers to offload commands to the GPU.

When dealing with queues in vulkan there is the family of the specific kinds of queues that are available.

Then there are the indices of the specific queue within that family, you may want to use.

## Creating a device_queue

To create the `vk::device_queue` in this demo you will set the `queue_params` structs.

Currently the API is still in development in how we handle queue families. vulkan-cpp uses physical devices to query queue families and their indices based on whats compatible with the current physical device.

To create a device_queue, set the parameters to the first family. Then to select the graphics queue index to indicate the graphics queue, you would like to select for presenting to the screen.

```C++

vK::queue_indices indices = physical_device.family_indices();
vk::queue_params present_queue_params = {
    .family = 0,
    .index = indices.graphics,
};
```

Then to create the actual device_queue, is simple.

Just do:

```C++

vk::device_queue present_queue(logical_device, present_queue_params);

```

# Final Code

This is additional final code that you should have added additionally to the code from the previous demos.

```C++

vk::surface window_surface(api_instance, window);

// Presentation queue family uses graphics queue
vk::queue_params present_queue_enumerate = {
    .family = 0,
    .index = queue_indices.graphics,
};
vk::device_queue presesnt_queue(logical_device, present_queue_enumerate);
```

# Last Minute Notes

Common patterns you will see often is any object that you create with vulkan-cpp, will always require a logical device.

This is common because the logical device is responsible for handling allocations,destruction, and managing the vulkan children objects.

This is why vulkan requires that all vulkan objects are cleaned up rather then having the cleanup be performed after the logical device, yet it must be before.

Logical devices already manage the device queue's, therefore you do not need to manage that yourself.