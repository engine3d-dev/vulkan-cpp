#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>
#include <span>

namespace vk {
    /**
     * @brief buffer stream for streaming arbitrary buffers of 32-bytes
    */
    class buffer_stream32 {
    public:
        buffer_stream32() = default;
        buffer_stream32(const VkDevice& p_device, const buffer_parameters& p_params);

        /**
         * @brief write arbitrary buffer of 32-bytes to GPU-memory
        */
        void write(std::span<const uint32_t> p_data);

        void copy_to_image(const VkCommandBuffer& p_command, const VkImage& p_image, image_extent p_extent);

        void destroy();

        operator VkBuffer() { return m_handle; }

        operator VkBuffer() const { return m_handle; }

    private:
        VkDevice m_device=nullptr;
        VkDeviceMemory m_device_memory=nullptr;
        VkBuffer m_handle=nullptr;
    };
};