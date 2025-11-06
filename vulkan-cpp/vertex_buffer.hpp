#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vulkan-cpp/buffer.hpp>

namespace vk {
    /**
     * @brief vulkan implementation for loading in vertices to a vulkan buffer handle
     * 
     * This implementation automates handle in loading the vertices and its memories for it
     */
    class vertex_buffer {
    public:
        vertex_buffer() = default;
        vertex_buffer(const VkDevice& p_device,
                      const vertex_buffer_settings& p_vertices);

        [[nodiscard]] uint32_t size_bytes() const { return m_size_bytes; }

        [[nodiscard]] uint32_t size() const { return m_size; }

        [[nodiscard]] bool alive() const { return m_vertex_handler; }

        void bind(const VkCommandBuffer& p_current);

        operator VkBuffer() const { return m_vertex_handler; }

        operator VkBuffer() { return m_vertex_handler; }

        void destroy();

    private:
        VkDevice m_device = nullptr;
        uint32_t m_size_bytes = 0;
        uint32_t m_size = 0;
        buffer_handler m_vertex_handler;
    };
};