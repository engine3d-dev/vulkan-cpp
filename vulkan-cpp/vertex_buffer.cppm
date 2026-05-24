module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>

export module vk:vertex_buffer;

export import :types;
export import :utilities;
export import :command_buffer;
export import :buffer;

export namespace vk {
    inline namespace v6 {
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

                // Staging buffer
                const uint32_t transfer =
                  static_cast<uint32_t>(buffer_usage::transfer_src_bit);
                const uint32_t storage =
                  static_cast<uint32_t>(buffer_usage::storage_buffer_bit);
                uint32_t usage = transfer | storage;
                buffer_parameters staging_buffer_params = {
                    .memory_mask = p_params.memory_mask,
                    .property_flags = static_cast<memory_property>(
                      memory_property::host_visible_bit |
                      memory_property::host_cached_bit),
                    .usage =
                      static_cast<uint32_t>(buffer_usage::transfer_src_bit) |
                      static_cast<uint32_t>(buffer_usage::storage_buffer_bit),
                    .debug_name = p_params.debug_name,
                    .vkSetDebugUtilsObjectNameEXT =
                      p_params.vkSetDebugUtilsObjectNameEXT
                };
                buffer staging_buffer(
                  m_device, p_vertices.size_bytes(), staging_buffer_params);
                staging_buffer.transfer(p_vertices);

                // Creating vertex buffer handle
                m_vertex_handler =
                  buffer(m_device, p_vertices.size_bytes(), p_params);

                // Retrieve the first queue
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

                copy_command_buffer.destruct();

                // Cleaning up staging buffer after using it
                staging_buffer.destruct();
            }

            ~vertex_buffer() = default;

            void construct(const VkDevice& p_device,
                           std::span<const vertex_input> p_vertices,
                           const buffer_parameters& p_params) {

                // Can be used to invalidate 3D mesh vertices
                // Staging buffer
                const uint32_t transfer =
                  static_cast<uint32_t>(buffer_usage::transfer_src_bit);
                const uint32_t storage =
                  static_cast<uint32_t>(buffer_usage::storage_buffer_bit);
                uint32_t usage = transfer | storage;
                buffer_parameters staging_buffer_params = {
                    .memory_mask = p_params.memory_mask,
                    .property_flags = static_cast<memory_property>(
                      memory_property::host_visible_bit |
                      memory_property::host_cached_bit),
                    .usage =
                      static_cast<uint32_t>(buffer_usage::transfer_src_bit) |
                      static_cast<uint32_t>(buffer_usage::storage_buffer_bit),
                    .debug_name = p_params.debug_name,
                    .vkSetDebugUtilsObjectNameEXT =
                      p_params.vkSetDebugUtilsObjectNameEXT
                };
                buffer staging_buffer(
                  m_device, p_vertices.size_bytes(), staging_buffer_params);
                staging_buffer.transfer(p_vertices);

                // Creating vertex buffer handle
                // m_vertex_handler =
                //   buffer(m_device, p_vertices.size_bytes(), p_params);
                m_vertex_handler.construct(p_vertices.size_bytes(), p_params);

                // 1. Retrieve the first queue
                // TODO: Use vk::device_queue for this
                VkQueue temp_graphics_queue = nullptr;
                uint32_t queue_family_index = 0;
                uint32_t queue_index = 0;
                vkGetDeviceQueue(p_device,
                                 queue_family_index,
                                 queue_index,
                                 &temp_graphics_queue);

                // creating command to copy data to GPU for available accessing
                command_params enumerate_command_info = {
                    .levels = command_levels::primary,
                    .queue_index = 0,
                };
                command_buffer copy_command_buffer(p_device,
                                                   enumerate_command_info);

                copy_command_buffer.begin(command_usage::one_time_submit);

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

                copy_command_buffer.destruct();

                // cleanup staging buffer -- no longer used
                staging_buffer.destruct();
            }

            void destruct() { m_vertex_handler.destruct(); }

            [[nodiscard]] bool alive() const { return m_vertex_handler; }

            // TODO: Probably handle flushing in vk::buffer to give
            // support for this...?
            void transfer(std::span<const vertex_input> p_vertices) {
                m_vertex_handler.transfer(p_vertices);
            }

            operator VkBuffer() const { return m_vertex_handler; }

            operator VkBuffer() { return m_vertex_handler; }

        private:
            VkDevice m_device = nullptr;
            buffer m_vertex_handler;
        };
    };
};