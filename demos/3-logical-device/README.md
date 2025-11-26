# Demo 3 -- Creating a Logical Device

After selecting a physical device. This demo will show you how to setup a `logical device`.

Creating a logical device is similar to the way you create a vulkan instance. Where you have sets of parameters that you may want to specify during the creation process of the vulkan logical device.

To select a logical device is the following:
* Select specific queue family (specify a uint32_t value)
* Queue priority -- because vulkan lets you assign priorities to influence how commands executions are executed
* Creating the actual vulkan device


Create an array that specify the queue priority, which setting to `0.f` is fine for the demos.

```C++
std::array<float, 1> priorities = { 0.f };
```

Once you specify the queue priorities, you can set the logical device extensions to `VK_KHR_SWAPCHAIN_EXTENSION_NAME`. 

```C++
std::array<const char*, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
```

# Extensions

In Vulkan, extensions are ways to enable specific vulkan functionality that can be enabled during your application's runtime.

## Instance Extensions

For `VkInstance` or `vk::instance`, you can enable functionality that is globally or instance-level wide. Outside of the vulkan core specifications.

## Device Extension

These extensions can add new device-level functionality to the API, outside of the core specification. Meaning you can query extensions that you are able to enable in the.


# Creating the Logical Device

After creating your two arrays, you can now call `vk::device_params` to set the queue_priorities, extensions, and the queue_family index to 0 (zero).

```C++
vk::device_params logical_device_params = {
    .queue_priorities = priorities,
    .extensions = extensions,
    .queue_family_index = 0,
};
```

Actual creation of the logical device is the following:

```C++

vk::device logical_device(physical_device, logical_device_params);

```

# Final Code

The entire code should look like:

```C++
std::array<float, 1> priorities = { 0.f };
std::array<const char*, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

vk::device_params logical_device_params = {
    .queue_priorities = priorities,
    .extensions = extensions,
    .queue_family_index = 0,
};

vk::device logical_device(physical_device, logical_device_params);
```

`vk::device` also provides an API `.alive()` function to make sure if the VkDevice handle is valid for use, which you can use for testing if its been created successfully.

# Thats it!

You've created a vulkan logical device.