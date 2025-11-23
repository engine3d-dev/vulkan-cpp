#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    /**
     * @brief Represents a VkBuffer handler for creating generic-buffer handlers
     *
     * Creating vertex, index, and uniform buffers use this buffer class to
     * create buffer handles. This buffer handle, will only accept writing data
     * to it if requested but not up front
     */
    class buffer_handler {
    public:
        buffer_handler() = default;
        buffer_handler(const VkDevice& p_device,
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
         * @param p_data is the bytes to write into the GPU's memory through the
         * Vulkan vkMapMemory/vkUnmapMemory API's.
         */
        void write(std::span<const uint8_t> p_data) {
            void* mapped = nullptr;
            vk_check(
              vkMapMemory(
                m_device, m_device_memory, 0, p_data.size_bytes(), 0, &mapped),
              "vkMapMemory");
            memcpy(mapped, p_data.data(), p_data.size_bytes());
            vkUnmapMemory(m_device, m_device_memory);
        }

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