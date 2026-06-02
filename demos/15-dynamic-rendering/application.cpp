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
#include <expected>

#include <chrono>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <tiny_obj_loader.h>

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

struct global_uniform {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

template<typename T, typename... Rest>
void
hash_combine(size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed << 2);
    (hash_combine(seed, rest), ...);
}

namespace std {

    template<>
    struct hash<vk::vertex_input> {
        size_t operator()(const vk::vertex_input& vertex) const {
            size_t seed = 0;
            hash_combine(
              seed, vertex.position, vertex.color, vertex.normals, vertex.uv);
            return seed;
        }
    };
}

// Part of this demo for loading a 3D .obj model
class obj_model {
public:
    obj_model() = default;
    obj_model(const std::filesystem::path& p_filename,
              const VkDevice& p_device,
              const vk::physical_device& p_physical) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        //! @note If we return the constructor then we can check if the mesh
        //! loaded successfully
        //! @note We also receive hints if the loading is successful!
        //! @note Return default constructor automatically returns false means
        //! that mesh will return the boolean as false because it wasnt
        //! successful
        if (!tinyobj::LoadObj(&attrib,
                              &shapes,
                              &materials,
                              &warn,
                              &err,
                              p_filename.string().c_str())) {
            std::println("Could not load model from path {}",
                         p_filename.string());
            m_is_loaded = false;
            return;
        }

        std::vector<vk::vertex_input> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<vk::vertex_input, uint32_t> unique_vertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                vk::vertex_input vertex{};

                // vertices.push_back(vertex);
                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (index.normal_index >= 0) {
                    vertex.normals = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }

                if (index.texcoord_index >= 0) {
                    vertex.uv = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }

                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
        }

        m_has_indices = (indices.size() > 0) ? true : false;

        if (m_has_indices) {
            m_indices_size = indices.size();
        }
        m_indices_size = vertices.size();
        m_indices_size = indices.size();

        vk::buffer_parameters vertex_params = {
            .memory_mask = p_physical.memory_properties(
              vk::memory_property::device_local_bit),
            .usage = vk::buffer_usage::transfer_dst_bit |
                     vk::buffer_usage::vertex_buffer_bit,
        };

        vk::buffer_parameters index_params = {
            .memory_mask = p_physical.memory_properties(
              vk::memory_property::host_visible_bit |
              vk::memory_property::host_cached_bit),
            .usage = vk::buffer_usage::index_buffer_bit,
        };

        m_vertex_buffer = vk::vertex_buffer(p_device, vertices, vertex_params);
        m_index_buffer = vk::index_buffer(p_device, indices, index_params);
        m_is_loaded = true;
    }

    [[nodiscard]] bool loaded() const { return m_is_loaded; }

    [[nodiscard]] VkBuffer vertex_handle() const { return m_vertex_buffer; }

    [[nodiscard]] VkBuffer index_handle() const { return m_index_buffer; }

    [[nodiscard]] bool has_indices() const { return m_has_indices; }

    [[nodiscard]] uint32_t indices_size() const { return m_indices_size; }

    void draw(const VkCommandBuffer& p_command) {
        if (m_has_indices) {
            vkCmdDrawIndexed(p_command, m_indices_size, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(p_command, m_indices_size, 1, 0, 0);
        }
    }

    void destruct() {
        m_vertex_buffer.destruct();
        m_index_buffer.destruct();
    }

private:
    bool m_is_loaded = false;
    bool m_has_indices = false;
    uint32_t m_indices_size = 0;
    vk::vertex_buffer m_vertex_buffer{};
    vk::index_buffer m_index_buffer{};
};

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
        std::print("glfwInit could not be initialized!\n");
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
    std::vector<const char*> global_extensions = get_instance_extensions();

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

    // setting up physical device
    std::expected<vk::physical_device, VkResult> physical_device_expected =
      api_instance.enumerate_physical_device(vk::physical_gpu::integrated);
    vk::physical_device physical_device = physical_device_expected.value();

    // setting up logical device
    std::array<float, 1> priorities = { 0.f };

#if defined(__APPLE__)
    std::array<const char*, 2> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset",
    };
#else
    std::array<const char*, 1> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
