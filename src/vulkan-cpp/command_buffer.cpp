#include <vulkan-cpp/command_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {
    command_buffer::command_buffer(
      const VkDevice& p_device,
      const command_enumeration& p_enumerate_command_info)
      : m_device(p_device) {
        VkCommandPoolCreateInfo pool_ci = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            // .flags =
            // (VkCommandPoolCreateFlags)p_enumerate_command_info.pool_flag,
            .flags = (VkCommandPoolCreateFlags)to_command_buffer_pool_flags(
              p_enumerate_command_info.flags),
            .queueFamilyIndex = p_enumerate_command_info.queue_index
        };

        vk_check(
          vkCreateCommandPool(m_device, &pool_ci, nullptr, &m_command_pool),
          "vkCreateCommandPool");

        VkCommandBufferAllocateInfo command_buffer_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = m_command_pool,
            .level =
              to_vk_command_buffer_level(p_enumerate_command_info.levels),
            .commandBufferCount = 1
        };

        vk_check(vkAllocateCommandBuffers(
                   m_device, &command_buffer_alloc_info, &m_command_buffer),
                 "vkAllocateCommandBuffers");
    }

    void command_buffer::begin(
      command_usage p_usage,
      std::span<const command_inherit_info> p_inherit_info) {
        // Resets to zero if get called every frame
        if (m_begin_end_count == 2) {
            m_begin_end_count = 0;
        }
        m_begin_end_count++;

        std::vector<VkCommandBufferInheritanceInfo> inheritance_infos(
          p_inherit_info.size());

        for (size_t i = 0; i < inheritance_infos.size(); i++) {
            inheritance_infos[i] = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                .renderPass = p_inherit_info[i].renderpass,
                .subpass = p_inherit_info[i].subpass_index,
                .framebuffer = p_inherit_info[i].framebuffer
            };
        }

        VkCommandBufferBeginInfo command_begin_info = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = static_cast<VkCommandBufferUsageFlags>(p_usage)
        };
        vk_check(vkBeginCommandBuffer(m_command_buffer, &command_begin_info),
                 "vkBeginCommandBuffer");
    }

    void command_buffer::end() {
        m_begin_end_count++;
        vkEndCommandBuffer(m_command_buffer);
    }

	void command_buffer::copy_buffer(const VkBuffer& p_src, const VkBuffer& p_dst, uint64_t p_size_bytes) {
		VkBufferCopy copy_region{};
		copy_region.size = p_size_bytes;
		vkCmdCopyBuffer(m_command_buffer, p_src, p_dst, 1, &copy_region);
	}

    void command_buffer::execute(std::span<const VkCommandBuffer> p_commands) {
        vkCmdExecuteCommands(m_command_buffer,
                             static_cast<uint32_t>(p_commands.size()),
                             p_commands.data());
    }

    void command_buffer::destroy() {
        vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
        vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    }
};