#pragma once
#include <filesystem>
#include <vulkan-cpp/uniform_buffer.hpp>
#include <vulkan-cpp/texture.hpp>
#include <vulkan-cpp/pipeline.hpp>
#include <vulkan-cpp/descriptor_resource.hpp>
#include <vulkan-cpp/shader_resource.hpp>
#include <vulkan-cpp/renderpass.hpp>

struct cubemap_ubo {
    glm::vec4 forwards;
    glm::vec4 right;
    glm::vec4 up;
};

class skybox {
public:
    skybox() = default;
    skybox(const VkDevice& p_device, VkPhysicalDeviceMemoryProperties p_memory_properties, std::filesystem::path& p_path) : m_device(p_device) {
        vk::texture_info params = {
            .phsyical_memory_properties = p_memory_properties,
            .filepath = p_path,
        };
        m_cubemap_texture = vk::texture(p_device, params, true);

        vk::uniform_params uniforms_params = {
            .size_bytes = sizeof(cubemap_ubo),
        };
        m_cubemap_uniform = vk::uniform_buffer(p_device, uniforms_params);
    }


    void load_descriptors() {

        std::vector<vk::descriptor_entry> set0_entries = {
        vk::descriptor_entry{
                // specifies "layout (set = 0, binding = 0) uniform global_uniform"
                .type = vk::buffer::uniform,
                .binding_point = {
                    .binding = 0,
                    .stage = vk::shader_stage::vertex,
                },
                .descriptor_count = 1,
            }
        };

        vk::descriptor_layout set0_layout = {
            .slot = 0,
            .max_sets = 1,
            .entries = set0_entries,
        };

        m_skybox_descriptor = vk::descriptor_resource(m_device, set0_layout);
    }


    void load_shaders() {
        std::array<vk::shader_source, 2> sources = {
            vk::shader_source{
                .filename = "shader_samples/sandbox-shader-samples/skybox.vert.spv",
                .stage = vk::shader_stage::vertex,
            },
            vk::shader_source{
                .filename = "shader_samples/sandbox-shader-samples/skybox.frag.spv",
                .stage = vk::shader_stage::fragment,
            }
        };

        std::array<vk::vertex_attribute_entry, 4> attribute_entries = {
            // layout(location = 0) in vec3 inPosition;
            vk::vertex_attribute_entry{
                .location = 0,
                .format = vk::format::rgb32_sfloat,
                .stride = offsetof(vk::vertex_input, position)
            },
            // layout(location = 1) in vec3 inColor;
            vk::vertex_attribute_entry{
                .location = 1,
                .format = vk::format::rgb32_sfloat,
                .stride = offsetof(vk::vertex_input, color)
            },
            // layout(location = 2) in vec2 inTexCoords;
            vk::vertex_attribute_entry{
                .location = 2,
                .format = vk::format::rg32_sfloat,
                .stride = offsetof(vk::vertex_input, uv)
            },
            // layout(location = 3) in vec3 inNormals;
            vk::vertex_attribute_entry{
                .location = 3,
                .format = vk::format::rgb32_sfloat,
                .stride = offsetof(vk::vertex_input, normals)
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

        vk::shader_resource_info info = {
            .sources = sources
        };
        m_skybox_shaders = vk::shader_resource(m_device, info);
        m_skybox_shaders.vertex_attributes(attributes);
    }

    void load_pipeline() {
        std::array<VkDescriptorSetLayout, 1> layouts = {m_skybox_descriptor.layout()};

        vk::pipeline_settings pipeline_params = {
            .renderpass = m_skybox_renderpass,
            .shader_modules = m_skybox_shaders.handles(),
            .vertex_attributes = m_skybox_shaders.vertex_attributes(),
            .vertex_bind_attributes = m_skybox_shaders.vertex_bind_attributes(),
            .descriptor_layouts = layouts
        };
        m_skybox_pipeline = vk::pipeline(m_device, pipeline_params);
    }

private:
    VkDevice m_device=nullptr;
    vk::texture m_cubemap_texture;
    vk::uniform_buffer m_cubemap_uniform;
    vk::pipeline m_skybox_pipeline;
    vk::descriptor_resource m_skybox_descriptor;

    vk::shader_resource m_skybox_shaders;

    vk::renderpass m_skybox_renderpass;
};