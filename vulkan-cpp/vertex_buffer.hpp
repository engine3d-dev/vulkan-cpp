#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {
    /**
     * @brief vulkan implementation of vertex buffer that takes in vertices
     * 
     * TODO: vk::copy(buffer_src, buffer_dst, size_bytes) should use vk::device_queue. As right now it just uses logical device to get the device queue
    */
    class vertex_buffer {
    public:
        vertex_buffer() = default;
        vertex_buffer(const VkDevice& p_device, const vertex_buffer_info& p_vertices);

        [[nodiscard]] uint32_t size_bytes() const { return m_size_bytes; }

        [[nodiscard]] uint32_t size() const { return m_size; }

        [[nodiscard]] bool alive() const { return m_vertex_buffer.handle; }

        void bind(const VkCommandBuffer& p_current);

        operator VkBuffer() const { return m_vertex_buffer.handle; }

        operator VkBuffer() { return m_vertex_buffer.handle; }

        void destroy();

    private:
        VkDevice m_device=nullptr;
        uint32_t m_size_bytes = 0;
        uint32_t m_size=0;
        buffer_handle m_vertex_buffer={};
    };
};