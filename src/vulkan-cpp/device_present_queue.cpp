#include <vulkan-cpp/device_present_queue.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <numeric>

namespace vk {

    device_present_queue::device_present_queue(const VkDevice& p_device, const VkSwapchainKHR& p_swapchain_context, const queue_enumeration& p_config) : m_device(p_device), m_swapchain(p_swapchain_context) {
        vkGetDeviceQueue(m_device, p_config.family, p_config.index, &m_queue_handler);

        m_work_completed = create_semaphore(m_device);
        m_presentation_completed = create_semaphore(m_device);
        m_out_of_date = false;
    }

    void device_present_queue::wait_idle() {
        vkQueueWaitIdle(m_queue_handler);
    }

    bool device_present_queue::out_of_date(bool p_is_reset) {
        // The return value we return
        bool return_value = false;

        // If the bool is set to true meaning its out of date
        // Then we set the internal variable tracking the acquired next image state to false, and return false
        // This is for ensuring that we do not need to set the boolean ourselves.
        // Should we handle this state to reset to false if checked or user should set the state???
        if(m_out_of_date) {
            return_value = m_out_of_date;
            if(p_is_reset) {
                m_out_of_date = false;
            }
            return return_value;
        }

        return return_value;
    }

    uint32_t device_present_queue::acquire_next_image() {
        wait_idle();
        
        uint32_t image_acquired;
        VkResult acquired_next_image_res = vkAcquireNextImageKHR(m_device, m_swapchain, std::numeric_limits<uint32_t>::max(), m_presentation_completed, nullptr, &image_acquired);

        if (acquired_next_image_res == VK_ERROR_OUT_OF_DATE_KHR) {
            m_out_of_date = true;
        }

        vk_check(acquired_next_image_res, "vkAcquireNextImageKHR");

        return image_acquired;
    }

    void device_present_queue::submit_sync(const VkCommandBuffer& p_command) {
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 0,
            .pWaitSemaphores = nullptr,
            .pWaitDstStageMask = nullptr,
            .commandBufferCount = 1,
            .pCommandBuffers = &p_command,
            .signalSemaphoreCount = 0,
            .pSignalSemaphores = nullptr,
        };

        VkResult res =
          vkQueueSubmit(m_queue_handler, 1, &submit_info, nullptr);
        vk_check(res, "vkQueueSubmit");

    }

    void device_present_queue::submit_async(const VkCommandBuffer& p_command, VkPipelineStageFlags p_flags) {
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &m_presentation_completed,
            .pWaitDstStageMask = &p_flags,
            .commandBufferCount = 1,
            .pCommandBuffers = &p_command,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &m_work_completed,
        };

        VkResult res = vkQueueSubmit(m_queue_handler, 1, &submit_info, nullptr);
        vk_check(res, "vkQueueSubmit");
    }

    void device_present_queue::present_frame(uint32_t p_frame_idx) {
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

    void device_present_queue::destroy() {
        vkDeviceWaitIdle(m_device);
        vkDestroySemaphore(m_device,m_presentation_completed, nullptr);
        vkDestroySemaphore(m_device, m_work_completed, nullptr);
    }
};