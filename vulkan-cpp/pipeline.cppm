module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>
#include <cstdint>
#include <array>

export module vk:pipeline;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {

        struct input_assembly_state {
            const enum primitive_topology topology = primitive_topology::triangle_list;
            bool primitive_restart_enable = false;
        };

        struct viewport_state {
            uint8_t viewport_count= 1;
            uint8_t scissor_count = 1;
        };

        struct rasterization_state {
            bool depth_clamp_enabled = false;
            bool rasterizer_discard_enabled = false;
            polygon_mode polygon_mode = polygon_mode::fill;
            enum cull_mode cull_mode = cull_mode::none;
            enum front_face front_face = front_face::counter_clockwise;
            bool depth_bias_enabled=false;
            float depth_bias_constant = 0.f;
            float depth_bias_clamp = 0.f;
            float depth_bias_slope = 0.f;
            float line_width = 1.f;
        };

        struct multisample_state {
            sample_bit rasterization_samples=sample_bit::count_1;
            bool shading_enabled=false;
            float min_shading = 1.f;                // optional
            std::span<uint32_t> p_sample_masks={};   // optional
            bool alpha_to_coverage_enable=false;    // optional
            bool alpha_to_one_enable=false;         // optional
        };

        struct color_blend_attachment_state {
            bool blend_enabled = true;
            blend_factor src_color_blend_factor=blend_factor::src_alpha;
            blend_factor dst_color_blend_factor = blend_factor::one_minus_src_alpha;
            blend_op color_blend_op = blend_op::add;
            blend_factor src_alpha_blend_factor = blend_factor::one;
            blend_factor dst_alpha_blend_factor = blend_factor::zero;
            blend_op alpha_blend_op = blend_op::add;
            uint32_t color_write_mask = color_component::red | color_component::green | color_component::blue | color_component::alpha;
        };

        struct color_blend_state {
            bool logic_op_enable=false;
            logical_op logical_op = logical_op::copy;
            std::span<const color_blend_attachment_state> attachments;
            std::span<float> blend_constants;
        };

        struct depth_stencil_state {
            bool depth_test_enable = true;
            bool depth_write_enable = true;
            compare_op depth_compare_op = compare_op::less;
            bool depth_bounds_test_enable = false;
            bool stencil_test_enable = false;
        };


        /**
         * @param renderpass is required for a VkPipeline to know up front
         * @param shader_modules is a std::span<VkShaderModule> of the loaded shader
         * sources for the pipeline to correspond to
         * @param descriptor_layouts are the VkDescriptorSetLayout that you pass up
         * front to the graphics pipeline if there are any provided
         */
        struct pipeline_params {
            VkRenderPass renderpass = nullptr;
            std::span<const shader_handle> shader_modules{};
            std::span<const VkVertexInputAttributeDescription> vertex_attributes;
            std::span<const VkVertexInputBindingDescription> vertex_bind_attributes;
            std::span<VkDescriptorSetLayout> descriptor_layouts;

            input_assembly_state input_assembly;
            viewport_state viewport;
            rasterization_state rasterization;
            multisample_state multisample;
            color_blend_state color_blend;
            bool depth_stencil_enabled = false;
            depth_stencil_state depth_stencil;
            std::span<dynamic_state> dynamic_states = {};
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
            pipeline(const VkDevice& p_device, const pipeline_params& p_info) : m_device(p_device) {
                create(p_info);
            }