#endif

    std::array<vk::format, 3> format_support = {
        vk::format::d32_sfloat,
        vk::format::d32_sfloat_s8_uint,
        vk::format::d24_unorm_s8_uint
    };

    // We provide a selection of format support that we want to check is
    // supported on current hardware device.
    VkFormat depth_format =
      physical_device.request_depth_format(format_support);

    vk::device_features device_features{
        vk::dynamic_rendering_feature{ {
          .dynamicRendering = true,
        } },
    };

    vk::device_params logical_device_params = {
        .features = device_features.data(),
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
        .present_index = 0,
    };
    vk::swapchain main_swapchain(logical_device,
                                 window_surface,
                                 enumerate_swapchain_settings,
                                 surface_properties);

    // querying presentable images
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
            .memory_mask = physical_device.memory_properties(
              vk::memory_property::device_local_bit),
            .aspect = vk::image_aspect_flags::color_bit,
            .usage = vk::image_usage::color_attachment_bit,
            .mip_levels = 1,
            .layer_count = 1,
        };

        swapchain_images[i] =
          vk::sample_image(logical_device, images[i], swapchain_image_config);

        vk::image_params image_config = {
            .extent = {
                .width = swapchain_extent.width,
                .height = swapchain_extent.height,
            },
            .format = depth_format,
            .memory_mask = physical_device.memory_properties(
              vk::memory_property::device_local_bit),
            .aspect = vk::image_aspect_flags::depth_bit,
            .usage = vk::image_usage::depth_stencil_bit,
            .mip_levels = 1,
            .layer_count = 1,
        };
        swapchain_depth_images[i] =
          vk::sample_image(logical_device, image_config);
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

    // setting up presentation queue to display commands to the screen
    vk::queue_params enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };

    // Loading graphics pipeline
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

    std::array<vk::color_blend_attachment_state, 1> color_blend_attachments = {
        vk::color_blend_attachment_state{},
    };

    std::array<vk::dynamic_state, 2> dynamic_states = {
        vk::dynamic_state::viewport, vk::dynamic_state::scissor
    };

    uint32_t format = static_cast<uint32_t>(surface_properties.format.format);
    vk::pipeline_params pipeline_configuration = {
        .use_render_pipeline = true,
        .color_attachment_formats = std::span<const uint32_t>(&format, 1),
        .depth_format = static_cast<uint32_t>(depth_format),
        .stencil_format = static_cast<uint32_t>(depth_format),
        .renderpass = nullptr,
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

    VkClearValue clear_color = {
        { 0.f, 0.5f, 0.5f, 1.f },
    };

    VkClearValue depth_value = {
        .depthStencil = { .depth = 1.f, .stencil = 0 },
    };

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        uint32_t current_frame = presentation_queue.acquire_next_image();
        vk::command_buffer current = swapchain_command_buffers[current_frame];

        current.begin(vk::command_usage::simulatneous_use_bit);

        swapchain_images[current_frame].memory_barrier(
          current,
          surface_properties.format.format,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        // Because dynamic rendering does not automatically handle layout
        // transitions These memory barriers set the color and depth images for
        // the output
        swapchain_depth_images[current_frame].memory_barrier(
          current,
          depth_format,
          VK_IMAGE_LAYOUT_UNDEFINED,
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
          VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

        vk::rendering_attachment color_render_attachment = {
            .image_view = swapchain_images[current_frame].image_view(),
            .layout = vk::image_layout::color_optimal,
            .resolve_mode = vk::resolved_mode_flags::none,
            .resolve_image_view = nullptr,
            .resolve_image_layout = vk::image_layout::undefined,
            .load = vk::attachment_load::clear,
            .store = vk::attachment_store::store,
            .clear_values = clear_color
        };

        vk::rendering_attachment depth_stencil_attachment = {
            .image_view = swapchain_depth_images[current_frame].image_view(),
            .layout = vk::image_layout::depth_stencil_optimal,
            .resolve_mode = vk::resolved_mode_flags::none,
            .resolve_image_view = nullptr,
            .resolve_image_layout = vk::image_layout::undefined,
            .load = vk::attachment_load::clear,
            .store = vk::attachment_store::store,
            .depth_values = depth_value
        };

        vk::rendering_begin_parameters begin_params = {
            .render_area = { { 0, 0 },
                             {
                               swapchain_extent.width,
                               swapchain_extent.height,
                             }, },
            .layer_count = 1,
            .color_attachments = std::span<const vk::rendering_attachment>(
              &color_render_attachment, 1),
            .depth_attachment = depth_stencil_attachment,
            .stencil_attachment = depth_stencil_attachment,
        };

        vk::viewport_params viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(swapchain_extent.width),
            .height = static_cast<float>(swapchain_extent.height),
            .min_depth = 0.0f,
            .max_depth = 1.0f,
        };
        current.set_viewport(
          0, 1, std::span<const vk::viewport_params>(&viewport, 1));

        vk::scissor_params scissor = {
            .offset = { 0, 0 },
            .extent = swapchain_extent,
        };

        current.set_scissor(
          0, 1, std::span<const vk::scissor_params>(&scissor, 1));

        current.begin_rendering(begin_params);

        main_graphics_pipeline.bind(current);
        vkCmdDraw(current, 3, 1, 0, 0);

        current.end_rendering();

        swapchain_images[current_frame].memory_barrier(
          current,
          surface_properties.format.format,
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

        current.end();

        // Submitting and then presenting to the screen
        std::array<const VkCommandBuffer, 1> commands = { current };
        presentation_queue.submit_async(commands);
        presentation_queue.present_frame(current_frame);
    }

    logical_device.wait();
    main_swapchain.destruct();

    geometry_resource.destruct();
    main_graphics_pipeline.destruct();

    for (auto& command : swapchain_command_buffers) {
        command.destruct();
    }

    for (auto& image : swapchain_images) {
        image.destruct();
    }

    for (auto& image : swapchain_depth_images) {
        image.destruct();
    }

    presentation_queue.destruct();

    logical_device.destruct();
    window_surface.destruct();
    glfwDestroyWindow(window);
    return 0;
}
