#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    /**
     * @brief Represents a VkBuffer handler for creating VkBuffer handle
	 * 
	 * Purpose for using VkBuffer handle to streaming bytes of data into the GPU memory
	 * 
     */
    class buffer_streams {
    public:
        buffer_streams() = default;
        buffer_streams(const VkDevice& p_device,
                       const buffer_settings& p_settings);

        /**
         * @param span<T> writes some buffer data in the GPU's memory using
         * vkMapMemory/vkUnmapMemory
         */
        template<typename T>
        void write(std::span<const T> p_in_data) {
            VkDeviceSize buffer_size = p_in_data.size_bytes();
            void* mapped = nullptr;
            vk_check(vkMapMemory(
                       m_device, m_device_memory, 0, buffer_size, 0, &mapped),
                     "vkMapMemory");
            memcpy(mapped, p_in_data.data(), buffer_size);
            vkUnmapMemory(m_device, m_device_memory);
        }

        void write(const void* p_in_data, uint32_t p_size_bytes) {
            void* mapped = nullptr;
            vk_check(vkMapMemory(
                       m_device, m_device_memory, 0, p_size_bytes, 0, &mapped),
                     "vkMapMemory");
            memcpy(mapped, p_in_data, p_size_bytes);
            vkUnmapMemory(m_device, m_device_memory);
        }

        /**
         *
         * @brief This function automatically assumes the destination image
         * layout is going to be set to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
         *
         * @param p_command is the current command buffer to perform and store
         * this operation into
         * @param p_image is the destination to copy data from the buffer to
         * @param p_extent is the size of the image that is being copied
         *
		 * ```C++
		 * 
		 * buffer_streams texture_image(logical_device, ...);
		 * 
		 * texture_image.copy(temp_command_buffer, texture_image, texture_format, old_layout, new_layout);
		 * ```
		 * 
         */
        void copy_to_image(const VkCommandBuffer& p_command,
                           const VkImage& p_image,
                           image_extent p_extent);

        /**
         * @param p_data is the bytes to write into the GPU's memory through the
         * Vulkan vkMapMemory/vkUnmapMemory API's.
		 * 
		 * ```C++
		 * 
		 * buffer_streams staging_buffer(logical_device, ...);
		 * 
		 * std::array<uint8_t, 4> white_color = { 0xFF, 0xFF, 0xFF, 0xFF };
		 * staging_buffer.write(white_color);
		 * ```
		 * 
         */
        void write(std::span<const uint8_t> p_data);

        void destroy();

        operator VkBuffer() const { return m_handle; }

        operator VkBuffer() { return m_handle; }

    private:
        VkDevice m_device = nullptr;
        VkBuffer m_handle;
        VkDeviceMemory m_device_memory;
        uint32_t m_allocation_size = 0;
    };
};