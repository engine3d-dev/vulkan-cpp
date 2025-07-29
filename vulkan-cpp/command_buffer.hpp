#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {
    
    class command_buffer {
    public:
        command_buffer() = default;
        command_buffer(const VkDevice& p_device, const command_enumeration& p_enumerate_command_info);

        void begin(VkCommandBufferUsageFlags p_usage);
        void end();

        [[nodiscard]] bool alive() const { return m_command_buffer; }

        void destroy();

        operator VkCommandBuffer() const { return m_command_buffer; }

        operator VkCommandBuffer() { return m_command_buffer; }

    private:
        VkDevice m_device=nullptr;
        uint32_t m_begin_end_count=0;
        VkCommandPool m_command_pool=nullptr;
        VkCommandBuffer m_command_buffer=nullptr;
    };
};