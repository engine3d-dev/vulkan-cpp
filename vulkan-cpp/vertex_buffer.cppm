module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>

export module vk:vertex_buffer;

export import :types;
export import :utilities;
export import :command_buffer;
export import :buffer_streams;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief vulkan implementation for loading in vertices to a vulkan
         * buffer handle
         *
         * TODO: Example implementation. Should consider implementing your own
         * vertex buffer handle specifically to your needs.
         *
         * If you'd like to use this for getting a head start to loading your
         * vertices, this is there to provide an implementation example
         */
        class vertex_buffer {
        public:
            vertex_buffer() = default;

            vertex_buffer(const VkDevice& p_device,
                          std::span<const vertex_input> p_vertices,
                          const buffer_parameters& p_params)
              : m_device(p_device) {

                // 1. creating staging buffer
                // uint32_t property_flags = memory_property::host_visible_bit |
                // memory_property::host_cached_bit; uint32_t buffer_usage =
                // buffer_usage::transfer_src_bit |
                // buffer_usage::storage_buffer_bit;
                const uint32_t transfer = static_cast<uint32_t>(buffer_usage::transfer_src_bit);
                const uint32_t storage = static_cast<uint32_t>(buffer_usage::storage_buffer_bit);
                uint32_t usage = transfer | storage;
                buffer_parameters staging_buffer_params = {
                    .memory_mask = p_params.memory_mask,
                    .property_flags = static_cast<memory_property>(
                      memory_property::host_visible_bit |
                      memory_property::host_cached_bit),
                    // .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    // .usage = static_cast<buffer_usage>(static_cast<uint32_t>(buffer_usage::transfer_src_bit) | static_cast<uint32_t>(buffer_usage::storage_buffer_bit)),
                    .usage = usage,
                    .debug_name = p_params.debug_name,
                    .vkSetDebugUtilsObjectNameEXT =
                      p_params.vkSetDebugUtilsObjectNameEXT
                };
                buffer_stream staging_buffer(
                  m_device, p_vertices.size_bytes(), staging_buffer_params);
                staging_buffer.transfer(p_vertices);

                // 3.) Now creating our actual vertex buffer handler
                // buffer_parameters vertex_params = {
                //     .physical_memory_properties =
                //       p_params.phsyical_memory_properties,
                //     .experiment = p_params.experiment,
                //     .memory_mask = p_params.mask,
                //     .property_flags = memory_property::device_local_bit,
                //     .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                //              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                // };
                // m_vertex_handler = buffer_stream(m_device,
                // p_vertices.size_bytes(), vertex_params);
                m_vertex_handler =
                  buffer_stream(m_device, p_vertices.size_bytes(), p_params);

                // 4. Copy data from staging buffer to the actual vertex buffer
                // itself! buffer_copy_info info = { .src = staging_buffer,
                //                         .dst = m_vertex_handler };
                // copy(m_device, info, m_size_bytes);

                // 1. Retrieve the first queue
                // TODO: Use vk::device_queue for this
                VkQueue temp_graphics_queue = nullptr;
                uint32_t queue_family_index = 0;
                uint32_t queue_index = 0;
                vkGetDeviceQueue(p_device,
                                 queue_family_index,
                                 queue_index,
                                 &temp_graphics_queue);

                // command_buffer_info
                command_params enumerate_command_info = {
                    .levels = command_levels::primary,
                    .queue_index = 0,
                };
                command_buffer copy_command_buffer(p_device,
                                                   enumerate_command_info);

                copy_command_buffer.begin(command_usage::one_time_submit);
                // VkBufferCopy copy_region{};
                // copy_region.size = (VkDeviceSize)m_size_bytes;
                // vkCmdCopyBuffer(
                //   copy_command_buffer, staging_buffer, m_vertex_handler, 1,
                //   &copy_region);
                copy_command_buffer.copy_buffer(
                  staging_buffer, m_vertex_handler, p_vertices.size_bytes());
                copy_command_buffer.end();
                VkCommandBuffer temp = copy_command_buffer;
                VkSubmitInfo submit_info{};
                submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                submit_info.commandBufferCount = 1;
                submit_info.pCommandBuffers = &temp;

                vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
                vkQueueWaitIdle(temp_graphics_queue);

                // vkFreeCommandBuffers(, command_pool, 1, &copy_cmd_buffer);
                // vkDestroyCommandPool(driver, command_pool, nullptr);
                copy_command_buffer.destroy();

                // 5. cleanup staging buffer -- no longer used
                staging_buffer.destroy();
            }

            [[nodiscard]] bool alive() const { return m_vertex_handler; }

            // TODO: Probably handle flushing in vk::buffer_stream to give
            // support for this...?
            // void write(std::span<const vertex_input> p_vertices) {}

            operator VkBuffer() const { return m_vertex_handler; }

            operator VkBuffer() { return m_vertex_handler; }

            void destroy() { m_vertex_handler.destroy(); }

        private:
            VkDevice m_device = nullptr;
            buffer_stream m_vertex_handler;
        };
    };
};