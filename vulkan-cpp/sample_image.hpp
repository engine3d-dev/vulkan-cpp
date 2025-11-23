#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    class sample_image {
    public:
        sample_image() = default;
        sample_image(const VkDevice& p_device,
                     const image_configuration_information& p_image_properties);
        sample_image(const VkDevice& p_device,
                     const VkImage& p_image,
                     const image_configuration_information& p_image_properties);

        [[nodiscard]] VkSampler sampler() const { return m_sampler; }

        [[nodiscard]] VkImageView image_view() const { return m_image_view; }


        /**
         * @brief performs vkCmdPipelineBarrier to handle transitioning image layouts
         * 
         * Vulkan has this responsibility be bestowed on the writer of their applications.
         * 
         * @param p_command is the current command buffer to operate the actual image layout transitions
         * @param p_format is the image format to make sure if there is a depth format available then request the aspect mask to include the stencil bit
         * @param p_old is the old image layout transition from
         * @param p_new is the new image layout transition to.
         * 
         * ```C++
         * 
         * sample_image texture_image(logical_device, ...);
         * 
         * 
         * texture_image.memory_barrier(temp_command, some_vk_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
         * 
         * // do some operation during this transition image layout
         * // such as copying from an buffer data to an image
         * // staging_buffer.copy_to_image(...);
         * 
         * texture_image.memory_barrier(temp_command, some_vk_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
         * 
         * ```
         * 
        */
        void memory_barrier(const VkCommandBuffer& p_command, VkFormat p_format, VkImageLayout p_old, VkImageLayout p_new);

        void destroy();

        operator VkImage() const { return m_image; }

        operator VkImage() { return m_image; }

    private:

    private:
        bool m_only_destroy_image_view = false;
        VkDevice m_device = nullptr;
        VkImage m_image = nullptr;
        VkImageView m_image_view = nullptr;
        VkSampler m_sampler = nullptr;
        VkDeviceMemory m_device_memory = nullptr;
    };
};