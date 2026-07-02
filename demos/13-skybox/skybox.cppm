module;

#include <vulkan/vulkan.h>
#include <span>
#include <string>
#include <print>
#include <optional>

#include <stb_image.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

export module skybox;

import vk;

export struct skybox_uniform {
    glm::mat4 proj_view;
};

// Converts raw stbi image stbi_uc* raw pixels pointers to a span<uint8_t>
std::span<uint8_t>
to_bytes(stbi_uc* p_pixels, uint32_t p_image_size) {
    return std::span<uint8_t>(reinterpret_cast<uint8_t*>(p_pixels),
                              p_image_size);
}

export class skybox_environment {
public:
    skybox_environment(const VkDevice& p_device,
                       const vk::physical_device& p_physical,
                       std::span<const std::string> p_faces,
                       const VkRenderPass& p_renderpass)
      : m_device(p_device) {
        m_physical = p_physical;
        m_renderpass = p_renderpass;

        if (p_faces.size() != 6) {
            std::println("Cubemap requires 6 faces, received {} count of faces",
                         p_faces.size());
            return;
        }

        // Loading in all 6-faces images for the skybox
        int w = 0;
        int h = 0;
        int channels = 0;
        std::array<std::span<uint8_t>, 6> faces{};

        auto* face0 =
          stbi_load(p_faces[0].c_str(), &w, &h, &channels, STBI_rgb_alpha);
        int face_width = w;
        int face_height = h;

        VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
        const uint32_t bytes_per_pixel =
          static_cast<uint32_t>(vk::bytes_per_texture_format(image_format));
        auto size_bytes = face_width * face_height * bytes_per_pixel;

        faces[0] = to_bytes(face0, size_bytes);

        for (size_t i = 1; i < faces.size(); i++) {
            auto* face_pixels =
              stbi_load(p_faces[i].c_str(), &w, &h, &channels, STBI_rgb_alpha);
            faces[i] = to_bytes(face_pixels, size_bytes);

            if (faces[i].empty()) {
                std::println("Could not load face: {}", p_faces[i]);
                return;
            }

            if (w != face_width || h != face_height) {
                std::println("Cubemap faces must match dimensions. Face 0 is "
                             "{}x{}, face {} is {}x{} ({})",
                             face_width,
                             face_height,
                             i,
                             w,
                             h,
                             p_faces[i]);
                return;
            }
        }

        const uint32_t width = static_cast<uint32_t>(face_width);
        const uint32_t height = static_cast<uint32_t>(face_height);
        const VkDeviceSize face_size_bytes =
          static_cast<VkDeviceSize>(width) * static_cast<VkDeviceSize>(height) *
          static_cast<VkDeviceSize>(bytes_per_pixel);
        const VkDeviceSize total_size_bytes = face_size_bytes * 6;

        vk::image_params skybox_params = {
            .extent = { .width = width, .height = height, .depth = 1 },
            .format = image_format,
            .memory_mask = p_physical.memory_properties(
              vk::memory_property::device_local_bit),
            .image_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
            .view_type = VK_IMAGE_VIEW_TYPE_CUBE,
            .layer_count = 6,
            .array_layers = 6,
            .usage =
              vk::image_usage::transfer_dst_bit | vk::image_usage::sampled_bit,
        };
        m_skybox_image = vk::sample_image(m_device, skybox_params);

        // perform staging buffer
        vk::buffer_parameters staging_params = {
            .memory_mask = p_physical.memory_properties(
              vk::memory_property::host_visible_bit |
              vk::memory_property::host_coherent_bit),
            .usage = vk::buffer_usage::transfer_src_bit,
        };

        vk::buffer staging(m_device, total_size_bytes, staging_params);

        staging.transfer(faces);

        for (size_t i = 0; i < faces.size(); i++) {
            stbi_image_free(faces[i].data());
        }

        vk::command_params upload_params = {
            .levels = vk::command_levels::primary,
            .queue_index = 0, // graphics queue family index
            .flags = vk::command_pool_flags::reset,
        };
        vk::command_buffer upload_cmd(m_device, upload_params);
        upload_cmd.begin(vk::command_usage::one_time_submit);

        // NOTE: explicitly set VK_IMAGE_ASPECT_COLOR_BIT to ensure the wrong
        // aspect flags are not being set when using
        // vk::sample_image::memory_barrier
        m_skybox_image.memory_barrier(upload_cmd,
                                      image_format,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      6);

        std::array<vk::buffer_image_copy, 6> regions;

        for (uint32_t face = 0; face < regions.size(); face++) {
            // Copy the specific face region to the image
            regions[face] = {
                .offset = static_cast<uint32_t>(face_size_bytes * face),
                .base_array_layer = face,
                .image_offset = { .width = 0, .height = 0, .depth = 0 },
                .image_extent = { .width = width, .height = height },
            };
        }

        staging.copy_to_image(upload_cmd, m_skybox_image, regions);

        m_skybox_image.memory_barrier(upload_cmd,
                                      image_format,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_IMAGE_ASPECT_COLOR_BIT,
                                      6);
        upload_cmd.end();

        VkQueue graphics_queue = nullptr;
        vkGetDeviceQueue(m_device, 0, 0, &graphics_queue);
        const VkCommandBuffer cmd = upload_cmd;
        VkSubmitInfo submit = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };
        vk::vk_check(vkQueueSubmit(graphics_queue, 1, &submit, nullptr),
                     "vkQueueSubmit(cubemap upload)");
        vk::vk_check(vkQueueWaitIdle(graphics_queue),
                     "vkQueueWaitIdle(cubemap upload)");

