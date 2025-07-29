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
#include <vulkan-cpp/utilities.hpp>

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
        .severity = vk::message::verbose | vk::message::warning | vk::message::error,
        // .message_type essentially takes in vk::debug. Like:
        // vk::debug::general, vk::debug::validation, vk::debug::performance
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

    vk::queue_indices queue_indices = physical_device.family_indices();
    std::println("Graphics Queue Family Index = {}", queue_indices.graphics);
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
    vk::surface window_surface(api_instance, window);
    std::println("Starting implementation of the swapchain!!!");


    vk::surface_enumeration surface_properties = vk::enumerate_surface(physical_device, window_surface);

    if(surface_properties.format.format != VK_FORMAT_UNDEFINED) {
        std::println("Surface Format.format is not undefined!!!");
    }

    
    vk::swapchain_enumeration enumerate_swapchain_settings = {
        .width = (uint32_t)width,
        .height = (uint32_t)height,
        .present_index = physical_device.family_indices().graphics, // presentation index just uses the graphics index
    };
    vk::swapchain main_swapchain(logical_device, window_surface, enumerate_swapchain_settings, surface_properties);

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
    std::vector<vk::image> swapchain_images;
    std::vector<vk::sampled_image> swapchain_depth_images;
    swapchain_images.resize(image_count);
    swapchain_depth_images.resize(image_count);

    VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;
    
    // Setting up the images
    uint32_t layer_count = 1;
    uint32_t mip_levels = 1;
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vk::swapchain_image_enumeration enumerate_image_properties = {
            .image = images[i],
            .format = surface_properties.format.format,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .layer_count = 1,
            .mip_levels = 1
        };
        swapchain_images[i] = create_image2d_view(logical_device, enumerate_image_properties);

        // Creating Depth Images for depth buffering
        // VkImageUsageFlagBits usage =
        //     VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkMemoryPropertyFlagBits property_flags =
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        vk::image_enumeration depth_image_enumeration = {
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .format = depth_format,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT
        };

        uint32_t memory_type_index = vk::image_memory_requirements(physical_device, logical_device, swapchain_images[i]);
        swapchain_depth_images[i] = create_depth_image2d(logical_device, depth_image_enumeration, memory_type_index);
    }

    // setting up command buffers
    std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
    for (size_t i = 0; i < swapchain_command_buffers.size(); i++) {
        vk::command_enumeration settings = {
            enumerate_swapchain_settings.present_index,
            vk::command_levels::primary,
            // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
            vk::command_pool_flags::reset
        };

        swapchain_command_buffers[i] = vk::command_buffer(logical_device, settings);
    }

    std::println("Command Buffers Created with size() = {}", swapchain_command_buffers.size());

    // setting up renderpass

    // setting up attachments for the renderpass
    VkAttachmentDescription color_attachment = {
        .flags = 0,
        .format = surface_properties.format.format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentDescription depth_attachment = {
        .flags = 0,
        .format = depth_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    std::array<VkAttachmentDescription, 2> attachments = {
        color_attachment, depth_attachment
    };

    VkAttachmentReference color_attachment_ref = {
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depth_attachment_reference = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass_description = {
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_ref,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment =
            &depth_attachment_reference, // enable depth buffering
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    std::array<VkSubpassDescription, 1> subpass_desc = {
        subpass_description
    };

    vk::renderpass_attachments main_attachments {
        .attachments = attachments,
        .subpass_descriptions = subpass_desc
    };

    vk::renderpass main_renderpass(logical_device, main_attachments);

    std::println("renderpass created!!!");

    // Setting up swapchain framebuffers
    std::vector<VkFramebuffer> swapchain_framebuffers;
    swapchain_framebuffers.resize(image_count);

    for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {
        std::vector<VkImageView> image_view_attachments;
        image_view_attachments.push_back(swapchain_images[i].view);
        image_view_attachments.push_back(swapchain_depth_images[i].view);

        VkFramebufferCreateInfo framebuffer_ci = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = main_renderpass,
            .attachmentCount =
                static_cast<uint32_t>(image_view_attachments.size()),
            .pAttachments = image_view_attachments.data(),
            .width = swapchain_extent.width,
            .height = swapchain_extent.height,
            .layers = 1
        };

        vk::vk_check(vkCreateFramebuffer(logical_device,
                                        &framebuffer_ci,
                                        nullptr,
                                        &swapchain_framebuffers[i]),
                    "vkCreateFramebuffer");
    }

    std::println("Created VkFramebuffer's with size = {}", swapchain_framebuffers.size());

    // setting up presentation queue to display commands to the screen
    vk::queue_enumeration enumerate_present_queue {
        .family = 0,
        .index =0
    };
    vk::device_present_queue presentation_queue(logical_device, main_swapchain, enumerate_present_queue);
    
    std::array<float, 4> color = {0.f, 0.5f, 0.5f, 1.f};

    // Set window background color
    // VkClearColorValue renderpass_color = {
    //     { color.at(0), color.at(1), color.at(2), color.at(3) }
    // };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t current_frame = presentation_queue.acquire_next_image();
        vk::command_buffer current = swapchain_command_buffers[current_frame];

        // std::array<VkClearValue, 2> clear_values = {};

        // clear_values[0].color = renderpass_color;
        // clear_values[1].depthStencil = { 1.f, 0 };

        // VkRenderPassBeginInfo renderpass_begin_info = {
        //     .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		// 	.pNext = nullptr,
		// 	.renderPass = main_renderpass,
		// 	.renderArea = {
		// 		.offset = {
		// 			.x = 0,
		// 			.y = 0
		// 		},
		// 		.extent = {
		// 			.width = swapchain_extent.width,
		// 			.height = swapchain_extent.height
		// 		},
		// 	},
		// 	.clearValueCount = static_cast<uint32_t>(clear_values.size()),
		// 	.pClearValues = clear_values.data()
        // };

        current.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        // VkViewport viewport = {
        //     .x = 0.0f,
        //     .y = 0.0f,
        //     .width = static_cast<float>(swapchain_extent.width),
        //     .height = static_cast<float>(swapchain_extent.height),
        //     .minDepth = 0.0f,
        //     .maxDepth = 1.0f,
        // };

        // vkCmdSetViewport(current, 0, 1, &viewport);

        // VkRect2D scissor = {
        //     .offset = { 0, 0 },
        //     .extent = { swapchain_extent.width, swapchain_extent.height },
        // };

        // vkCmdSetScissor(current, 0, 1, &scissor);

        // renderpass_begin_info.framebuffer =
        //   main_swapchain.active_framebuffer(current);
        // renderpass_begin_info.framebuffer = swapchain_framebuffers[current_frame];
        vk::renderpass_begin_info begin_renderpass = {
            .current_command = current,
            .extent = swapchain_extent,
            .current_framebuffer = swapchain_framebuffers[current_frame],
            .color = color,
            .subpass = vk::subpass_contents::inline_bit
        };
        main_renderpass.begin(begin_renderpass);

        // vkCmdBeginRenderPass(current,
        //                      &renderpass_begin_info,
        //                      VK_SUBPASS_CONTENTS_INLINE);
        
        // vkCmdEndRenderPass(current);
        main_renderpass.end(current);
        current.end();

        presentation_queue.submit_async(current);
        presentation_queue.present_frame(current_frame);
    }

    // TODO: Make the cleanup much saner. For now we are cleaning it up like this to ensure they are cleaned up in the proper order
    logical_device.wait();
    main_swapchain.destroy();

    for(auto& command : swapchain_command_buffers) {
        command.destroy();
    }

    for(auto& fb : swapchain_framebuffers) {
        vkDestroyFramebuffer(logical_device, fb, nullptr);
    }

    for(auto& img : swapchain_images) {
        // vk::free_image(logical_device, img);
        vkDestroyImageView(logical_device, img.view, nullptr);
    }

    for(auto& depth_img : swapchain_depth_images) {
        vk::free_image(logical_device, depth_img);
    }

    main_renderpass.destroy();
    presentation_queue.destroy();

    logical_device.destroy();
    window_surface.destroy();
    glfwDestroyWindow(window);
    api_instance.destroy();
    return 0;
}
