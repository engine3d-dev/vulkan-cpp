module;

#include <vulkan/vulkan.h>
#include <span>

export module vk:index_buffer;

export import :types;
export import :utilities;
export import :command_buffer;
export import :buffer_streams32;

export namespace vk {
    inline namespace v1 {

        /**
         * @brief Test implementation for index buffers
         *
         * This implementatino is meant to be used for an example.
         *
         * Though this can be used into your own code if you would like.
         */
        class index_buffer {
        public:
            index_buffer() = default;
            index_buffer(const VkDevice& p_device,
                         std::span<const uint32_t> p_indices,
                         const buffer_parameters& p_params)
              : m_device(p_device) {

                // buffer_parameters index_params = {
                //     .physical_memory_properties =
                //       p_info.phsyical_memory_properties,
                //     .property_flags = static_cast<memory_property>(
                //       memory_property::host_visible_bit |
                //       memory_property::host_cached_bit),
                //     .usage = static_cast<VkBufferUsageFlags>(
                //       buffer_usage::index_buffer_bit),
                //     .debug_name = p_info.debug_name.c_str(),
                //     .vkSetDebugUtilsObjectNameEXT =
                //       p_info.vkSetDebugUtilsObjectNameEXT
                // };

                m_index_buffer =
                  buffer_stream32(m_device, p_indices.size_bytes(), p_params);

                m_index_buffer.write(p_indices);
            }

            [[nodiscard]] bool alive() const { return m_index_buffer; }

            operator VkBuffer() const { return m_index_buffer; }

            operator VkBuffer() { return m_index_buffer; }

            void destroy() { m_index_buffer.destroy(); }

        private:
            VkDevice m_device = nullptr;
            buffer_stream32 m_index_buffer{};
        };
    };
};