#include <array>
#include <print>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <GLFW/glfw3.h>
#include <vulkan-cpp/utilities.hpp>
#include <vulkan-cpp/instance.hpp>
#include <vulkan-cpp/physical_device.hpp>
#include <vulkan-cpp/device.hpp>
#include <vulkan-cpp/device_queue.hpp>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(
  [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT p_message_severity,
  [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT p_message_type,
  const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
  [[maybe_unused]] void* p_user_data) {
    std::print("validation layer:\t\t{}\n\n", p_callback_data->pMessage);
    return false;
}

std::vector<const char*>
initialize_instance_extensions() {
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

int
main() {
    //! @note Just added the some test code to test the conan-starter setup code
    if (!glfwInit()) {
        fmt::print("glfwInit could not be initialized!\n");
        return -1;
    }

    if (!glfwVulkanSupported()) {
        std::print("GLFW: Vulkan is not supported!");
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    int width = 800;
    int height = 600;
    std::string title = "Hello Window";
    GLFWwindow* window =
      glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    
    glfwMakeContextCurrent(window);

    std::array<const char*, 1> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    // setting up extensions
    std::vector<const char*> global_extensions =
      initialize_instance_extensions();

    vk::debug_message_utility debug_callback_info = {
        .severity = vk::message::verbose | vk::message::warning | vk::message::error,
        .message_type = vk::debug::general | vk::debug::validation | vk::debug::performance,
        .callback = debug_callback
    };

    vk::application_configuration config = {
        .name = "vulkan instance",
        .version = vk::api_version::vk_1_3, // specify to using vulkan 1.3
        .validations = validation_layers, // .validation takes in a std::span<const char*>
        .extensions = global_extensions // .extensions also takes in std::span<const char*>
    };

    // 1. Setting up vk instance
    vk::instance api_instance(config, debug_callback_info);

    if(api_instance.alive()) {
        std::println("\napi_instance alive and initiated!!!");
    }


    // TODO: Implement this as a way to setup physical devices
    // vk::enumerate_physical_devices(vk::instance) -> returns std::span<vk::physical_device>

    // setting up physical device
    vk::physical_enumeration enumerate_devices {
        .device_type = vk::physical::discrete
    };
    vk::physical_device physical_device(api_instance, enumerate_devices);

    // selecting depth format
    std::array<VkFormat, 3> format_support = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    // We provide a selection of format support that we want to check is supported on current hardware device.
    VkFormat depth_format = vk::select_depth_format(physical_device, format_support);

    if(depth_format != VK_FORMAT_UNDEFINED) {
        std::println("Depth format specifically was able to be found!!!");
    }

    vk::queue_indices queue_indices = physical_device.family_indices();
    std::println("Graphics Queue Family Index = {}", (int)queue_indices.graphics);
    std::println("Compute Queue Family Index = {}", queue_indices.compute);
    std::println("Transfer Queue Family Index = {}", queue_indices.transfer);


    // setting up logical device
    std::array<float, 1> priorities = {0.f};
    std::array<const char*, 1> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vk::device_enumeration logical_device_enumeration = {
        .queue_priorities = priorities,
        .extensions = extensions,
        .queue_family_index = 0,
    };

    vk::device logical_device(physical_device, logical_device_enumeration);

    // Presentation queue family uses graphics queue
    vk::queue_enumeration present_queue_enumerate = {
        .family = 0,
        .index = queue_indices.graphics,
    };
    vk::device_queue presesnt_queue(logical_device, present_queue_enumerate);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    logical_device.wait();
    logical_device.destroy();

    glfwDestroyWindow(window);
    api_instance.destroy();
    return 0;
}
