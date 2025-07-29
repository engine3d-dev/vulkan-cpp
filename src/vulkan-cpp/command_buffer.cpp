#include <vulkan-cpp/command_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {
    command_buffer::command_buffer(const VkDevice& p_device, const command_enumeration& p_enumerate_command_info) : m_device(p_device) {
        VkCommandPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = (VkCommandPoolCreateFlags)p_enumerate_command_info.pool_flag,
            .queueFamilyIndex = p_enumerate_command_info.queue_index
        };

        vk_check(
          vkCreateCommandPool(m_device, &pool_ci, nullptr, &m_command_pool),
          "vkCreateCommandPool");

        VkCommandBufferAllocateInfo command_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_command_pool,
            .level = to_vk_command_buffer_level(p_enumerate_command_info.levels),
            .commandBufferCount = 1
        };

        vk_check(vkAllocateCommandBuffers(m_device,
                                          &command_buffer_alloc_info,
                                          &m_command_buffer),
                 "vkAllocateCommandBuffers");
    }

    void command_buffer::begin(VkCommandBufferUsageFlags p_usage) {
        // Resets to zero if get called every frame
        if(m_begin_end_count == 2) {
            m_begin_end_count = 0;
        }
        m_begin_end_count++;
        VkCommandBufferBeginInfo command_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = p_usage
        };
        vk_check(
          vkBeginCommandBuffer(m_command_buffer, &command_begin_info),
          "vkBeginCommandBuffer");
    }

    void command_buffer::end() {
        m_begin_end_count++;
        vkEndCommandBuffer(m_command_buffer);
    }

    void command_buffer::destroy() {
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    }
};