#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    struct command_inherit_info {
        VkRenderPass renderpass = nullptr;
        uint32_t subpass_index = 0;
        VkFramebuffer framebuffer = nullptr;
    };

    class command_buffer {
    public:
        command_buffer() = default;
        command_buffer(const VkDevice& p_device,
                       const command_enumeration& p_enumerate_command_info);

        void begin(command_usage p_usage,
                   std::span<const command_inherit_info> p_inherit_info = {});
        void end();

        [[nodiscard]] bool alive() const { return m_command_buffer; }

        /**
         * @brief Used to execute secondary command buffers
         *
         * The command buffer that is executing these commands must be a
         * specified primary command buffer
         *
         * @param p_commands is the secondary command buffer that gets executed
         * if the command buffer itself is a primary command buffer
         */
        void execute(std::span<const VkCommandBuffer> p_commands);

        void destroy();

        operator VkCommandBuffer() const { return m_command_buffer; }

        operator VkCommandBuffer() { return m_command_buffer; }

    private:
        VkDevice m_device = nullptr;
        uint32_t m_begin_end_count = 0;
        VkCommandPool m_command_pool = nullptr;
        VkCommandBuffer m_command_buffer = nullptr;
    };
};