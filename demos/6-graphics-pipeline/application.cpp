#define GLFW_INCLUDE_VULKAN
#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#endif

#include <array>
#include <print>
#include <span>
#include <vector>
#include <expected>

import vk;

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
get_instance_extensions() {
    std::vector<const char*> extension_names;
    uint32_t extension_count = 0;
    const char** required_extensions =
      glfwGetRequiredInstanceExtensions(&extension_count);

    for (uint32_t i = 0; i < extension_count; i++) {
        std::println("Required Extension = {}", required_extensions[i]);
        extension_names.emplace_back(required_extensions[i]);
    }

    extension_names.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#if defined(__APPLE__)
    extension_names.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    extension_names.emplace_back(
      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

    return extension_names;
}

int
main() {
    //! @note Just added the some test code to test the conan-starter setup code
    if (!glfwInit()) {
        std::println("glfwInit could not be initialized!");
        return -1;
    }

    if (!glfwVulkanSupported()) {
        std::println("GLFW: Vulkan is not supported!");
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
    std::vector<const char*> global_extensions = get_instance_extensions();

    vk::debug_message_utility debug_callback_info = {
        .severity =
          vk::message::verbose | vk::message::warning | vk::message::error,
        .message_type =
          vk::debug::general | vk::debug::validation | vk::debug::performance,
        .callback = debug_callback
    };

    vk::application_params config = {
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

    // Selecting a specific physical device
    std::expected<vk::physical_device, VkResult> physical_device_expected =
      api_instance.enumerate_physical_device(vk::physical_gpu::integrated);
    vk::physical_device physical_device = physical_device_expected.value();

    // selecting depth format
    std::array<vk::format, 3> format_support = {
        vk::format::d32_sfloat,
        vk::format::d32_sfloat_s8_uint,
        vk::format::d24_unorm_s8_uint
    };

    // We provide a selection of format support that we want to check is
    // supported on current hardware device.
    VkFormat depth_format =
      physical_device.request_depth_format(format_support);

    // setting up logical device
    std::array<float, 1> priorities = { 0.f };

#if defined(__APPLE__)
    std::array<const char*, 2> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                                              "VK_KHR_portability_subset" };
#else
    std::array<const char*, 1> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
#endif

    vk::device_params logical_device_params = {
        .queue_priorities = priorities,
        .extensions = extensions,
        .queue_family_index = 0,
    };

    vk::device logical_device(physical_device, logical_device_params);

    vk::surface window_surface(api_instance, window);

    vk::surface_params surface_properties =
      physical_device.request_surface(window_surface);

    vk::swapchain_params enumerate_swapchain_settings = {
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
        .present_index = 0
    };
    vk::swapchain main_swapchain(logical_device,
                                 window_surface,
                                 enumerate_swapchain_settings,
                                 surface_properties);

    // querying swapchain images
    std::span<const VkImage> images = main_swapchain.get_images();
    uint32_t image_count = static_cast<uint32_t>(images.size());

    // Creating images from vk::swapchain
    std::vector<vk::sample_image> swapchain_images(image_count);

    VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;

    // Setting up the images
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vk::image_params swapchain_image_config = {
            .extent = {
                .width = swapchain_extent.width,
                .height = swapchain_extent.height,
            },
            .format = surface_properties.format.format,
            .memory_mask = physical_device.memory_properties(
              vk::memory_property::device_local_bit),
            .aspect = vk::image_aspect_flags::color_bit,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .mip_levels = 1,
            .layer_count = 1,
        };

        swapchain_images[i] =
          vk::sample_image(logical_device, images[i], swapchain_image_config);
    }

    // setting up command buffers
    std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
    for (size_t i = 0; i < swapchain_command_buffers.size(); i++) {
        vk::command_params settings = {
            .levels = vk::command_levels::primary,
            .queue_index = 0,
            .flags = vk::command_pool_flags::reset,
        };

        swapchain_command_buffers[i] =
          vk::command_buffer(logical_device, settings);
    }

    // Configuring a renderpass
    // setting up attachments for the renderpass
    std::array<vk::attachment, 1> renderpass_attachments = {
        vk::attachment{
          .format = surface_properties.format.format,
          .layout = vk::image_layout::color_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::store,
          .stencil_load = vk::attachment_load::clear,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
    };

    vk::renderpass main_renderpass(logical_device, renderpass_attachments);

    // Setting up swapchain framebuffers
    std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
    for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {

        // NOTE: This must match the amount of attachments the renderpass also
        // has to match the image_view attachment for per-framebuffers as well
        // I just set the size to whatever the renderpass attachment size are to
        // ensure this is the case Since you have an image for color attachment
        // and another image for the depth atttachment to specify
        std::array<VkImageView, renderpass_attachments.size()>
          image_view_attachments = { swapchain_images[i].image_view() };

        vk::framebuffer_params framebuffer_info = {
            .renderpass = main_renderpass,
            .views = image_view_attachments,
            .extent = swapchain_extent
        };
        swapchain_framebuffers[i] =
          vk::framebuffer(logical_device, framebuffer_info);
    }

    // setting up presentation queue to display commands to the screen
    vk::queue_params enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };

    // Now creating a vulkan graphics pipeline for the shader loading
    std::array<vk::shader_source, 2> shader_sources = {
        vk::shader_source{
          .filename = "shader_samples/sample1/test.vert.spv",
          .stage = vk::shader_stage::vertex,
        },
        vk::shader_source{
          .filename = "shader_samples/sample1/test.frag.spv",
          .stage = vk::shader_stage::fragment,
        },
    };

    // To render triangle, we do not need to set any vertex attributes
    vk::shader_resource_info shader_info = {
        .sources = shader_sources,
        .vertex_attributes = {} // this is to explicitly set to none, but also
                                // dont need to set this at all regardless
    };
    vk::shader_resource geometry_resource(logical_device, shader_info);

    // Setting up descriptor sets for graphics pipeline
    std::array<vk::color_blend_attachment_state, 1> color_blend_attachments = {
        vk::color_blend_attachment_state{},
    };

    std::array<vk::dynamic_state, 2> dynamic_states = {
        vk::dynamic_state::viewport, vk::dynamic_state::scissor
    };
    vk::pipeline_params pipeline_configuration = {
        .renderpass = main_renderpass,
        .shader_modules = geometry_resource.handles(),
        .vertex_attributes = geometry_resource.vertex_attributes(),
        .vertex_bind_attributes = geometry_resource.vertex_bind_attributes(),
        .color_blend = {
            .attachments = color_blend_attachments,
        },
        .depth_stencil_enabled = true,
        .dynamic_states = dynamic_states,
    };
    vk::pipeline main_graphics_pipeline(logical_device, pipeline_configuration);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t current_frame = presentation_queue.acquire_next_image();
        vk::command_buffer current = swapchain_command_buffers[current_frame];

        current.begin(vk::command_usage::simulatneous_use_bit);

        // renderpass begin/end must be within a recording command buffer
        vk::renderpass_begin_params begin_renderpass = {
            .extent = swapchain_extent,
            .current_framebuffer = swapchain_framebuffers[current_frame],
            .color = color,
            .subpass = vk::subpass_contents::inline_bit
        };
        main_renderpass.begin(current, begin_renderpass);

        // Binding a graphics pipeline -- before drawing stuff
        // Inside of this graphics pipeline bind, is where you want to do the
        // drawing stuff to
        main_graphics_pipeline.bind(current);

        // Drawing-call to render actual triangle to the screen
        vkCmdDraw(current, 3, 1, 0, 0);

        main_renderpass.end(current);
        current.end();

        // Submitting and then presenting to the screen
        std::array<const VkCommandBuffer, 1> commands = { current };
        presentation_queue.submit_async(commands);
        presentation_queue.present_frame(current_frame);
    }

    // this to ensure they are cleaned up in the proper order
    logical_device.wait();
    main_swapchain.destruct();

    for (auto& command : swapchain_command_buffers) {
        command.destruct();
    }

    for (auto& fb : swapchain_framebuffers) {
        fb.destruct();
    }

    for (auto& image : swapchain_images) {
        image.destruct();
    }

    main_graphics_pipeline.destruct();
    geometry_resource.destruct();
    main_renderpass.destruct();
    presentation_queue.destruct();

    logical_device.destruct();
    window_surface.destruct();
    glfwDestroyWindow(window);
    return 0;
}