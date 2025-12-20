module;

#include <span>
#include <vulkan/vulkan.h>
#include <limits>

export module vk:device_present_queue;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        /**
         * @name device_present_queue
         * @brief Represents a presentation queue that must have an associated
         * context to present images to the screen
         *
         * This class is different from device_queue. device_present_queue
         * represents a presentable queue for displaying to some specific screen
         * context
         */
        class device_present_queue {
        public:
            device_present_queue() = default;
            device_present_queue(const VkDevice& p_device,
                                const VkSwapchainKHR& p_swapchain_context,
                                const queue_params& p_config) 
                                    : m_device(p_device), m_swapchain(p_swapchain_context) {
                 
                 vkGetDeviceQueue(
                    m_device, p_config.family, p_config.index, &m_queue_handler);

                m_work_completed = create_semaphore(m_device);
                m_presentation_completed = create_semaphore(m_device);
                m_out_of_date = false;
            }

            void wait_idle() {
                vkQueueWaitIdle(m_queue_handler);
            }

            //! @return true if this queue is out of date
            // Can occur when acquired_next_image or present_frame are out of date
            // indication swapchain resizeability.
            // TODO: Change this to using C++'s exceptions for handling out-of-date invalidation cases
            bool out_of_date(bool p_is_reset = true) {
                // The return value we return
                bool return_value = false;

                // If the bool is set to true meaning its out of date
                // Then we set the internal variable tracking the acquired next image
                // state to false, and return false This is for ensuring that we do not
                // need to set the boolean ourselves. Should we handle this state to
                // reset to false if checked or user should set the state???
                if (m_out_of_date) {
                    return_value = m_out_of_date;
                    if (p_is_reset) {
                        m_out_of_date = false;
                    }
                    return return_value;
                }

                return return_value;
            }

            uint32_t acquire_next_image() {
                wait_idle();

                uint32_t image_acquired;
                VkResult acquired_next_image_res =
                vkAcquireNextImageKHR(m_device,
                                        m_swapchain,
                                        std::numeric_limits<uint32_t>::max(),
                                        m_presentation_completed,
                                        nullptr,
                                        &image_acquired);

                if (acquired_next_image_res == VK_ERROR_OUT_OF_DATE_KHR) {
                    m_out_of_date = true;
                }

                vk_check(acquired_next_image_res, "vkAcquireNextImageKHR");

                return image_acquired;
            }

            //! @brief Submit commands to this specific present queue (without
            //! asynchronously)
            void submit_sync(std::span<const VkCommandBuffer> p_commands) {
                VkSubmitInfo submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .waitSemaphoreCount = 0,
                    .pWaitSemaphores = nullptr,
                    .pWaitDstStageMask = nullptr,
                    .commandBufferCount = static_cast<uint32_t>(p_commands.size()),
                    .pCommandBuffers = p_commands.data(),
                    .signalSemaphoreCount = 0,
                    .pSignalSemaphores = nullptr,
                };

                VkResult res = vkQueueSubmit(m_queue_handler, 1, &submit_info, nullptr);
                vk_check(res, "vkQueueSubmit");
            }

            //! @brief Submit commands to this specific present queue
            //! (asynchronously)
            void submit_async(std::span<const VkCommandBuffer> p_commands,
                            pipeline_stage_flags p_flags =
                                pipeline_stage_flags::color_attachment_output) {
                VkPipelineStageFlags flags = static_cast<VkPipelineStageFlags>(p_flags);
                VkSubmitInfo submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .pNext = nullptr,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &m_presentation_completed,
                    .pWaitDstStageMask = &flags,
                    .commandBufferCount = static_cast<uint32_t>(p_commands.size()),
                    .pCommandBuffers = p_commands.data(),
                    .signalSemaphoreCount = 1,
                    .pSignalSemaphores = &m_work_completed,
                };

                VkResult res = vkQueueSubmit(m_queue_handler, 1, &submit_info, nullptr);
                vk_check(res, "vkQueueSubmit");
            }

            //! @brief Displays specific image to the presentation frame with
            //! specific frame index
            void present_frame(uint32_t p_frame_idx) {
                VkPresentInfoKHR present_info = {
                    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                    .pNext = nullptr,
                    .waitSemaphoreCount = 1,
                    .pWaitSemaphores = &m_work_completed,
                    .swapchainCount = 1,
                    .pSwapchains = &m_swapchain,
                    .pImageIndices = &p_frame_idx,
                };

                VkResult res = vkQueuePresentKHR(m_queue_handler, &present_info);
                vk_check(res, "vkQueuePresentKHR");
                if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
                    m_out_of_date = true;
                }
            }

            void destroy() {
                vkDeviceWaitIdle(m_device);
                vkDestroySemaphore(m_device, m_presentation_completed, nullptr);
                vkDestroySemaphore(m_device, m_work_completed, nullptr);
            }

            operator VkQueue() { return m_queue_handler; }

            operator VkQueue() const { return m_queue_handler; }

        private:
            VkDevice m_device = nullptr;
            bool m_out_of_date = false;
            VkSwapchainKHR m_swapchain = nullptr;
            VkQueue m_queue_handler = nullptr;
            VkSemaphore m_work_completed = nullptr;
            VkSemaphore m_presentation_completed = nullptr;
        };
    };
};