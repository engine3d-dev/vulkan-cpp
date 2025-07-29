#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    /**
     * @name device_present_queue
     * @brief Represents a presentation queue that must have an associated context to present images to the screen
     * 
     * This class is different from device_queue. device_present_queue represents a presentable queue for displaying to some specific screen context
    */
    class device_present_queue {
    public:
        device_present_queue(const VkDevice& p_device, const VkSwapchainKHR& p_swapchain_context, const queue_enumeration& p_config);

        void wait_idle();

        uint32_t acquire_next_image();

        //! @brief Submit commands to this specific present queue (without asynchronously)
        void submit_sync(const VkCommandBuffer& p_command);

        //! @brief Submit commands to this specific present queue (asynchronously)
        void submit_async(const VkCommandBuffer& p_command, VkPipelineStageFlags p_flags=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        //! @brief Displays specific image to the presentation frame with specific frame index
        void present_frame(uint32_t p_frame_idx);

        //! @return true if this queue is out of date
        // Can occur when acquired_next_image or present_frame are out of date indication swapchain resizeability.
        bool out_of_date(bool p_is_reset=true);

        void destroy();
        
        operator VkQueue() { return m_queue_handler; }

        operator VkQueue() const { return m_queue_handler; }

    private:
        VkDevice m_device=nullptr;
        bool m_out_of_date=false;
        VkSwapchainKHR m_swapchain=nullptr;
        VkQueue m_queue_handler=nullptr;
        VkSemaphore m_work_completed = nullptr;
        VkSemaphore m_presentation_completed=nullptr;
    };
};