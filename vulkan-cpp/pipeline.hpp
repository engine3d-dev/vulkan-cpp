#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

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

        /**
         * @brief constructs the graphics pipeline handle
         * 
         * @param p_device is logical device to create the graphics pipeline handles
         * @param p_info are the parameters for creating the pipelines with
        */
        pipeline(const VkDevice& p_device, const pipeline_settings& p_info);

        /**
         * @brief explicit API for creating a VkPipeline and VkPipelineLayout
         * handle
         * 
         * Code Usage Example
         * 
         * ```C++
         * 
         * vk::pipeline_settings pipeline_params = {
         *      .renderpass = main_renderpass // pass in VkRenderPass handle
         *      .shader_modules = shader_resource.handles() // sets the std::span<const shader_module>
         *      .vertex_attributes = shader_resource.vertex_attributes(),
         *      .vertex_bind_attributes = shader_resource.vertex_bind_attributes(),
         *      .descriptor_layouts = layouts
         * };
         * 
         * vk::pipeline graphics_pipeline(logical_device, pipeline_params);
         * 
         * // or when need to invalidate, explicitly call create
         * graphics_pipeline.create(pipeline_params);
         * 
         * ```
         *
         * @param p_info is the parameters required to set the graphics pipeline handles
         * 
         * More info on vulkan's official 
         * [docs](https://docs.vulkan.org/refpages/latest/refpages/source/vkCreateGraphicsPipelines.html)
         */
        void create(const pipeline_settings& p_info);

        /**
         *
         * @brief Once a pipeline is created can be bounded to the command
         * buffer
         *
         *
         * Code Example Usage
         *
         * ```C++
         *
         * vk::pipeline graphics_pipeline(logical_device, *assume pipeline_settings is specified*);
         *
         * // bound to current command buffer
         * // in this example we set binding point to VK_PIPELINE_BIND_POINT_GRAPHICS
         * graphics_pipeline.bind(current_command,
         * pipeline_bind_point::graphics);
         * ```
         *
         * @param p_command is the current command buffer the graphics pipeline
         * is bound to
         * @param p_bind_point is the specified bind point graphics pipeline is
         * bound to.
         * 
         * More info on vulkan's official 
         * [docs](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdBindPipeline.html)
         */
        void bind(
          const VkCommandBuffer& p_command,
          pipeline_bind_point p_bind_point = pipeline_bind_point::graphics);

        /**
         * @brief Update values of push constants
         * 
         * Push constants let us send small amount of data with a small
         * limited size to the shader
         *
         * Push constants only accept up to a max of 128 bytes of push constant
         * data
         *
         * They allow for sending data to any specified shader stage (both
         * vertex and fragment) that are stored within the command buffer itself
         *
         * Using push constants are commonly used when you have chunks of data
         * that may be changed every frame.
         *
         * Data that may need to be updated every frame such as camera
         * projection/view and model matrices.
         *
         * Example Code Usage:
         *
         * ```C++
         * 
         * vk::pipeline graphics_pipeline(logical_device, *assume pipeline_settings is specified*);
         * 
         * m_pipeline.push_constants(current, shader_stage::vertex, 0, 1,
         * &global_data);
         * 
         * ```
         *
         * @param p_current current command to push constants directly to the
         * shader
         * @param p_stage is specifying what stage of the push constants are at
         * @param p_offset is specified of the beginning of the offset to start
         * from
         * @param p_range is the range of bytes of the specified push constant
         * data
         * @param p_data is the data that is represented into bytes to push
         * constants
         *
         * More info on vulkan's official 
         * [docs](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdPushConstants.html)
         */
        void push_constant(const VkCommandBuffer& p_current,
                           shader_stage p_stage,
                           uint32_t p_offset,
                           uint32_t p_range,
                           const void* p_data);

        //! @return true if m_pipeline is valid, false if invalid
        [[nodiscard]] bool alive() const { return m_pipeline; }

        //! @return VkPipelineLayout that has been created with the vk::pipeline
        //! handle
        [[nodiscard]] VkPipelineLayout layout() const {
            return m_pipeline_layout;
        }

        //! @brief explicit cleanup performed on vk::pipeline
        void destroy();

        //! @brief allows for treating vk::pipeline as a VkPipeline handle for
        //! simple use
        operator VkPipeline() const { return m_pipeline; }

        operator VkPipeline() { return m_pipeline; }

    private:
        VkDevice m_device = nullptr;
        VkPipelineLayout m_pipeline_layout;
        VkPipeline m_pipeline = nullptr;
    };
};