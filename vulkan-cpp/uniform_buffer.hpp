#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <variant>

namespace vk {
    /**
     * @brief represents a vulkan uniform buffer
     * 
     * Maps uniforms and gpu-specific resources
    */
    class uniform_buffer {
    public:
        uniform_buffer() = default;
        uniform_buffer(const VkDevice& p_device, const uniform_buffer_info& p_uniform_info);

        [[nodiscard]] bool alive() const { return m_uniform_buffer.handle; }

        void update(const void* p_data);

        operator VkBuffer() const { return m_uniform_buffer.handle; }

        operator VkBuffer() { return m_uniform_buffer.handle; }

        [[nodiscard]] uint32_t size_bytes() const { return m_size_bytes; }

        void destroy();

    private:
        uint32_t m_size_bytes=0;
        VkDevice m_device=nullptr;
        buffer_handle m_uniform_buffer{};
    };
};