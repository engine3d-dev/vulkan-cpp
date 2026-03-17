module;

#include <span>
#include <vulkan/vulkan.h>
#include <vector>

export module vk:command_buffer;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        struct command_inherit_info {
            VkRenderPass renderpass = nullptr;
            uint32_t subpass_index = 0;
            VkFramebuffer framebuffer = nullptr;
        };

        /**
         * @brief vk::command_buffer is an abstraction around the
         * VkCommandBuffer.
         *
         * vk::command_buffer can be represented as a VkCommandBuffer
         * if the user decides to use the raw Vulkan API.
         *
         */
        class command_buffer {
        public:
            command_buffer() = default;
            command_buffer(const VkDevice& p_device,
                           const command_params& p_enumerate_command_info)
              : m_device(p_device) {
                VkCommandPoolCreateInfo pool_ci = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    // .flags =
                    // (VkCommandPoolCreateFlags)p_enumerate_command_info.pool_flag,
                    .flags = static_cast<VkCommandPoolCreateFlags>(
                      p_enumerate_command_info.flags),
                    .queueFamilyIndex = p_enumerate_command_info.queue_index
                };

                vk_check(vkCreateCommandPool(
                           m_device, &pool_ci, nullptr, &m_command_pool),
                         "vkCreateCommandPool");

                VkCommandBufferAllocateInfo command_buffer_alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .commandPool = m_command_pool,
                    .level = static_cast<VkCommandBufferLevel>(
                      p_enumerate_command_info.levels),
                    .commandBufferCount = 1
                };

                vk_check(vkAllocateCommandBuffers(m_device,
                                                  &command_buffer_alloc_info,
                                                  &m_command_buffer),
                         "vkAllocateCommandBuffers");
            }

            /**
             *
             * @brief Begin operation for GPU-specific work and where it is
             * defined. A common GPU workflow is queueing up tasks, in this case
             * commands.
             *
             * @param p_usage are flags to specify recording behavior
             * - ::one_time_submit: commands recorded once, then reset.
             * - ::simultaneous_use: Commands that can be re-submitted while
             * still running.
             *
             * @param p_inherit_info are used for command buffers specified as
             * secondary
             * - .renderpass: Inheriting renderpass handle from primary command.
             * - .framebuffer: Inherit the "image" target from primary command.
             *
             * The CPU cannot directly offload tasks to the GPU, therefore this
             * must be represented using command buffers which are queue'd up
             * during recording states of the command buffer.
             *
             * @brief Additional Considerations:
             * - Command buffer must be in 'initial' or 'executable' state
             * before calling .begin/end. You cannot call `begin()` on a buffer
             * that is already recording.
             * - If a command buffer was already recording, it must be reset
             * (manually or via
             * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT).
             * - Every begin() call must follow-up with an end() call
             * afterwards.
             * - If using p_inherit_info, the command buffer must be created
             * with the 'secondary' level flag.
             *
             *
             * [ CPU Command ]                  [Record Stream]
             * +----------------------+         +------------------------+
             * |   .begin() <- start  |         |  [Header: Usage Flags] |
             * |   [Draw Triangle]    | =====>  |  [Op: Bind Pipeline]   |
             * |   [Copy Image]       |         |                        |
             * |   .end()             |         |  [Op: Draw Call]       |
             * |                      |         |  [Footer: End Tag]     |
             * +----------------------+         +------------------------+
             *
             * Example Usage:
             *
             * ```C++
             *
             * vk::command_buffer primary_command(logical_device, ...);
             *
             * primary_command.begin(vk::command_usage::one_time_submit);
             * // perform GPU-specific work
             * primary_command.end();
             *
             * ```
             *
             *
             */
            void begin(
              command_usage p_usage,
              std::span<const command_inherit_info> p_inherit_info = {}) {
                // Resets to zero if get called every frame
                if (m_begin_end_count == 2) {
                    m_begin_end_count = 0;
                }
                m_begin_end_count++;

                std::vector<VkCommandBufferInheritanceInfo> inheritance_infos(
                  p_inherit_info.size());

                for (size_t i = 0; i < inheritance_infos.size(); i++) {
                    inheritance_infos[i] = {
                        .sType =
                          VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
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
                vk_check(
                  vkBeginCommandBuffer(m_command_buffer, &command_begin_info),
                  "vkBeginCommandBuffer");
            }

            /**
             * @brief Stops the recording process, command moves from
             * 'recording' to 'executing' state. Which implies the command is
             * ready to be submitted to the vk::device_queue handle.
             *
             * @brief Additional Considerations:
             * - Cannot end a command buffer if a renderpass is still active.
             * (must invoked renderpass::end beforehand).
             * - If the command_buffer::end returns an error, command handle
             * becomes invalid and cannot be submitted.
             *
             */
            void end() {
                m_begin_end_count++;
                vkEndCommandBuffer(m_command_buffer);
            }

            /**
             * @brief Performs high-speed raw memory transfers between two
             * buffer handles.
             *
             * Commonly used for uploading data by copying
             * between CPU-visible staging buffers to GPU-local buffers. (faster
             * for GPU to read)
             *
             * @param p_src is the handle to source buffer (incoming data)
             * @param p_dst is the handle to destination buffer (data
             * transfering to)
             * @param p_size_bytes are the total amount of memory to copy in
             * bytes.
             *
             * @brief Additional Considerations:
             * - p_src must have vk::buffer_usage::transfer_src_bit set
             * - p_dst must have vk::buffer_usage::transfer_dst_bit set.
             * - p_size_bytes must be less or equals to size of both source and
             * destination buffers.
             * - Must be called between '.begin()' and '.end()' of a command
             * buffer.
             * - If when you plan to using p_dst after performing this
             * operation, you need to make sure to perform a buffer memory
             * barrier to ensure the copy is finished beforehand.
             *
             * [CPU-Visible Staging (buffer)]        [Dst: GPU-local Buffer]
             * +-------------------------+         +------------------------+
             * | [Data Segment]          |         |    [Identical Copy]    |
             * | [Size: size_bytes]      | =====>  |    [Size: size_bytes]  |
             * |                         |         |                        |
             * +-------------------------+         +------------------------+
             *
             * Example Usage:
             *
             * ```C++
             * vk::buffer_stream staging_buffer(logical_device, ...);
             * vk::buffer_stream vertex_buffer(logical_device, ...);
             *
             * vk::command_buffer primary_command(logical_device, ...);
             *
             * primary_command.begin(vk::command_usage::one_time_submit);
             *
             * // Transferring data from staging to GPU-accessible buffer handle
             * const auto size_bytes = vertices.size_bytes();
             * primary_command.copy(staging_buffer, vertex_buffer, size_bytes)
             *
             * primary_command.end();
             * ```
             *
             */
            void copy_buffer(const VkBuffer& p_src,
                             const VkBuffer& p_dst,
                             uint64_t p_size_bytes) {
                VkBufferCopy copy_region{};
                copy_region.size = p_size_bytes;
                vkCmdCopyBuffer(
                  m_command_buffer, p_src, p_dst, 1, &copy_region);
            }

            [[nodiscard]] bool alive() const { return m_command_buffer; }

            /**
             * @brief Primary command buffer calls batch of secondary command
             * buffers.
             *
             * Useful for multithreaded rendering: Recording to different
             * secondary commands with responsibilities of your scenes (UI,
             * Shadow, Geometry, etc).
             *
             * Then being able to link the secondary command buffers to the
             * primary command buffer altogether.
             *
             * @param p_commands are arbitrary secondary command buffers to be
             * executed part of the primary command buffer, if m_command_buffer
             * is a primary command.
             *
             * @brief Additional Considerations:
             * - m_command_buffer must be a primary command buffer.
             * - 'p_commands' MUST be secondary command buffers that have
             * already been closed with the `.end()` API.
             * - Inheritance info used in the secondary `.begin()` MUST match
             * the current state of the primary command buffer.
             *
             *
             * [ PRIMARY COMMAND ]                      [ Secondary Commands ]
             * +--------------------+                   +--------------------+
             * | Begin Command      |                   | [Secondary Cmd A]  |
             * |                    |                   | (Draw UI)          |
             * | Begin RenderPass   | -- [execute] -->  +--------------------+
             * |                    |                   | [Secondary Cmd B] |
             * |    .execute(A, B)  |                   | (Draw Particles)   |
             * +--------------------+                   +--------------------+
             *
             *
             */
            void execute(std::span<const VkCommandBuffer> p_commands) {
                vkCmdExecuteCommands(m_command_buffer,
                                     static_cast<uint32_t>(p_commands.size()),
                                     p_commands.data());
            }

            /**
             * @brief Explicitly API to properly do command buffer cleanup
             */
            void destroy() {
                vkFreeCommandBuffers(
                  m_device, m_command_pool, 1, &m_command_buffer);
                vkDestroyCommandPool(m_device, m_command_pool, nullptr);
            }

            operator VkCommandBuffer() const { return m_command_buffer; }

            operator VkCommandBuffer() { return m_command_buffer; }

        private:
            VkDevice m_device = nullptr;
            uint32_t m_begin_end_count = 0;
            VkCommandPool m_command_pool = nullptr;
            VkCommandBuffer m_command_buffer = nullptr;
        };
    };
};