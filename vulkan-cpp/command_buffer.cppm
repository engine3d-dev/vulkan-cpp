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
         * @brief vk::command_buffer represents the wrapper around VkCommandBuffer
         * 
         * Defines the command buffer and provides API's to directly interact with how
         * the command buffer may be utilized based on the needs of the application that uses it
         * 
        */
        class command_buffer {
        public:
            command_buffer() = default;
            command_buffer(const VkDevice& p_device, const command_params& p_enumerate_command_info) : m_device(p_device) {
                VkCommandPoolCreateInfo pool_ci = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .pNext = nullptr,
                // .flags =
                // (VkCommandPoolCreateFlags)p_enumerate_command_info.pool_flag,
                .flags = static_cast<VkCommandPoolCreateFlags>(p_enumerate_command_info.flags),
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
                static_cast<VkCommandBufferLevel>(p_enumerate_command_info.levels),
                .commandBufferCount = 1
            };

            vk_check(vkAllocateCommandBuffers(
                    m_device, &command_buffer_alloc_info, &m_command_buffer),
                    "vkAllocateCommandBuffers");
            }

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
            void begin(command_usage p_usage, std::span<const command_inherit_info> p_inherit_info = {}) {
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

            /**
             * @brief ends command buffer recording operation
            */
            void end() {
                m_begin_end_count++;
                vkEndCommandBuffer(m_command_buffer);
            }

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
            void copy_buffer(const VkBuffer& p_src, const VkBuffer& p_dst, uint64_t p_size_bytes) {
                VkBufferCopy copy_region{};
                copy_region.size = p_size_bytes;
                vkCmdCopyBuffer(m_command_buffer, p_src, p_dst, 1, &copy_region);
            }

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
            void execute(std::span<const VkCommandBuffer> p_commands) {
                vkCmdExecuteCommands(m_command_buffer,
                             static_cast<uint32_t>(p_commands.size()),
                             p_commands.data());
            }

            void destroy() {
                vkFreeCommandBuffers(m_device, m_command_pool, 1, &m_command_buffer);
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