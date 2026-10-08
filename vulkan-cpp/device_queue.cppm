module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>

export module vk:device_queue;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v6 {
        /**
         * @name
         * @brief Represents a queue part of a specific logical device created
         */
        class device_queue {
        public:
            device_queue() = default;

            device_queue(const VkDevice& p_device,
                         const queue_params& p_params) {
                vkGetDeviceQueue(
                  p_device, p_params.family, p_params.index, &m_queue_handler);
            }

            void wait_idle() { vkQueueWaitIdle(m_queue_handler); }

            /**
             *
             * @brief Performs queue submission without timeline semaphore
             * support.
             *
             */
            void submit(std::span<const VkCommandBuffer> p_commands,
                        std::span<const VkSemaphore> p_waits = {},
                        std::span<const VkSemaphore> p_signals = {},
                        pipeline_stage_flags p_flags =
                          pipeline_stage_flags::color_attachment_output) {
                VkPipelineStageFlags flags =
                  static_cast<VkPipelineStageFlags>(p_flags);
                VkSubmitInfo submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .waitSemaphoreCount = static_cast<uint32_t>(p_waits.size()),
                    .pWaitSemaphores = p_waits.data(),
                    .pWaitDstStageMask = &flags,
                    .commandBufferCount =
                      static_cast<uint32_t>(p_commands.size()),
                    .pCommandBuffers = p_commands.data(),
                    .signalSemaphoreCount =
                      static_cast<uint32_t>(p_signals.size()),
                    .pSignalSemaphores = p_signals.data(),
                };

                vk_check(
                  vkQueueSubmit(m_queue_handler, 1, &submit_info, nullptr),
                  "vkQueueSubmit");
            }

            /**
             * @brief Performs queue submissions with timeline semaphore support
             *
             * Requires VkPhysicalDeviceSynchronization2Features to be enabled.
             *
             */
            void submit2(std::span<const VkCommandBuffer> p_commands,
                         std::span<const VkSemaphoreSubmitInfo> p_waits = {},
                         std::span<const VkSemaphoreSubmitInfo> p_signals = {},
                         pipeline_stage_flags p_flags =
                           pipeline_stage_flags::color_attachment_output) {

                std::vector<VkCommandBufferSubmitInfo> command_infos;
                command_infos.reserve(p_commands.size());

                for (uint32_t i = 0; i < command_infos.size(); i++) {
                    command_infos[i] = {
                        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                        .commandBuffer = p_commands[i],
                    };
                }

                VkSubmitInfo2 submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
                    .pNext = nullptr,
                    .waitSemaphoreInfoCount =
                      static_cast<uint32_t>(p_waits.size()),
                    .pWaitSemaphoreInfos = p_waits.data(),
                    .signalSemaphoreInfoCount =
                      static_cast<uint32_t>(p_signals.size()),
                    .pSignalSemaphoreInfos = p_signals.data(),
                };

                vk_check(
                  vkQueueSubmit2(m_queue_handler, 1, &submit_info, nullptr),
                  "vkQueueSubmit2");
            }

            [[nodiscard]] bool alive() const { return m_queue_handler; }

            operator VkQueue() const { return m_queue_handler; }

            operator VkQueue() { return m_queue_handler; }

        private:
            VkQueue m_queue_handler = nullptr;
        };
    };
};