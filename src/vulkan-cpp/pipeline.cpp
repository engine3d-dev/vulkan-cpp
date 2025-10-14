#include <vulkan-cpp/pipeline.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <vector>

namespace vk {
    pipeline::pipeline(const VkDevice& p_device,
                       const pipeline_settings& p_info)
      : m_device(p_device) {
        create(p_info);
    }

    void pipeline::create(const pipeline_settings& p_info) {
        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages(
          p_info.shader_modules.size());

        uint32_t shader_src_index = 0;

        // 1. Load in and setup the VKShaderModule handlers for VkPipeline
        for (const shader_handle& src : p_info.shader_modules) {
            VkShaderStageFlags stage = to_shader_stage(src.stage);
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
            .sType =
              VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        };

        VkPipelineViewportStateCreateInfo viewport_state = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount = 1,
        };

        //! @note Rasterization
        // Keep in mind: if lineWidth is zero, validation layers will occur
        // because cant be zero. Must be set to 1.0f
        VkPipelineRasterizationStateCreateInfo rasterizer_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = false,
            .rasterizerDiscardEnable =
              false, // set to true make fragmenta that are beyond near/far
                     // planes clamped to them as opposed to discarding them
            .polygonMode =
              VK_POLYGON_MODE_FILL, // if set to true then geometry never passes
                                    // through rasterizer stage. This basically
                                    // disables output to frame_buffer
            .cullMode = VK_CULL_MODE_NONE, // determines what culling to use.
                                           // Can also be disabled, culls
                                           // front-face, back-face or both
            .frontFace =
              VK_FRONT_FACE_COUNTER_CLOCKWISE, // specifies vertex order of
                                               // fdaces considered front-face
                                               // or clockwise/counter-clockwise
            .depthBiasEnable = false,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f,          // Optional
            .depthBiasSlopeFactor = 0.0f,    // Optional
            .lineWidth = 1.f
        };

        //! @note Multi-sampling
        VkPipelineMultisampleStateCreateInfo multisampling_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = false,
            // .minSampleShading = 1.0f,          // Optional
            // .pSampleMask = nullptr,            // Optional
            // .alphaToCoverageEnable = VK_FALSE, // Optional
            // .alphaToOneEnable = VK_FALSE,      // Optional
        };

        // Color blending Attachment -- blending color when the fragment returns
        // the color
        VkPipelineColorBlendAttachmentState color_blend_attachment = {
            .blendEnable = true,
            .srcColorBlendFactor =
              VK_BLEND_FACTOR_SRC_ALPHA, // Enabled: alpha blending
            .dstColorBlendFactor =
              VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, // Enabled: alpha blending
            .colorBlendOp = VK_BLEND_OP_ADD,       // Enabled: alpha blending
            .srcAlphaBlendFactor =
              VK_BLEND_FACTOR_ONE, // Enabled: alpha blending
            .dstAlphaBlendFactor =
              VK_BLEND_FACTOR_ZERO,          // Enabled: alpha blending
            .alphaBlendOp = VK_BLEND_OP_ADD, // Enabled: alpha blending
            .colorWriteMask =
              VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blending_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
            // these are optional
            .blendConstants = { 0.f, 0.f, 0.f, 0.f } // optional
        };

        // Enable depth-stencil state
        VkPipelineDepthStencilStateCreateInfo pipeline_deth_stencil_state_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = true,
            .depthWriteEnable = true,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = false,
            .stencilTestEnable = false,
        };

        //! @note Dynamic State
        //! @note -- pipeline states needs to be baked into the pipeline state
        std::array<VkDynamicState, 2> dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
            .pDynamicStates = dynamic_states.data()
        };

        VkPipelineLayoutCreateInfo pipeline_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };

        //! This is just to double-check that the descriptor set layout is
        //! valid. If the descriptor set layout is invalid, then proceed but not
        //! use the descriptor set layout
        // if (m_descriptor_set_layout != nullptr) {
        // if (!m_descriptor_layouts.empty()) {
        //     pipeline_layout_ci.setLayoutCount =
        //       static_cast<uint32_t>(m_descriptor_layouts.size());
        //     pipeline_layout_ci.pSetLayouts = m_descriptor_layouts.data();
        // }
        // else {
        //     // TODO: Uncomment this when adding back in descriptor sets
        //     // For now I will disable it to get the base working and add
        //     // descriptor sets back in afterwards
        //     pipeline_layout_ci.setLayoutCount = 0;
        //     pipeline_layout_ci.pSetLayouts = nullptr;
        // }
        pipeline_layout_ci.setLayoutCount =
          static_cast<uint32_t>(p_info.descriptor_layouts.size());
        pipeline_layout_ci.pSetLayouts = p_info.descriptor_layouts.data();
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
            // .pDepthStencilState = nullptr, // Optional
            .pDepthStencilState = &pipeline_deth_stencil_state_ci,
            .pColorBlendState = &color_blending_ci,
            .pDynamicState = &dynamic_state_ci,
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

    void pipeline::bind(const VkCommandBuffer& p_command) {
        vkCmdBindPipeline(
          p_command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    }

    void pipeline::destroy() {
        if (m_pipeline_layout != nullptr) {
            vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        }
        if (m_pipeline != nullptr) {
            vkDestroyPipeline(m_device, m_pipeline, nullptr);
        }
    }
};