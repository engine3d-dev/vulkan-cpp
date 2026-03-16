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
#include <filesystem>
import vk;

#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <tiny_obj_loader.h>
import environment_map;

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

struct global_uniform {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

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

    // Setting up vk instance
    vk::instance api_instance(config, debug_callback_info);

    if (api_instance.alive()) {
        std::println("\napi_instance alive and initiated!!!");
    }

    vk::physical_enumeration enumerate_devices{
        .device_type = vk::physical_gpu::discrete,
    };

    // Specifically set for the mac m1 series platform
#if defined(__APPLE__)
    enumerate_devices.device_type = vk::physical_gpu::integrated;
#endif

    vk::physical_device physical_device(api_instance, enumerate_devices);

    // selecting depth format
    std::array<vk::format, 3> format_support = {
        vk::format::d32_sfloat,
        vk::format::d32_sfloat_s8_uint,
        vk::format::d24_unorm_s8_uint
    };

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
    std::println("Starting implementation of the swapchain!!!");

    vk::surface_params surface_properties =
      vk::enumerate_surface(physical_device, window_surface);

    if (surface_properties.format.format != VK_FORMAT_UNDEFINED) {
        std::println("Surface Format.format is not undefined!!!");
    }

    vk::swapchain_params enumerate_swapchain_settings = {
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
    std::span<const VkImage> images = main_swapchain.get_images();
    uint32_t image_count = static_cast<uint32_t>(images.size());

    // Creating Images
    std::vector<vk::sample_image> swapchain_images(image_count);
    std::vector<vk::sample_image> swapchain_depth_images(image_count);

    VkExtent2D swapchain_extent = surface_properties.capabilities.currentExtent;

    // Setting up the images
    uint32_t layer_count = 1;
    uint32_t mip_levels = 1;
    for (uint32_t i = 0; i < swapchain_images.size(); i++) {
        vk::image_params swapchain_image_config = {
            .extent = { .width = swapchain_extent.width,
                        .height = swapchain_extent.height },
            .format = surface_properties.format.format,
            .aspect = vk::image_aspect_flags::color_bit,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };

        swapchain_images[i] =
          vk::sample_image(logical_device, images[i], swapchain_image_config);

        // Creating Images for depth buffering
        vk::image_params image_config = {
            .extent = { .width = swapchain_extent.width,
                        .height = swapchain_extent.height },
            .format = depth_format,
            .aspect = vk::image_aspect_flags::depth_bit,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .mip_levels = 1,
            .layer_count = 1,
            .phsyical_memory_properties = physical_device.memory_properties(),
        };
        swapchain_depth_images[i] =
          vk::sample_image(logical_device, image_config);
    }

    // setting up command buffers
    std::vector<vk::command_buffer> swapchain_command_buffers(image_count);
    for (size_t i = 0; i < swapchain_command_buffers.size(); i++) {
        vk::command_params settings = {
            .levels = vk::command_levels::primary,
            .queue_index = enumerate_swapchain_settings.present_index,
            .flags = vk::command_pool_flags::reset,
        };

        swapchain_command_buffers[i] =
          vk::command_buffer(logical_device, settings);
    }

    // setting up attachments for the renderpass
    std::array<vk::attachment, 2> renderpass_attachments = {
        // color attachment
        vk::attachment{
          .format = surface_properties.format.format,
          .layout = vk::image_layout::color_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::store,
          .stencil_load = vk::attachment_load::dont_care,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::present_src_khr,
        },
        // depth attachment
        vk::attachment{
          .format = depth_format,
          .layout = vk::image_layout::depth_stencil_optimal,
          .samples = vk::sample_bit::count_1,
          .load = vk::attachment_load::clear,
          .store = vk::attachment_store::dont_care,
          .stencil_load = vk::attachment_load::dont_care,
          .stencil_store = vk::attachment_store::dont_care,
          .initial_layout = vk::image_layout::undefined,
          .final_layout = vk::image_layout::depth_stencil_read_only_optimal,
        },
    };

    vk::renderpass main_renderpass(logical_device, renderpass_attachments);

    std::println("renderpass created!!!");

    // Setting up swapchain framebuffers

    std::vector<vk::framebuffer> swapchain_framebuffers(image_count);
    for (uint32_t i = 0; i < swapchain_framebuffers.size(); i++) {

        // NOTE: This must match the amount of attachments the renderpass also
        // has to match the image_view attachment for per-framebuffers as well
        // I just set the size to whatever the renderpass attachment size are to
        // ensure this is the case Since you have an image for color attachment
        // and another image for the depth atttachment to specify
        std::array<VkImageView, renderpass_attachments.size()>
          image_view_attachments = { swapchain_images[i].image_view(),
                                     swapchain_depth_images[i].image_view() };

        vk::framebuffer_params framebuffer_info = {
            .renderpass = main_renderpass,
            .views = image_view_attachments,
            .extent = swapchain_extent
        };
        swapchain_framebuffers[i] =
          vk::framebuffer(logical_device, framebuffer_info);
    }

    std::println("Created VkFramebuffer's with size = {}",
                 swapchain_framebuffers.size());

    // setting up presentation queue to display commands to the screen
    vk::queue_params enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };
    
