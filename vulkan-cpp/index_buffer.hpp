#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>

namespace vk {

    class index_buffer {
    public:
        index_buffer() = default;
        index_buffer(const VkDevice& p_device, const index_buffer_info& p_indices);

        [[nodiscard]] bool alive() const { return m_index_buffer.handle; }

        [[nodiscard]] uint32_t size() const { return m_indices_count; }

        void bind(const VkCommandBuffer& p_current);

        operator VkBuffer() const { return m_index_buffer.handle; }

        operator VkBuffer() { return m_index_buffer.handle; }

        void destroy();
        

    private:
        VkDevice m_device=nullptr;
        uint32_t m_indices_count = 0;
        buffer_handle m_index_buffer{};
    };
};