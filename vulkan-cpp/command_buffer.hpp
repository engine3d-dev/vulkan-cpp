#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    struct command_inherit_info {
        VkRenderPass renderpass = nullptr;
        uint32_t subpass_index = 0;
        VkFramebuffer framebuffer = nullptr;
    };

    /**
     * @brief vk::command_buffer represents the wrapper around VkCommandBuffer
     * 
     * Defines the command buffer and provides API's to directly interact with how
     * the command buffer may be utilized based on the needs of the application that uses it
     * 
    */
    class command_buffer {
    public:
        command_buffer() = default;
        command_buffer(const VkDevice& p_device,
                       const command_enumeration& p_enumerate_command_info);

        /**
         * @brief begins the recording operation of the particular command buffer
         * 
         * @param p_usage are flags to specify the behavior of the command buffer
         * @param p_inherit_info is if this command buffer is created and used as a secondary command buffer, then
         * this defines any state that will be inherited from the primary command buffer
         * 
         * 
         * ```C++
         * 
         * vk::command_buffer temp_command(logical_device, ....);
         * 
         * temp_command.begin(command_usage::one_time_submit);
         * temp_command.end(); // required whenever .begin is called
         * ```
         * 
        */
        void begin(command_usage p_usage,
                   std::span<const command_inherit_info> p_inherit_info = {});

        /**
         * @brief ends command buffer recording operation
        */
        void end();

        /**
         * 
         * @brief Copy from the source buffer to a destination specified buffer
         * 
         * 
         * Command buffer records this and expects a staging buffer  to map chunks of data in GPU memory
         * 
         * Then calling this API to handle copying the sources in staging buffer to its final destination buffer, moving those chunks into the final buffer handle
         * 
         * 
         * @param p_src is the buffer to copy its region from
         * @param p_dst is the buffer to copy that region into
         * @param p_size_bytes is the amount of bytes stored in the buffer that is being copied into the destination buffer
         * 
         * 
         * ```C++
         * 
         * // staging buffer to make sure we copy data regions chunks to vertex buffer correctly
         * vk::buffer_stream staging_buffer(logical_device, ...);
         * 
         * // vertex_buffer handle is the destination to copy the regions to
         * vk::buffer_stream vertex_buffer(logical_device, ...);
         * 
         * vk::command_buffer temp_command(logical_device, ...);
         * 
         * temp_command.begin(command_usage::one_time_submit);
         * temp_command.copy(staging_buffer, vertex_buffer, size_bytes);
         * temp_command.end();
         * 
         * 
         * ```
         * 
        */
        void copy_buffer(const VkBuffer& p_src, const VkBuffer& p_dst, uint64_t p_size_bytes);

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