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

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

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
        //! @brief Creating vertex/index buffers with host visibility flags
        const auto property_flags = static_cast<vk::memory_property>(
          vk::memory_property::host_visible_bit |
          vk::memory_property::host_cached_bit);

        vk::buffer_parameters vertex_params = {
            .memory_mask = p_physical.memory_properties(
              vk::memory_property::device_local_bit |
              vk::memory_property::host_visible_bit),
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

/**
 * @brief STBI-specific implementation of the vk::image interface
 */
class stb_image : public vk::image {
public:
    stb_image() = delete;

    stb_image(std::string_view p_path, vk::texture_params p_params) {
        image_load(p_path, p_params);
    }

    ~stb_image() = default;

protected:
    bool image_load(std::string_view p_path,
                    vk::texture_params p_params) override {
        int w = 0;
        int h = 0;
        int channels = 0;

        stbi_uc* image_pixel_data =
          stbi_load(p_path.data(), &w, &h, &channels, STBI_rgb_alpha);

        if (!image_pixel_data) {
            return false;
        }

        const VkFormat texture_format =
          static_cast<VkFormat>(vk::format::r8g8b8a8_unorm);
        int bytes_per_pixel = vk::bytes_per_texture_format(texture_format);

        m_extent = {
            .width = static_cast<uint32_t>(w),
            .height = static_cast<uint32_t>(h),
        };

        // Retrieving total size of bytes of the dimensions of the image and
        // accounting for pixels of the image
        uint32_t size_bytes =
          m_extent.width * m_extent.height * bytes_per_pixel;

        // Retrieving total image size to the count of the image layers
        uint32_t size = size_bytes * p_params.layer_count;

        vk::image_params image_options = {
            .extent = m_extent,
            .format = texture_format,
            .memory_mask = p_params.memory_mask,
            .usage =
              vk::image_usage::transfer_dst_bit | vk::image_usage::sampled_bit,
            .mip_levels = p_params.mip_levels,
            .layer_count = p_params.layer_count,
        };

        m_bytes.reserve(size);
        std::span<uint8_t> bytes_view =
          std::span<uint8_t>(image_pixel_data, size);

        m_bytes.assign(bytes_view.begin(), bytes_view.end());

        stbi_image_free(image_pixel_data);

        return true;
    }

    std::span<const uint8_t> image_read() const override { return m_bytes; }

    vk::image_extent image_extent() const override { return m_extent; }

private:
    vk::image_extent m_extent{};
    std::vector<uint8_t> m_bytes{};
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
    std::array<const char*, 2> extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        "VK_KHR_portability_subset",
    };
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
        .present_index = 0,
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
                        .height = swapchain_extent.height, },
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

        // Creating Images for depth buffering
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

    // setting up presentation queue to display commands to the screen
    vk::queue_params enumerate_present_queue{
        .family = 0,
        .index = 0,
    };
    vk::device_present_queue presentation_queue(
      logical_device, main_swapchain, enumerate_present_queue);

    // gets set with the renderpass
    std::array<float, 4> color = { 0.f, 0.5f, 0.5f, 1.f };

    // std::println("Start implementing graphics pipeline!!!");

    // Now creating a vulkan graphics pipeline for the shader loading
    std::array<vk::shader_source, 2> shader_sources = {
        vk::shader_source{
          .filename = "shader_samples/sample5/test.vert.spv",
          .stage = vk::shader_stage::vertex,
        },
        vk::shader_source{
          .filename = "shader_samples/sample5/test.frag.spv",
          .stage = vk::shader_stage::fragment,
        },
    };

    // Setting up vertex attributes in the test shaders
    std::array<vk::vertex_attribute_entry, 4> attribute_entries = {
        vk::vertex_attribute_entry{
          .location = 0,
          .format = vk::format::rgb32_sfloat,
          .stride = offsetof(vk::vertex_input, position),
        },
        vk::vertex_attribute_entry{
          .location = 1,
          .format = vk::format::rgb32_sfloat,
          .stride = offsetof(vk::vertex_input, color),
        },
        vk::vertex_attribute_entry{
          .location = 2,
          .format = vk::format::rg32_sfloat,
          .stride = offsetof(vk::vertex_input, uv),
        },
        vk::vertex_attribute_entry{
          .location = 3,
          .format = vk::format::rgb32_sfloat,
          .stride = offsetof(vk::vertex_input, normals),
        }
    };

    std::array<vk::vertex_attribute, 1> attributes = {
        vk::vertex_attribute{
          // layout (set = 0, binding = 0)
          .binding = 0,
          .entries = attribute_entries,
          .stride = sizeof(vk::vertex_input),
          .input_rate = vk::input_rate::vertex,
        },
    };

    // To render triangle, we do not need to set any vertex attributes
    vk::shader_resource_info shader_info = {
        .sources = shader_sources,
        .vertex_attributes =
          attributes // NOT NEEDED: Specifying vertex attributes
    };
    vk::shader_resource geometry_resource(logical_device, shader_info);
    geometry_resource.vertex_attributes(attributes);

    // Set 0: For Uniform BUffers (or global scene data)
    std::vector<vk::descriptor_entry> entries = {
    vk::descriptor_entry{
            // specifies "layout (set = 0, binding = 0) uniform GlobalUbo"
            .type = vk::descriptor_type::uniform,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::vertex,
            },
            .descriptor_count = 1,
        },
    };
    vk::descriptor_layout set0_layout = {
        .slot = 0,               // indicate specific descriptor slot 0
        .max_sets = image_count, // max descriptors to allocate
        .entries = entries,      // descriptor layout entries description
    };
    vk::descriptor_resource set0_resource(logical_device, set0_layout);

    // Set 1 = For Textures
    std::vector<vk::descriptor_entry> entries_set1 = {
        vk::descriptor_entry{
            // layout (set = 1, binding = 0) uniform sampler2D
            .type = vk::descriptor_type::combined_image_sampler,
            .binding_point = {
                .binding = 0,
                .stage = vk::shader_stage::fragment,
            },
            .descriptor_count = 1,
        }
    };
    vk::descriptor_layout set1_layout = {
        .slot = 1,               // indicate specific descriptor slot 0
        .max_sets = image_count, // max descriptors to allocate
        .entries = entries_set1, // descriptor layout entries description
    };

    vk::descriptor_resource set1_resource(logical_device, set1_layout);

    std::array<VkDescriptorSetLayout, 2> layouts = {
        set0_resource.layout(),
        set1_resource.layout(),
    };

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
        .descriptor_layouts = layouts,
        .color_blend = {
            .attachments = color_blend_attachments,
        },
        .depth_stencil_enabled = true,
        .dynamic_states = dynamic_states,
    };
    vk::pipeline main_graphics_pipeline(logical_device, pipeline_configuration);

    // Loading mesh
    obj_model test_model(std::filesystem::path("asset_samples/viking_room.obj"),
                         logical_device,
                         physical_device);

    // Setting up descriptor sets for handling uniforms
    vk::buffer_parameters uniform_params = {
        .memory_mask =
          physical_device.memory_properties(static_cast<vk::memory_property>(
            vk::memory_property::host_visible_bit |
            vk::memory_property::host_cached_bit)),
        .usage = vk::buffer_usage::uniform_buffer_bit |
                 vk::buffer_usage::shader_device_address_bit,
        .allocate_flags = vk::memory_allocate_flags::device_address_bit_khr,
    };
    vk::uniform_buffer test_ubo = vk::uniform_buffer(
      logical_device, sizeof(global_uniform), uniform_params);
    // std::println("uniform_buffer.alive() = {}", test_ubo.alive());

    std::array<vk::write_buffer, 1> uniforms0 = {
        vk::write_buffer{
          .buffer = test_ubo,
          .offset = 0,
          .range = static_cast<uint32_t>(test_ubo.size_bytes()),
        },
    };
    std::array<vk::write_buffer_descriptor, 1> uniforms = {
        vk::write_buffer_descriptor{
          .dst_binding = 0,
          .uniforms = uniforms0,
        },
    };

    // Loading a texture
    vk::texture_params config_texture = {
        .memory_mask = physical_device.memory_properties(
          vk::memory_property::host_visible_bit |
          vk::memory_property::host_cached_bit),
    };

    stb_image img = stb_image("asset_samples/viking_room.png", config_texture);
    vk::texture texture1(logical_device, &img, config_texture);

    std::array<vk::write_image, 1> samplers = {
        vk::write_image{
          .sampler = texture1.image().sampler(),
          .view = texture1.image().image_view(),
          .layout = vk::image_layout::shader_read_only_optimal,
        },
    };

    // Specify image descriptor images/samplers to the descriptor
    std::array<vk::write_image_descriptor, 1> set1_samples = {
        vk::write_image_descriptor{
          .dst_binding = 0,
          .sample_images = samplers,
        }
    };
    set0_resource.update(uniforms);

    set1_resource.update({}, set1_samples);

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

        const VkBuffer vertex = test_model.vertex_handle();
        uint64_t offset = 0;
        current.bind_vertex_buffers(std::span<const VkBuffer>(&vertex, 1),
                                    std::span<uint64_t>(&offset, 1));

        if (test_model.has_indices()) {
            current.bind_index_buffers32(test_model.index_handle());
        }

        static auto start_time = std::chrono::high_resolution_clock::now();

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(
                       current_time - start_time)
                       .count();

        // We set the uniforms and then we offload that to the GPU
        global_uniform ubo = {
            .model = glm::rotate(glm::mat4(1.0f),
                                 time * glm::radians(90.0f),
                                 glm::vec3(0.0f, 0.0f, 1.0f)),
            .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                                glm::vec3(0.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 1.0f)),
            .proj = glm::perspective(glm::radians(45.0f),
                                     (float)swapchain_extent.width /
                                       (float)swapchain_extent.height,
                                     0.1f,
                                     10.0f)
        };
        ubo.proj[1][1] *= -1;

        std::array<global_uniform, 1> ubo_arr = { ubo };
        test_ubo.transfer<global_uniform>(ubo_arr);

        // Before we can send stuff to the GPU, since we already updated the
        // descriptor set 0 beforehand, we must bind that descriptor resource
        // before making any of the draw calls Something to note: You cannot
        // update descriptor sets in the process of a current-recording command
        // buffers or else that becomes undefined behavior
        // set0_resource.bind(current, main_graphics_pipeline.layout());

        std::array<const VkDescriptorSet, 2> descriptors = { set0_resource,
                                                             set1_resource };

        current.bind_descriptors(main_graphics_pipeline.layout(),
                                 VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 descriptors);

        // Drawing-call to render actual triangle to the screen
        // vkCmdDrawIndexed(current, static_cast<uint32_t>(indices.size()), 1,
        // 0, 0, 0);
        test_model.draw(current);

        main_renderpass.end(current);
        current.end();

        // Submitting and then presenting to the screen
        std::array<const VkCommandBuffer, 1> commands = { current };
        // presentation_queue.submit_async(current);
        presentation_queue.submit_async(commands);
        presentation_queue.present_frame(current_frame);
    }

    // this to ensure they are cleaned up in the proper order
    logical_device.wait();
    main_swapchain.destruct();

    texture1.destruct();
    set0_resource.destruct();
    set1_resource.destruct();
    test_ubo.destruct();
    test_model.destruct();

    for (auto& command : swapchain_command_buffers) {
        command.destruct();
    }

    for (auto& fb : swapchain_framebuffers) {
        fb.destruct();
    }

    for (auto& image : swapchain_images) {
        image.destruct();
    }

    for (auto& depth_img : swapchain_depth_images) {
        depth_img.destruct();
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