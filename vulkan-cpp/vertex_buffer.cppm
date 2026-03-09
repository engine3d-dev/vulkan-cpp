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
         * This implementation automates handle in loading the vertices and its
         * memories for it
         */
        class vertex_buffer {
        public:
            vertex_buffer() = default;
            vertex_buffer(const VkDevice& p_device,
                          std::span<const vertex_input> p_vertices,
                          const vertex_params& p_vertex_info)
              : m_device(p_device) {

                // 1. creating staging buffer
                // uint32_t property_flags = memory_property::host_visible_bit |
                // memory_property::host_cached_bit; uint32_t buffer_usage =
                // buffer_usage::transfer_src_bit |
                // buffer_usage::storage_buffer_bit;

                buffer_parameters staging_buffer_params = {
                    .physical_memory_properties =
                      p_vertex_info.phsyical_memory_properties,
                    // .property_flags = (memory_property)property_flags,
                    .property_flags = static_cast<memory_property>(
                      memory_property::host_visible_bit |
                      memory_property::host_cached_bit),
                    .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                             VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    .debug_name = p_vertex_info.debug_name.c_str(),
                    .vkSetDebugUtilsObjectNameEXT =
                      p_vertex_info.vkSetDebugUtilsObjectNameEXT
                };
                buffer_stream staging_buffer(
                  m_device, p_vertices.size_bytes(), staging_buffer_params);
                staging_buffer.transfer(p_vertices);

                // 3.) Now creating our actual vertex buffer handler
                buffer_parameters vertex_params = {
                    .physical_memory_properties =
                      p_vertex_info.phsyical_memory_properties,
                    .property_flags = memory_property::device_local_bit,
                    .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                };
                m_vertex_handler = buffer_stream(
                  m_device, p_vertices.size_bytes(), vertex_params);

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

            void bind(const VkCommandBuffer& p_current) {
                std::array<VkBuffer, 1> handlers = { m_vertex_handler };
                VkDeviceSize offsets[] = { 0 };
                vkCmdBindVertexBuffers(
                  p_current, 0, 1, handlers.data(), offsets);
            }

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