    std::vector<std::string> faces = {
        "asset_samples/skybox/right.jpg",
        "asset_samples/skybox/left.jpg",
        "asset_samples/skybox/top.jpg",
        "asset_samples/skybox/bottom.jpg",
        "asset_samples/skybox/front.jpg",
        "asset_samples/skybox/back.jpg"
    };
    environment_map skybox = environment_map(logical_device, std::filesystem::path("asset_samples/skybox/monkstown_castle_4k.hdr"), physical_device.memory_properties(), main_renderpass);


    // editor camera properties
    float field_of_view = 45.f;
    glm::vec3 position = { 3.5f, 4.90f, 36.40f};
    glm::vec3 scale{1.f};
    glm::vec2 plane = {0.1f, 5000.f};

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
       static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(
                       current_time - start_time)
                       .count();
    //    global_uniform ubo = {
    //         .model = glm::rotate(glm::mat4(1.0f),
    //                              time * glm::radians(90.0f),
    //                              glm::vec3(0.0f, 0.0f, 1.0f)),
    //         .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
    //                             glm::vec3(0.0f, 0.0f, 0.0f),
    //                             glm::vec3(0.0f, 0.0f, 1.0f)),
    //         .proj = glm::perspective(glm::radians(90.f),
    //                                  (float)swapchain_extent.width /
    //                                    (float)swapchain_extent.height,
    //                                  0.1f,
    //                                  10.0f)
    //     };


        // if(glfwGetKey(main_window, GLFW_KEY))
        // glm::vec3 up = glm::rotate(to_quaternion, atlas::math::up());
        // glm::vec3 forward = glm::rotate(to_quaternion, atlas::math::backward());
        // glm::vec3 right = glm::rotate(to_quaternion, atlas::math::right());
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            position.z += 1.f;
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            position.x += 1.f;
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            position.z -= 1.f;
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            position.x -= 1.f;
        }
        global_uniform ubo = {};
        ubo.proj = glm::mat4(1.f);
        ubo.proj = glm::perspective(glm::radians(field_of_view), static_cast<float>(swapchain_extent.width / swapchain_extent.height), plane.x, plane.y);
        ubo.proj[1][1] *= -1;

        ubo.view = glm::mat4(1.f);
        ubo.view = glm::translate(ubo.view, position);
        ubo.view = glm::inverse(ubo.view);

        skybox_uniform sky_ubo = {
            .proj_view = ubo.proj * glm::mat4(glm::mat3(ubo.view)),
        };
        skybox.update_uniform(sky_ubo);

        skybox.bind(current);
        skybox.draw(current);

        main_renderpass.end(current);
        current.end();

        // Submitting and then presenting to the screen
        std::array<const VkCommandBuffer, 1> commands = { current };
        presentation_queue.submit_async(commands);
        presentation_queue.present_frame(current_frame);
    }

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

    for (auto& depth_img : swapchain_depth_images) {
        depth_img.destroy();
    }

    skybox.destroy();
    main_renderpass.destroy();
    presentation_queue.destroy();

    logical_device.destroy();
    window_surface.destroy();
    glfwDestroyWindow(window);
    api_instance.destroy();
    return 0;
}