#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/buffer_streams32.hpp>

namespace vk {

    class index_buffer {
    public:
        index_buffer() = default;
        index_buffer(const VkDevice& p_device,
                     const index_params& p_info);

        [[nodiscard]] bool alive() const { return m_index_buffer; }

        [[nodiscard]] uint32_t size() const { return m_indices_count; }

        void bind(const VkCommandBuffer& p_current);

        operator VkBuffer() const { return m_index_buffer; }

        operator VkBuffer() { return m_index_buffer; }

        void destroy();

    private:
        VkDevice m_device = nullptr;
        uint32_t m_indices_count = 0;
        buffer_stream32 m_index_buffer{};
    };
};