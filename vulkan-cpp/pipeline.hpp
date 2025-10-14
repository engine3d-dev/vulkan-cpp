#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    /*
        Draft #2 -- Shader Modules, Vertex Attributes, Offload to VkPipeline
       Usage std::array<vk::shader_source, 2> shader_sources = {
            vk::shader_source{
                .filename = "some_path/test_shader.vert.spv",
                .stage = vk::shader_stage::vertex
            },
            vk::shader_source{
                .filename = "some_path/test_shader.frag.spv",
                .stage = vk::shader_stage::fragment
            },
            // Eventually when we do deal with compute shaders (todo in the
       future) vk::shader_source{ .filename = "some_path/test_shader.comp.spv",
                .stage = vk::shader_stage::compute
            },
        };

        vk::shader_resource_info resource_info {
            .sources = shader_sources,
            .vertex_attributes = my_test_vertex_attr

        };
        vk::shader_resource geo_res(logical_device, resource_info);

        vk::pipeline_info pipeline_resource_info = {
            .shader_modules = geo_res.handlers(), // return
       std::span<VkShaderModule> .vertex_attributes = geo_res.attributes() //
       return std::span<vertex_attribute>
        };
        vk::pipeline main_pipeline(pipeline_resource_info);


    */

    /**
     * @param renderpass is required for a VkPipeline to know up front
     * @param shader_modules is a std::span<VkShaderModule> of the loaded shader
     * sources for the pipeline to correspond to
     * @param descriptor_layouts are the VkDescriptorSetLayout that you pass up
     * front to the graphics pipeline if there are any provided
     */
    struct pipeline_settings {
        VkRenderPass renderpass = nullptr;
        std::span<const shader_handle> shader_modules{};
        std::span<const VkVertexInputAttributeDescription> vertex_attributes;
        std::span<const VkVertexInputBindingDescription> vertex_bind_attributes;
        std::span<VkDescriptorSetLayout> descriptor_layouts;
    };

    /**
     * @brief pipeline represents a vulkan graphics pipeline implementation
     */
    class pipeline {
    public:
        pipeline() = default;
        pipeline(const VkDevice& p_device, const pipeline_settings& p_info);

        void create(const pipeline_settings& p_info);

        void bind(const VkCommandBuffer& p_command);

        [[nodiscard]] bool alive() const { return m_pipeline; }

        //! @return the VkPipelineLayout of the graphics pipeline
        [[nodiscard]] VkPipelineLayout layout() const {
            return m_pipeline_layout;
        }

        void destroy();

        operator VkPipeline() const { return m_pipeline; }

        operator VkPipeline() { return m_pipeline; }

    private:
        VkDevice m_device = nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline = nullptr;
    };
};