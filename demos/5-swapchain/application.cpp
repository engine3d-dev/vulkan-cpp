#include <array>
#include <print>

#include <vulkan-cpp/imports.hpp>

#define FMT_HEADER_ONLY
#include <fmt/format.h>
#include <vulkan-cpp/utilities.hpp>
#include <vulkan-cpp/instance.hpp>
#include <vulkan-cpp/physical_device.hpp>
#include <vulkan-cpp/device.hpp>
#include <vulkan-cpp/device_queue.hpp>
#include <vulkan-cpp/surface.hpp>
#include <vulkan-cpp/swapchain.hpp>
#include <vulkan-cpp/device_present_queue.hpp>
#include <vulkan-cpp/command_buffer.hpp>
#include <vulkan-cpp/renderpass.hpp>
#include <vulkan-cpp/framebuffer.hpp>
#include <vulkan-cpp/sample_image.hpp>

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
        // .severity essentially takes in vk::message::verbose,
        // vk::message::warning, vk::message::error
        .severity =
          vk::message::verbose | vk::message::warning | vk::message::error,
        // .message_type essentially takes in vk::debug. Like:
        // vk::debug::general, vk::debug::validation, vk::debug::performance
        .message_type =
          vk::debug::general | vk::debug::validation | vk::debug::performance,
        .callback = debug_callback
    };

    vk::application_configuration config = {
        .name = "vulkan instance",
        .version = vk::api_version::vk_1_3, // specify to using vulkan 1.3
        .validations =
          validation_layers, // .validation takes in a std::span<const char*>
        .extensions =
          global_extensions // .extensions also takes in std::span<const char*>
    };

    // 1. Setting up vk instance
    vk::instance api_instance(config, debug_callback_info);

    if (api_instance.alive()) {
        std::println("\napi_instance alive and initiated!!!");
    }

    // TODO: Implement this as a way to setup physical devices
    // vk::enumerate_physical_devices(vk::instance) -> returns
    // std::span<vk::physical_device>

    // setting up physical device
    vk::physical_enumeration enumerate_devices{ .device_type =
                                                  vk::physical::discrete };
    vk::physical_device physical_device(api_instance, enumerate_devices);

    // selecting depth format
    std::array<VkFormat, 3> format_support = { VK_FORMAT_D32_SFLOAT,
                                               VK_FORMAT_D32_SFLOAT_S8_UINT,
                                               VK_FORMAT_D24_UNORM_S8_UINT };

    // We provide a selection of format support that we want to check is
    // supported on current hardware device.
    VkFormat depth_format =
      vk::select_depth_format(physical_device, format_support);

    vk::queue_indices queue_indices = physical_device.family_indices();
    std::println("Graphics Queue Family Index = {}", queue_indices.graphics);
    std::println("Compute Queue Family Index = {}", queue_indices.compute);
    std::println("Transfer Queue Family Index = {}", queue_indices.transfer);

    // setting up logical device
    std::array<float, 1> priorities = { 0.f };
    std::array<const char*, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    vk::device_enumeration logical_device_enumeration = {
        .queue_priorities = priorities,
        .extensions = extensions,
        .queue_family_index = 0,
    };

    vk::device logical_device(physical_device, logical_device_enumeration);

    vk::surface window_surface(api_instance, window);
    std::println("Starting implementation of the swapchain!!!");

    vk::surface_enumeration surface_properties =
      vk::enumerate_surface(physical_device, window_surface);

    if (surface_properties.format.format != VK_FORMAT_UNDEFINED) {
        std::println("Surface Format.format is not undefined!!!");
    }

    vk::swapchain_enumeration enumerate_swapchain_settings = {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .present_index =
          physical_device.family_indices()
            .graphics, // presentation index just uses the graphics index
    };
    vk::swapchain main_swapchain(logical_device,
                                 window_surface,
                                 enumerate_swapchain_settings,
                                 surface_properties);

    // querying swapchain images
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(logical_device,
                            main_swapchain,
                            &image_count,
                            nullptr); // used to get the amount of images
    std::vector<VkImage> images(image_count);
    vkGetSwapchainImagesKHR(logical_device,
                            main_swapchain,
                            &image_count,
                            images.data()); // used to store in the images

    // Creating Images
    std::vector<vk::sample_image> swapchain_images(image_count);
    std::vector<vk::sample_image> swapchain_depth_images(image_count);

    VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;

    // Setting up the images
    uint32_t layer_count = 1;
    uint32_t mip_levels = 1;
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vk::image_configuration_information swapchain_image_config = {
            .extent = {swapchain_extent.width, swapchain_extent.width},
            .format = surface_properties.format.format,
            .aspect = vk::image_aspect_flags::color_bit,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };
        // swapchain_images[i] = create_image2d_view(logical_device, enumerate_image_properties);
        swapchain_images[i] = vk::sample_image(logical_device, images[i], swapchain_image_config);

        // Creating Depth Images for depth buffering
        vk::image_configuration_information depth_image_config = {
            .extent = {swapchain_extent.width, swapchain_extent.width},
            .format = depth_format,
            .aspect = vk::image_aspect_flags::depth_bit,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };
        swapchain_depth_images[i] = vk::sample_image(logical_device, depth_image_config);
    }

    // setting up command buffers
    std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
    for (size_t i = 0; i < swapchain_command_buffers.size(); i++) {
        vk::command_enumeration settings = {
            .levels = vk::command_levels::primary,
            .queue_index = enumerate_swapchain_settings.present_index,
            .flags = vk::command_pool_flags::reset,
        };

        swapchain_command_buffers[i] =
          vk::command_buffer(logical_device, settings);
    }

    // setting up renderpass

    // setting up attachments for the renderpass
    std::array<vk::attachment, 2> renderpass_attachments = {
        vk::attachment{
          .format = surface_properties.format.format,
          .layout = vk::image_layout::color_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::dont_care,
          .stencil_load = vk::attachment_load::clear,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
        vk::attachment{
          .format = depth_format,
          .layout = vk::image_layout::depth_stencil_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::dont_care,
          .stencil_load = vk::attachment_load::clear,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
    };

    vk::renderpass main_renderpass(logical_device, renderpass_attachments);

    std::println("renderpass created!!!");

	std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
	for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
		std::array<VkImageView, 2> image_view_attachments = {
			swapchain_images[i].image_view(),
			swapchain_depth_images[i].image_view()
		};

		vk::framebuffer_settings framebuffer_info = {
			.renderpass = main_renderpass,
			.views = image_view_attachments,
			.extent = swapchain_extent
		};
		swapchain_framebuffers[i] = vk::framebuffer(logical_device, framebuffer_info);
	}

    std::println("Created VkFramebuffer's with size = {}",
                 swapchain_framebuffers.size());

    // setting up presentation queue to display commands to the screen
    vk::queue_enumeration enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t current_frame = presentation_queue.acquire_next_image();
        vk::command_buffer current = swapchain_command_buffers[current_frame];

        current.begin(vk::command_usage::simulatneous_use_bit);

        // renderpass begin/end must be within a recording command buffer
        vk::renderpass_begin_info begin_renderpass = {
            .current_command = current,
            .extent = swapchain_extent,
            .current_framebuffer = swapchain_framebuffers[current_frame],
            .color = color,
            .subpass = vk::subpass_contents::inline_bit
        };
        main_renderpass.begin(begin_renderpass);

        main_renderpass.end(current);
        current.end();

        // Submitting and then presenting to the screen
        presentation_queue.submit_async(current);
        presentation_queue.present_frame(current_frame);
    }

    // TODO: Make the cleanup much saner. For now we are cleaning it up like
    // Potentially bring back submit_resource_free([this](){ .. free stuff ..
    // }); (???)
    // this to ensure they are cleaned up in the proper order
    logical_device.wait();
    main_swapchain.destroy();

    for (auto& command : swapchain_command_buffers) {
        command.destroy();
    }

    for (auto& fb : swapchain_framebuffers) {
        fb.destroy();
    }

    for (auto& image : swapchain_images) {
        image.destroy();
    }

    for (auto& depth_image : swapchain_depth_images) {
        depth_image.destroy();
    }

    main_renderpass.destroy();
    presentation_queue.destroy();

    logical_device.destroy();
    window_surface.destroy();
    glfwDestroyWindow(window);
    api_instance.destroy();
    return 0;
}