            /**
             * @brief explicit API for creating a VkPipeline and VkPipelineLayout
             * handle
             * 
             * Code Usage Example
             * 
             * ```C++
             * 
             * vk::pipeline_params pipeline_params = {
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
            void create(const pipeline_params& p_info) {
                std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages(p_info.shader_modules.size());

                uint32_t shader_src_index = 0;

                // 1. Load in and setup the VKShaderModule handlers for VkPipeline
                for (const shader_handle& src : p_info.shader_modules) {
                    VkShaderStageFlags stage = static_cast<VkShaderStageFlags>(src.stage);
                    pipeline_shader_stages[shader_src_index] =
                    VkPipelineShaderStageCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage = (VkShaderStageFlagBits)stage,
                        .module = src.module,
                        .pName = "main"
                    };

                    shader_src_index++;
                }

                // 2. Setting up the vertex attribute details for VkPipeline
                std::span<const VkVertexInputBindingDescription> bind_attributes =
                p_info.vertex_bind_attributes;
                std::span<const VkVertexInputAttributeDescription> attributes =
                p_info.vertex_attributes;

                VkPipelineVertexInputStateCreateInfo vertex_input_info = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                    .vertexBindingDescriptionCount =
                    static_cast<uint32_t>(bind_attributes.size()),
                    .pVertexBindingDescriptions = bind_attributes.data(),
                    .vertexAttributeDescriptionCount =
                    static_cast<uint32_t>(attributes.size()),
                    .pVertexAttributeDescriptions = attributes.data()
                };

                VkPipelineInputAssemblyStateCreateInfo input_assembly = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology = static_cast<VkPrimitiveTopology>(p_info.input_assembly.topology),
                    .primitiveRestartEnable = p_info.input_assembly.primitive_restart_enable,
                };

                VkPipelineViewportStateCreateInfo viewport_state = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                    .viewportCount = p_info.viewport.viewport_count,
                    .scissorCount = p_info.viewport.scissor_count,
                };

                // if lineWidth is zero, validation layers will occur
                // because cant be zero. Must be set to 1.0f
                VkPipelineRasterizationStateCreateInfo rasterizer_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .depthClampEnable = p_info.rasterization.depth_clamp_enabled,
                    .rasterizerDiscardEnable = p_info.rasterization.rasterizer_discard_enabled,
                    .polygonMode = static_cast<VkPolygonMode>(p_info.rasterization.polygon_mode),
                    .cullMode = static_cast<VkCullModeFlags>(p_info.rasterization.cull_mode),
                    .frontFace = static_cast<VkFrontFace>(p_info.rasterization.front_face),
                    .depthBiasEnable = p_info.rasterization.depth_bias_enabled,
                    .depthBiasConstantFactor = p_info.rasterization.depth_bias_constant,
                    .depthBiasClamp = p_info.rasterization.depth_bias_clamp,
                    .depthBiasSlopeFactor = p_info.rasterization.depth_bias_slope,
                    .lineWidth = p_info.rasterization.line_width
                };

                VkPipelineMultisampleStateCreateInfo multisampling_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .rasterizationSamples = static_cast<VkSampleCountFlagBits>(p_info.multisample.rasterization_samples),
                    .sampleShadingEnable = p_info.multisample.shading_enabled,
                    .minSampleShading = p_info.multisample.min_shading,
                    .pSampleMask = p_info.multisample.p_sample_masks.data(),
                    .alphaToCoverageEnable = p_info.multisample.alpha_to_coverage_enable,
                    .alphaToOneEnable = p_info.multisample.alpha_to_one_enable,
                };

                std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments(p_info.color_blend.attachments.size());

                for(size_t i = 0; i < color_blend_attachments.size(); i++) {
                    color_blend_attachments[i] = {
                        .blendEnable = p_info.color_blend.attachments[i].blend_enabled,
                        .srcColorBlendFactor = static_cast<VkBlendFactor>(p_info.color_blend.attachments[i].src_color_blend_factor),     // Enabled: alpha blending
                        .dstColorBlendFactor = static_cast<VkBlendFactor>(p_info.color_blend.attachments[i].dst_color_blend_factor),     // Enabled: alpha blending
                        .colorBlendOp = static_cast<VkBlendOp>(p_info.color_blend.attachments[i].color_blend_op),                        // Enabled: alpha blending
                        .srcAlphaBlendFactor = static_cast<VkBlendFactor>(p_info.color_blend.attachments[i].src_alpha_blend_factor),     // Enabled: alpha blending
                        .dstAlphaBlendFactor = static_cast<VkBlendFactor>(p_info.color_blend.attachments[i].dst_alpha_blend_factor),     // Enabled: alpha blending
                        .alphaBlendOp = static_cast<VkBlendOp>(p_info.color_blend.attachments[i].alpha_blend_op),                        // Enabled: alpha blending
                        .colorWriteMask = static_cast<VkColorComponentFlags>(p_info.color_blend.attachments[i].color_write_mask),
                    };
                }


                VkPipelineColorBlendStateCreateInfo color_blending_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                    .logicOpEnable = p_info.color_blend.logic_op_enable,
                    .logicOp = static_cast<VkLogicOp>(p_info.color_blend.logical_op), // Optional
                    .attachmentCount = static_cast<uint32_t>(color_blend_attachments.size()),
                    .pAttachments = color_blend_attachments.data(),
                    // these are optional
                    .blendConstants =  {0.f, 0.f, 0.f, 0.f} // optional -- set to default in being 0.0f's
                };

                // Using ranges to load in the floats from an arbitrary array into this. Though it should only be valid to accept only 4 floats rather then N arbitrary floats in this buffer.
                if(!p_info.color_blend.blend_constants.empty()) {
                    // Get the first 4 elements in the span as those are
                    // the data we are to set the .blendConstants to.
                    // As .blendConstants only take up to 4 elements in the array.
                    std::span<float, 4> color_blend_constants = p_info.color_blend.blend_constants.first<4>();
                    std::ranges::copy(color_blend_constants.begin(), color_blend_constants.end(), color_blending_ci.blendConstants);
                }

                VkPipelineDepthStencilStateCreateInfo pipeline_deth_stencil_state_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                    .depthTestEnable = p_info.depth_stencil.depth_test_enable,
                    .depthWriteEnable = p_info.depth_stencil.depth_write_enable,
                    .depthCompareOp = static_cast<VkCompareOp>(p_info.depth_stencil.depth_compare_op),
                    .depthBoundsTestEnable = p_info.depth_stencil.depth_bounds_test_enable,
                    .stencilTestEnable = p_info.depth_stencil.stencil_test_enable,
                };

                //! @note -- pipeline states needs to be baked into the pipeline state
                VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
                    .dynamicStateCount = static_cast<uint32_t>(p_info.dynamic_states.size()),
                    .pDynamicStates = reinterpret_cast<const VkDynamicState*>(p_info.dynamic_states.data())
                };

                // Specifies layout of the uniforms (data resources) to be used by this specified graphics pipeline
                VkPipelineLayoutCreateInfo pipeline_layout_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .setLayoutCount = static_cast<uint32_t>(p_info.descriptor_layouts.size()),
                    .pSetLayouts = p_info.descriptor_layouts.data(),
                };

                vk_check(vkCreatePipelineLayout(
                        m_device, &pipeline_layout_ci, nullptr, &m_pipeline_layout),
                        "vkCreatePipelineLayout");

                VkGraphicsPipelineCreateInfo graphics_pipeline_ci = {
                    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stageCount = static_cast<uint32_t>(pipeline_shader_stages.size()),
                    .pStages = pipeline_shader_stages.data(),
                    .pVertexInputState = &vertex_input_info,
                    .pInputAssemblyState = &input_assembly,
                    .pViewportState = &viewport_state,
                    .pRasterizationState = &rasterizer_ci,
                    .pMultisampleState = &multisampling_ci,
                    .pDepthStencilState = &pipeline_deth_stencil_state_ci,
                    .pColorBlendState = &color_blending_ci,
                    .pDynamicState = (p_info.depth_stencil_enabled) ? &dynamic_state_ci : nullptr,
                    .layout = m_pipeline_layout,
                    .renderPass = p_info.renderpass,
                    .subpass = 0,
                    .basePipelineHandle = nullptr,
                    .basePipelineIndex = -1
                };

                vk::vk_check(
                vkCreateGraphicsPipelines(
                    m_device, nullptr, 1, &graphics_pipeline_ci, nullptr, &m_pipeline),
                "vkCreateGraphicsPipelines");
            }

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
             * vk::pipeline graphics_pipeline(logical_device, *assume pipeline_params is specified*);
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
            void bind(const VkCommandBuffer& p_command, pipeline_bind_point p_bind_point = pipeline_bind_point::graphics) {
                vkCmdBindPipeline(p_command, static_cast<VkPipelineBindPoint>(p_bind_point), m_pipeline);
            }

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
             * vk::pipeline graphics_pipeline(logical_device, *assume pipeline_params is specified*);
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
                            const void* p_data) {
                vkCmdPushConstants(p_current, m_pipeline_layout, static_cast<VkShaderStageFlags>(p_stage), p_offset, p_range, p_data);
            }

            //! @return true if m_pipeline is valid, false if invalid
            [[nodiscard]] bool alive() const { return m_pipeline; }

            //! @return VkPipelineLayout that has been created with the vk::pipeline
            //! handle
            [[nodiscard]] VkPipelineLayout layout() const {
                return m_pipeline_layout;
            }

            //! @brief explicit cleanup performed on vk::pipeline
            void destroy() {
                if (m_pipeline_layout != nullptr) {
                    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
                }
                if (m_pipeline != nullptr) {
                    vkDestroyPipeline(m_device, m_pipeline, nullptr);
                }
            }

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
};