        upload_cmd.destruct();
        staging.destruct();

        create_skybox_pipeline();
    }

    void create_skybox_pipeline() {
        create_buffers();

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
              .format = vk::format::rgb32_sfloat,
              .stride = offsetof(vk::vertex_input, normals),
            },
            vk::vertex_attribute_entry{
              .location = 3,
              .format = vk::format::rg32_sfloat,
              .stride = offsetof(vk::vertex_input, uv),
            }
        };
        std::array<vk::vertex_attribute, 1> attribute = {
            vk::vertex_attribute{
              // layout (set = 0, binding = 0)
              .binding = 0,
              .entries = attribute_entries,
              .stride = sizeof(vk::vertex_input),
              .input_rate = vk::input_rate::vertex,
            },
        };

        const std::array<vk::shader_source, 2> sources = {
            vk::shader_source{
              .filename = "shader_samples/sample7-skybox/skybox.vert.spv",
              .stage = vk::shader_stage::vertex,
            },
            vk::shader_source{
              .filename = "shader_samples/sample7-skybox/skybox.frag.spv",
              .stage = vk::shader_stage::fragment,
            },
        };

        vk::shader_resource_info shader_info = {
            .sources = sources,
        };
        m_skybox_shaders = vk::shader_resource(m_device, shader_info);
        m_skybox_shaders.vertex_attributes(attribute);

        // set=0 binding=0 UBO: mat4 VP
        // vk::uniform_params ubo_params = {
        //     .phsyical_memory_properties = p_memory_properties,
        //     .debug_name = "skybox_ubo",
        //     .vkSetDebugUtilsObjectNameEXT = nullptr,
        // };
        vk::buffer_parameters uniform_params = {
            .memory_mask = m_physical.value().memory_properties(
              vk::memory_property::host_visible_bit |
              vk::memory_property::host_cached_bit),
            .usage = vk::buffer_usage::uniform_buffer_bit,
        };
        m_skybox_ubo =
          vk::uniform_buffer(m_device, sizeof(skybox_uniform), uniform_params);

        skybox_uniform identity = { .proj_view = glm::mat4(1.0f) };
        identity.proj_view[1][1] *= -1;

        m_skybox_ubo.transfer<skybox_uniform>(
          std::span<const skybox_uniform>(&identity, 1));

        // set=0 bindings:
        //  - binding 0: UBO (vertex)
        //  - binding 1: samplerCube (fragment)
        std::array<vk::descriptor_entry, 2> entries = {
            vk::descriptor_entry{
              .type = vk::descriptor_type::uniform,
              .binding_point =
                vk::descriptor_binding_point{
                  .binding = 0, .stage = vk::shader_stage::vertex },
              .descriptor_count = 1,
            },
            vk::descriptor_entry{
              .type = vk::descriptor_type::combined_image_sampler,
              .binding_point =
                vk::descriptor_binding_point{
                  .binding = 1, .stage = vk::shader_stage::fragment },
              .descriptor_count = 1,
            },
        };

        vk::descriptor_layout desc_layout = {
            .slot = 0,
            .max_sets = 1,
            .entries = entries,
        };
        m_skybox_descriptors = vk::descriptor_resource(m_device, desc_layout);

        const std::array<vk::write_buffer, 1> ubo_writes = {
            vk::write_buffer{ .buffer = m_skybox_ubo,
                              .offset = 0,
                              .range =
                                static_cast<uint32_t>(sizeof(skybox_uniform)) },
        };
        const vk::write_buffer_descriptor ubo_write_desc = {
            .dst_binding = 0,
            .uniforms = ubo_writes,
        };

        const std::array<vk::write_image, 1> image_writes = {
            vk::write_image{
              .sampler = m_skybox_image.sampler(),
              .view = m_skybox_image.image_view(),
              .layout = vk::image_layout::shader_read_only_optimal,
            },
        };
        const vk::write_image_descriptor image_write_desc = {
            .dst_binding = 1,
            .sample_images = image_writes,
        };

        m_skybox_descriptors.update(std::span(&ubo_write_desc, 1),
                                    std::span(&image_write_desc, 1));

        const std::array<VkDescriptorSetLayout, 1> layouts = {
            m_skybox_descriptors.layout(),
        };

        const std::array<vk::color_blend_attachment_state, 1>
          blend_attachments = {
              vk::color_blend_attachment_state{ .blend_enabled = false },
          };
        vk::color_blend_state blend_state = {
            .logic_op_enable = false,
            .logical_op = vk::logical_op::copy,
            .attachments = blend_attachments,
            .blend_constants = {},
        };

        std::array<vk::dynamic_state, 2> dyn = {
            vk::dynamic_state::viewport,
            vk::dynamic_state::scissor,
        };

        // pipeline expects a non-const span<VkDescriptorSetLayout>
        std::array<VkDescriptorSetLayout, 1> pipeline_layouts = layouts;

        vk::pipeline_params pipe_info = {
            .renderpass = m_renderpass,
            .shader_modules = m_skybox_shaders.handles(),
            .vertex_attributes =
              m_skybox_shaders.vertex_attributes(), // no vertex input
            .vertex_bind_attributes =
              m_skybox_shaders.vertex_bind_attributes(), // no vertex input
            .descriptor_layouts = pipeline_layouts,
            .input_assembly =
              vk::input_assembly_state{
                .topology = vk::primitive_topology::triangle_list,
                .primitive_restart_enable = false,
              },
            .viewport =
              vk::viewport_state{ .viewport_count = 1, .scissor_count = 1 },
            .rasterization =
              vk::rasterization_state{
                .polygon_mode = vk::polygon_mode::fill,
                .cull_mode = vk::cull_mode::front_bit,
                // .cull_mode = vk::cull_mode::none,
                // .front_face = vk::front_face::counter_clockwise,
                .front_face = vk::front_face::clockwise,
                .line_width = 1.f,
              },
            .multisample = vk::multisample_state{},
            .color_blend = blend_state,
            .depth_stencil_enabled = true,
            .depth_stencil =
              vk::depth_stencil_state{
                .depth_test_enable = true,
                .depth_write_enable = false,
                .depth_compare_op = vk::compare_op::less_or_equal,
                .depth_bounds_test_enable = false,
                .stencil_test_enable = false,
              },
            .dynamic_states = dyn,
        };

        m_skybox_pipeline = vk::pipeline(m_device, pipe_info);
    }

    void create_buffers() {

        std::vector<vk::vertex_input> vertices = {
            // Front Face
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },

            // Left Face
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },

            // Right Face
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },

            // Back Face
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },

            // Top Face
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, 1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },

            // Bottom Face
            vk::vertex_input{
              { -1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, -1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { -1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
            vk::vertex_input{
              { 1.0f, -1.0f, 1.0f },
              { 1.0f, 1.0f, 1.0f },
              { 0.0f, 0.0f, 0.0f },
              { 0.0f, 0.0f },
            },
        };

        m_skybox_vbo_size = vertices.size();

        vk::buffer_parameters vertex_params = {
            .memory_mask = m_physical.value().memory_properties(
              vk::memory_property::device_local_bit |
              vk::memory_property::host_visible_bit),
            .usage = vk::buffer_usage::transfer_dst_bit |
                     vk::buffer_usage::vertex_buffer_bit,
        };

        m_skybox_vbo = vk::vertex_buffer(m_device, vertices, vertex_params);
    }

    void update_uniform(const skybox_uniform& p_uniform) {
        m_skybox_ubo.transfer<skybox_uniform>(
          std::span<const skybox_uniform>(&p_uniform, 1));
    }

    void bind(vk::command_buffer p_command) {
        m_skybox_pipeline.bind(p_command);
        std::array<VkDescriptorSet, 1> descriptors = { m_skybox_descriptors };
        p_command.bind_descriptors(m_skybox_pipeline.layout(),
                                   VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   descriptors);

        std::array<const VkBuffer, 1> skybox_buffers = { m_skybox_vbo };
        uint64_t offset = 0;
        p_command.bind_vertex_buffers(skybox_buffers,
                                      std::span<const uint64_t>(&offset, 1));
    }

    void draw(const vk::command_buffer& p_command) {
        vkCmdDraw(p_command, m_skybox_vbo_size, 1, 0, 0);
    }

    void destruct() {
        m_skybox_ubo.destruct();
        m_skybox_image.destruct();
        m_skybox_shaders.destruct();
        m_skybox_pipeline.destruct();

        m_skybox_vbo.destruct();
        m_skybox_descriptors.destruct();
    }

private:
    vk::uniform_buffer m_skybox_ubo;
    VkDevice m_device = nullptr;
    std::optional<vk::physical_device> m_physical;
    vk::sample_image m_skybox_image;
    vk::shader_resource m_skybox_shaders;
    vk::pipeline m_skybox_pipeline;
    vk::vertex_buffer m_skybox_vbo;
    vk::descriptor_resource m_skybox_descriptors;
    VkRenderPass m_renderpass;
    uint32_t m_skybox_vbo_size=0;
};