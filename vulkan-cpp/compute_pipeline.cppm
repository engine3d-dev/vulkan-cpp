module;

#include <vulkan/vulkan.h>
#include <span>

export module vk::compute_pipeline;

import :types;
import :utilities;

export namespace vk {
    inline namespace v6 {

        struct compute_pipeline_params {
            const shader_handle shader_module{};
            std::span<VkDescriptorSetLayout> descriptor_layouts{};
            std::span<const push_constant_range> push_constants{};
            VkPipeline base_pipeline-1;
            int32_t base_pipeline_index-1;
        };

        class compute_pipeine {
        public:

            compute_pipeline(const VkDevice& p_device, const compute_pipeline_params& p_params) : m_device(p_device) {
                construct(p_params);
            }


            /**
             * @brief Constructs the compute pipeline and initialized based on the set parameters using vk::compute_pipeline_params struct
            */
            void construct(const compute_pipeline_params& p_params) {
                const shader_handle src = p_params.shader_module;

                // Retrieving the compiled compute shader module
                VkPipelineShaderStageCreateInfo shader_module = {
                    .sType =
                        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage = static_cast<VkShaderStageFlagBits>(src.stage),
                        .module = src.module,
                        .pName = "main"
                };

                VkPipelineLayoutCreateInfo pipeline_layout_ci = {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .setLayoutCount =
                      static_cast<uint32_t>(p_params.descriptor_layouts.size()),
                    .pSetLayouts = p_params.descriptor_layouts.data(),
                    .pushConstantRangeCount =
                      static_cast<uint32_t>(push_constants.size()),
                    .pPushConstantRanges = push_constants.data(),
                };

                vk_check(vkCreatePipelineLayout(m_device, &pipeline_layout_ci, nullptr, &m_layout), "vkCreatePipelineLayout");

                VkComputePipelineCreateInfo pipeline_ci = {
                    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .stage = shader_module,
                    .layout = m_layout,
                    .basePipelineHandle = p_params.base_pipeline,
                    .basePipelineIndex = p_params.base_pipeline_index,
                };

                vk_check(vkCreateComputePipelines(m_device, nullptr, 1, &pipeline_ci, nullptr, &m_pipeline), "vkCreateComputePipelines");
            }

            [[nodiscard]] VkPipelineLayout layout() const { return m_layout; }

            void bind(const VkCommandBuffer& p_command) {
                vkCmdBindPipeline(p_command, static_cast<VkPipelineBindPoint>(pipeline_bind_point::compute));
            }

            /**
             * @brief Update values of push constants
             *
             * Push constants let us send small amount of data with a small
             * limited size to the shader
             *
             * Push constants only accept up to a max of 128 bytes of push
             * constant data
             *
             * They allow for sending data to any specified shader stage (both
             * vertex and fragment) that are stored within the command buffer
             * itself
             *
             * Using push constants are commonly used when you have chunks of
             * data that may be changed every frame.
             *
             * Data that may need to be updated every frame such as camera
             * projection/view and model matrices.
             *
             * Example Code Usage:
             *
             * ```C++
             *
             * vk::compute_pipeline comp_pipeline(logical_device, pipeline_params_specified;
             *
             * comp_pipeline.push_constant(current, compute_data, 0);
             *
             * ```
             *
             * @tparam T is the type of the push constant
             * @tparam max_size parameter for controlling max of bytes to send
             *
             * @param p_current current command to push constants directly to
             * the shader
             * @param p_stage automated to be specified as shader_stage::compute for compute_pipeline
             * @param p_offset is specified of the beginning of the offset to
             * start from
             * @param p_range is the range of bytes of the specified push
             * constant data
             * @param p_data is the data that is represented into bytes to push
             * constants
             *
             * More info on vulkan's official
             * [docs](https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdPushConstants.html)
             */
            template<typename T, size_t max_size = 128>
            void push_constant(const VkCommandBuffer& p_current,
                               const T& p_data,
                               uint32_t p_offset) {

                // Perform compile-time checks if push constant data exceeds
                // maximum size of data to be transferred via push constant
                static_assert(sizeof(T) <= max_size,
                              "Type T exceeds max allowed size of bytes for "
                              "push constants.");

                vkCmdPushConstants(p_current,
                                   m_layout,
                                   static_cast<VkShaderStageFlags>(shader_stage::compute),
                                   p_offset,
                                   sizeof(T),
                                   &p_data);
            }

            void destruct() {
                if (m_layout != nullptr) {
                    vkDestroyPipelineLayout(
                      m_device, m_layout, nullptr);
                }

                if (m_pipeline != nullptr) {
                    vkDestroyPipeline(m_device, m_pipeline, nullptr);
                }
            }

            operator VkPipeline() const { return m_pipeline; }

            // Allows for `vk::pipeline` to be treated as a VkPipeline handle in raw Vulkan APIs
            operator VkPipeline() { return m_pipeline; }


        private:
            VkDevice m_device=nullptr;
            VkPipelineLayout m_layout;
            VkPipeline m_pipeline;
        };
    };
};