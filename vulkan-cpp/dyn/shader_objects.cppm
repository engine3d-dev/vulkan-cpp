module;

#include <string_view>
#include <span>
#include <vulkan/vulkan.h>
#include <vector>
#include <print>

export module vk:shader_objects;

import :types;
import :utilities;

export namespace vk::dyn::experimental {
    inline namespace v1 {
        struct shader_ext_params {
            shader_stage stage;
            shader_stage next_stage;
            shader_code_type code_type;
            std::span<const uint8_t> code_binary{};
            std::string_view name = "";
            std::span<const VkDescriptorSetLayout> descriptor_layouts;
            std::span<const push_constant_range> push_constants{};
        };

        // When querying the binary from the shaders
        // If the pData is null, then we can retrieve both either the error to
        // check if succeeded or the bytes needed when queried
        struct shader_ext_error {
            VkResult error;
            uint32_t bytes = 0;
        };

        class shader_ext {
        public:
            shader_ext(const VkDevice& p_device,
                       const shader_ext_params& p_params)
              : m_device(p_device) {

                vkCreateShadersEXT = reinterpret_cast<PFN_vkCreateShadersEXT>(
                  vkGetDeviceProcAddr(m_device, "vkCreateShadersEXT"));

                std::println("ShadersEXT = {}",
                             (vkCreateShadersEXT == nullptr));

                vkDestroyShaderEXT = reinterpret_cast<PFN_vkDestroyShaderEXT>(
                  vkGetDeviceProcAddr(m_device, "vkDestroyShaderEXT"));
                vkCmdBindShadersEXT = reinterpret_cast<PFN_vkCmdBindShadersEXT>(
                  vkGetDeviceProcAddr(m_device, "vkCmdBindShadersEXT"));
                vkGetShaderBinaryDataEXT =
                  reinterpret_cast<PFN_vkGetShaderBinaryDataEXT>(
                    vkGetDeviceProcAddr(m_device, "vkGetShaderBinaryDataEXT"));

                vkCmdSetAlphaToCoverageEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetAlphaToCoverageEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetAlphaToCoverageEnableEXT"));
                vkCmdSetColorBlendEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetColorBlendEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetColorBlendEnableEXT"));
                vkCmdSetColorWriteMaskEXT =
                  reinterpret_cast<PFN_vkCmdSetColorWriteMaskEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetColorWriteMaskEXT"));
                vkCmdSetCullModeEXT = reinterpret_cast<PFN_vkCmdSetCullModeEXT>(
                  vkGetDeviceProcAddr(m_device, "vkCmdSetCullModeEXT"));
                vkCmdSetDepthBiasEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetDepthBiasEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetDepthBiasEnableEXT"));
                vkCmdSetDepthCompareOpEXT =
                  reinterpret_cast<PFN_vkCmdSetDepthCompareOpEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetDepthCompareOpEXT"));
                vkCmdSetDepthTestEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetDepthTestEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetDepthTestEnableEXT"));
                vkCmdSetDepthWriteEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetDepthWriteEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetDepthWriteEnableEXT"));
                vkCmdSetFrontFaceEXT =
                  reinterpret_cast<PFN_vkCmdSetFrontFaceEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetFrontFaceEXT"));
                vkCmdSetPolygonModeEXT =
                  reinterpret_cast<PFN_vkCmdSetPolygonModeEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetPolygonModeEXT"));
                vkCmdSetPrimitiveRestartEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetPrimitiveRestartEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetPrimitiveRestartEnableEXT"));
                vkCmdSetPrimitiveTopologyEXT =
                  reinterpret_cast<PFN_vkCmdSetPrimitiveTopologyEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetPrimitiveTopologyEXT"));
                vkCmdSetRasterizationSamplesEXT =
                  reinterpret_cast<PFN_vkCmdSetRasterizationSamplesEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetRasterizationSamplesEXT"));
                vkCmdSetRasterizerDiscardEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetRasterizerDiscardEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetRasterizerDiscardEnableEXT"));
                vkCmdSetSampleMaskEXT =
                  reinterpret_cast<PFN_vkCmdSetSampleMaskEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetSampleMaskEXT"));
                vkCmdSetScissorWithCountEXT =
                  reinterpret_cast<PFN_vkCmdSetScissorWithCountEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetScissorWithCountEXT"));
                vkCmdSetStencilTestEnableEXT =
                  reinterpret_cast<PFN_vkCmdSetStencilTestEnableEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetStencilTestEnableEXT"));
                vkCmdSetVertexInputEXT =
                  reinterpret_cast<PFN_vkCmdSetVertexInputEXT>(
                    vkGetDeviceProcAddr(m_device, "vkCmdSetVertexInputEXT"));
                vkCmdSetViewportWithCountEXT =
                  reinterpret_cast<PFN_vkCmdSetViewportWithCountEXT>(
                    vkGetDeviceProcAddr(m_device,
                                        "vkCmdSetViewportWithCountEXT"));
                ;

                std::vector<VkPushConstantRange> push_constants(
                  p_params.push_constants.size());

                for (uint32_t i = 0; i < push_constants.size(); i++) {
                    const push_constant_range data = p_params.push_constants[i];
                    push_constants[i] = {
                        .stageFlags =
                          static_cast<VkShaderStageFlags>(data.stage),
                        .offset = data.offset,
                        .size = data.range,
                    };
                }

                VkShaderCreateInfoEXT shader_info_ci = {
                    .sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO_EXT,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = static_cast<VkShaderStageFlagBits>(p_params.stage),
                    .nextStage =
                      static_cast<VkShaderStageFlags>(p_params.next_stage),
                    .codeType =
                      static_cast<VkShaderCodeTypeEXT>(p_params.code_type),
                    .codeSize =
                      static_cast<uint32_t>(p_params.code_binary.size()),
                    .pCode = p_params.code_binary.data(),
                    .pName = p_params.name.data(),
                    .setLayoutCount =
                      static_cast<uint32_t>(p_params.descriptor_layouts.size()),
                    .pSetLayouts = p_params.descriptor_layouts.data(),
                    .pushConstantRangeCount =
                      static_cast<uint32_t>(push_constants.size()),
                    .pPushConstantRanges = push_constants.data(),
                };

                vk_check(
                  vkCreateShadersEXT(
                    m_device, 1, &shader_info_ci, nullptr, &m_shader_handle),
                  "vkCreateShadersEXT");
            }

            void bind(const VkCommandBuffer& p_command,
                      uint32_t p_stage_count,
                      std::span<const shader_stage> p_stages) {
                vkCmdBindShadersEXT(
                  p_command,
                  p_stage_count,
                  reinterpret_cast<const VkShaderStageFlagBits*>(
                    p_stages.data()),
                  &m_shader_handle);
            }

            void destroy() {
                vkDestroyShaderEXT(m_device, m_shader_handle, nullptr);
            }

        private:
            VkDevice m_device = nullptr;
            VkShaderEXT m_shader_handle = nullptr;

            PFN_vkCreateShadersEXT vkCreateShadersEXT{ nullptr };
            PFN_vkDestroyShaderEXT vkDestroyShaderEXT{ nullptr };
            PFN_vkCmdBindShadersEXT vkCmdBindShadersEXT{ nullptr };
            PFN_vkGetShaderBinaryDataEXT vkGetShaderBinaryDataEXT{ nullptr };

            // With VK_EXT_shader_object pipeline state must be set at command
            // buffer creation using these functions
            PFN_vkCmdSetAlphaToCoverageEnableEXT
              vkCmdSetAlphaToCoverageEnableEXT{ nullptr };
            PFN_vkCmdSetColorBlendEnableEXT vkCmdSetColorBlendEnableEXT{
                nullptr
            };
            PFN_vkCmdSetColorWriteMaskEXT vkCmdSetColorWriteMaskEXT{ nullptr };
            PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT{ nullptr };
            PFN_vkCmdSetDepthBiasEnableEXT vkCmdSetDepthBiasEnableEXT{
                nullptr
            };
            PFN_vkCmdSetDepthCompareOpEXT vkCmdSetDepthCompareOpEXT{ nullptr };
            PFN_vkCmdSetDepthTestEnableEXT vkCmdSetDepthTestEnableEXT{
                nullptr
            };
            PFN_vkCmdSetDepthWriteEnableEXT vkCmdSetDepthWriteEnableEXT{
                nullptr
            };
            PFN_vkCmdSetFrontFaceEXT vkCmdSetFrontFaceEXT{ nullptr };
            PFN_vkCmdSetPolygonModeEXT vkCmdSetPolygonModeEXT{ nullptr };
            PFN_vkCmdSetPrimitiveRestartEnableEXT
              vkCmdSetPrimitiveRestartEnableEXT{ nullptr };
            PFN_vkCmdSetPrimitiveTopologyEXT vkCmdSetPrimitiveTopologyEXT{
                nullptr
            };
            PFN_vkCmdSetRasterizationSamplesEXT vkCmdSetRasterizationSamplesEXT{
                nullptr
            };
            PFN_vkCmdSetRasterizerDiscardEnableEXT
              vkCmdSetRasterizerDiscardEnableEXT{ nullptr };
            PFN_vkCmdSetSampleMaskEXT vkCmdSetSampleMaskEXT{ nullptr };
            PFN_vkCmdSetScissorWithCountEXT vkCmdSetScissorWithCountEXT{
                nullptr
            };
            PFN_vkCmdSetStencilTestEnableEXT vkCmdSetStencilTestEnableEXT{
                nullptr
            };
            PFN_vkCmdSetViewportWithCountEXT vkCmdSetViewportWithCountEXT{
                nullptr
            };

            // VK_EXT_vertex_input_dynamic_state
            PFN_vkCmdSetVertexInputEXT vkCmdSetVertexInputEXT{ nullptr };
        };
    };
};