# Vulkan-CPP

Vulkan abstraction layer, that uses modern C++ at an attempt to simplify the development with vulkan.

# Overview

This does not mean that the instnace, physical, and logical devices are handled for you. You still would need to create those. This abstracion allows for a simpler design as to how those creation get done. Allowing you to focus on the specification of those creation without much of the verbose of the vulkan's API.

This API focuses on simplicity, but providing enough information to still be flexible in your own application. Whether you want to build a device management system that understands your hardware specifications and support multiple GPU's configurations for multi-viewport. To even having a single logical device for simpler use.

That is the purpose as to why I decided to make vulkan-cpp, to focus on usability and simplicity. While introducing minimal code to get something working if possible.

## How to Build

This project uses the C++ conan package manager to manage dependencies. Reference to [getting started](https://engine3d-dev.github.io/0.1/getting_started) to setup the development environment.

The setup is the same across all vulkan-related projects in the atlas organization.

## Examples

The examples are still currently in the works, here are some examples of the potential API designs for vulkan-cpp to get working.

### Setting up Vulkan Instance

This code example can be used for setting up the vulkan API with `vk::instance`, including setting up application-related information the users should be concerned with.

```C++

#include <vulkan-cpp/vk_instance.hpp>

// dummy debug callback
static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity
  VkDebugUtilsMessageTypeFlagsEXT messageType
  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData
  void* pUserData
);

std::vector<const char*> initialize_instance_extensions() {
  std::vector<const char*> extension_names;

  extension_names.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

  // An additional surface extension needs to be loaded. This extension is
  // platform-specific so needs to be selected based on the platform the
  // example is going to be deployed to. Preprocessor directives are used
  // here to select the correct platform.
#ifdef VK_USE_PLATFORM_WIN32_KHR
        extension_names.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
        extensionNames.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
        extensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
        extensionNames.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        extensionNames.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
        extensionNames.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
#ifdef USE_PLATFORM_NULLWS
        extensionNames.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif

        return extension_names;
}

int main() {
  // setting up validation layers
  std::array<const char*, 2> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
  };

  // setting up extensions
  std::span<const char*> global_extensions = initialize_instance_extensions();

  vk::debug_message_utility debug_callback_info = {
    // .severity essentially takes in vk::message::verbose, vk::message::warning, vk::message::error
    .severity = vk::verbose | vk::warning | vk::error,
    // .message_type essentially takes in vk::debug. Like: vk::debug::general, vk::debug::validation, vk::debug::performance
    .message_type = vk::general | vk::validation | vk::performance
    .callback = debug_callback
  };

  vk::application_configuration config = {
    .app_name = "Vulkan-HelloWorld",
    .version = vk::version::vk_1_3, // specify to using vulkan 1.3
    .validation = validation_layers // .validation takes in a std::span<const char*>
    .extensions = global_extensions // .extensions also takes in std::span<const char*>
  };

  vk::instance init_vk_instance = vk::instance(config, debug_message_utility);

  // If you need to get the validation layers currently supported, this is how you do it.
  // If built in debug mode, this will actually contain data, if built in release. The std::span<vk::layer_properties> .empty() would be true.
  std::span<vk::layer_properties> available_validation_layers = init_vk_instance.validation_layers();

  // do stuff with available_validation_layers
}
```

## Setup Vulkan Physical and Logical Devices

In this example, showing what are configuration settings that are only concerned as part of setting up the vulkan physical and logical devices.

These configurations involve configuring what the vulkan API should look for as far the physical devices are concerns and what is currently available on your current machine.

As for logical devices, configurations are for specifying properties such as queue families, queue indices, and specifying what queue handlers are available based on queue family specifications.

```C++

int main() {
  // 1. Setting up vk::instance
  // Assuming the VkInstance is already handled beforehand
  vk::instance init_vk_instance = vk::instance(config, debug_message_utility);

  // 2. Create a physical device

  // 2.1 Sets up physical device specifications for what kind of VkPhysical handler to create with
  // You can create multiple physical devices with vary specifications, if needed.
  vk::physical_driver_configuration phys_config = {
    // Used to retrieve specific vulkan physical device based on your hardware specifications
    // you can also do vk::physical::integrated, vk::physical::virtual,
    // vk::physical::cpu, vk::physical::max_enum, defaults to vk::physical::other
    .device_type = vk::physical::discrete
  };

  vk::physical_driver phys_device = vk::physical_driver(init_vk_instance, phys_config);

  // 3. Create a logical device
  std::array<const char*, 1> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
  };

  // 3.1 Selecting queue priority to schedule command execution
  std::array<float, 1> queue_priority = {0.f};
  vk::schedule_queue device_queue = {
    .family_index = 0,
    .priority = queue_priority
  };

  // Setting up only one device queue for this example
  std::array<vk::schedule_queue, 1> device_queues_arr = {
    device_queue
  };

  // 3.2 Configuring logical device before creation
  vk::device_configuration logical_device_config = {
    .extension = device_extensions, // .extension is a std::span<const char*>
    .queue = device_queues_arr // takes in a std::span<vk::schedule_queue>
  };

  // 3.3 Creates and instantiates the actual logical device handler with minimal specifications
  vk::driver logical_device = vk::driver(phys_device, logical_device_config);

  // 3.4 Example of showing how to create a graphics queue
  vk::queue_info queue_settings = {
    .family_index = 0,
    .index = 0
  };

  // This I am still not too sure if we may need a separate implementation, possible because I was going to detail them differently (still up in the air)
  vk::graphics_queue single_graphics_queue(queue_settings); // creating single graphics queue
  vk::transfer_queue single_transfer_queue(queue_settings); // creating single transfer queue
  vk::compute_queue single_compute_queue(queue_settings); // creating single compute queue
}
```

 ## Setting up Vulkan Surface (with GLFW) 

 This code example shows how to utilize `vk::surface`. Include minimal code example for setting the vulkan surface with the GLFW API.

 This code involves showing how to setup a vulkan surface, specifying the configuration parts of the code to setup the vulkan surface successfully. This code assumes the vulkan instance, physical, and logical devices are already created prior.

 ```C++

int main() {
  if(glfwInit()) {
    // print if something went wrong
    return -1;
  }

  GLFWwindow* window_handle = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
  // Creates and instantiates a VkSurfaceKHR under the hood, also allows to use vk::surface as a `VkSurfaceKHR` handler itself.
  vk::surface surface_for_window = vk::surface(window_handle);

  // If you do not want to manually invoke it in your abstractions, you can also do an invokation to a callback to resource free and specify when these freed callbacks get invoked in your application
  vk::defer_resource_free([&surface_for_window](){
    surface_for_window.destroy();
    init_vk_instance.destroy();
  });

  // Eventually, I'd want to do:
  // This way if you have something such as swapchain resizability, you can do immediate invokations. Though this may not even be needed, and you could just invoke .destroy() or manually destroy themself
  // NOTE: If this was to be called into a class of some kind
  vk::immediate_resource_free(this, [&surface_for_window](){
    logical_device.destroy();
    surface_for_window.destroy();
  });

  while(!glfwWindowShouldClose(window_handle)) {

    glfwPollEvents();
  }

  // This invokes explicitly freeing the vulkan resources instantiated
  init_vk_instance.destroy();
  surface_for_window.destroy();
  glfwDestroyWindow(window_handle);

  // or you can just invoke resource_free
  vk::resource_free();
}
```

## Setup Vulkan Swapchain

This code example is for setting up the swapchain. There are some things that needs to happen to get the base working swapchain up and running.

Here are the things that needs to happen:

* Initiate `vk::swapchain` handler.
* Initiate `vector<vk::image>` to correspond frames-in-flight.
* Initiate `vector<vk::framebuffer>`.
* Initiate `vector<vk::command_buffer>`
* Then specify attachments for the renderpass
* Initiate `vk::renderpass` with attachments pre-defined.

### vk::image

For the creation of the swapchain, we are going to create `vk::image` for this code tutorial, the next tutorial will cover how to create a renderpass.

In Vulkan when you create images, each `vk::image` contains `VkImage` and `VkImageView`. `VkImageView` in vulkan, is most the handler that will be referenced throughout Vulkan's API, because `VkImage` is never directly invoked by any of the API's.


```C++

int main() {
  if(glfwInit()) {
    // print if something went wrong
    return -1;
  }

  GLFWwindow* window_handle = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
  // Creates and instantiates a VkSurfaceKHR under the hood, also allows to use vk::surface as a `VkSurfaceKHR` handler itself.
  vk::surface surface_for_window = vk::surface(window_handle);

  // If you do not want to manually invoke it in your abstractions, you can also do an invokation to a callback to resource free and specify when these freed callbacks get invoked in your application
  vk::defer_resource_free([&surface_for_window](){
    surface_for_window.destroy();
    init_vk_instance.destroy();
  });
  // Eventually, I'd want to do:
  // This way if you have something such as swapchain resizability, you can do immediate invokations. Though this may not even be needed, and you could just invoke .destroy() or manually destroy themself
  vk::immediate_resource_free(this, [&surface_for_window](){
    logical_device.destroy();
    surface_for_window.destroy();
  });

  vk::present_queue presentation_queue(queue_settings); // creating single graphics queue
  std::array<uint32_t, 1> queue_family_indices  { present_index };
  vk::swapchain_configuration swapchain_config = {
    .image = {
      .min_count = request_min_count,
      .format = surface_for_window.colorspace(),
      .usage = (vk::image::color_attachment | vk::image::transfer_dst) // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = vk::image::color_attachment, VK_IMAGE_USAGE_TRANSFER_DST_BIT = vk::image::transfer_dst
    },
    .extent = surface_for_window.current_extent(),
    .surface = surface_for_window,
    .present_queue = presentation_queue,
    .queue_family_indices = queue_family_indices // .queue_family_indices = std::span<uint32_t>
    .composite_alpha = vk::composite:opaque,
    .mode = vk::present::immediate // equivalent to doing .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR
  };

  vk::swapchain window1_swapchain = vk::swapchain(swapchain_config);
  std::span<vk::image> swapchain_images = window1_swapchain.images();
  uint32_t image_size = read_image_size(window1_swapchain);

  // Creating a vk::image that contains `VkImage` and `VkImageView`
  std::vector<vk::image> images(image_size);
  for(size_t i = 0; i < images.size(); i++) {
    images[i].image = swapchain_images[i];
    image_view_properties properties {
      .format = window1_swapchain.surface_format(),
      .aspect_flags = vk::aspect::color_bit
    };
    images[i].image_view = create_image2d_view(logical_device, images[i].image, properties);
  }

  while(!glfwWindowShouldClose(window_handle)) {
    uint32_t current_image_frame = window1_swapchain.acquired_next_image();
    vk::image current_image = images[current_image_frame];

    glfwPollEvents();
  }

  // This invokes explicitly freeing the vulkan resources instantiated
  init_vk_instance.destroy();
  surface_for_window.destroy();
  glfwDestroyWindow(window_handle);

  // or you can just invoke resource_free
  // This just does vkDeviceWaitIdle(logical_device); or we can directly invoke it as vkDeviceWaitIdle(logical_device) as well
  vk::device_wait_idle(logical_device);
  vk::resource_free();
}
```


## Setup Vulkan Renderpass

Renderpass in Computer graphics is a list of operations. These operations are referred to as attachments which tell each renderpass how to handle the objects when they are getting rendered.
