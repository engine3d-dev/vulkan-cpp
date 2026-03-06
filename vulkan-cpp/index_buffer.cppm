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
        class index_buffer {
        public:
            index_buffer() = default;
            index_buffer(const VkDevice& p_device,
                        const index_params& p_info) : m_device(p_device) {
                m_indices_count = p_info.indices.size();

                buffer_parameters index_params = {
                    .device_size = p_info.indices.size_bytes(),
                    .physical_memory_properties = p_info.phsyical_memory_properties,
                    .property_flags = static_cast<memory_property>(memory_property::host_visible_bit | memory_property::host_cached_bit),
                    .usage = static_cast<VkBufferUsageFlags>(buffer_usage::index_buffer_bit),
                    .debug_name = p_info.debug_name.c_str(),
                    .vkSetDebugUtilsObjectNameEXT = p_info.vkSetDebugUtilsObjectNameEXT
                };

                m_index_buffer = buffer_stream32(m_device, index_params);

                m_index_buffer.write(p_info.indices);
            }

            [[nodiscard]] bool alive() const { return m_index_buffer; }

            [[nodiscard]] uint32_t size() const { return m_indices_count; }

            void bind(const VkCommandBuffer& p_current, uint64_t p_offset = 0) {
                vkCmdBindIndexBuffer(
                    p_current, m_index_buffer, p_offset, VK_INDEX_TYPE_UINT32);
            }

            operator VkBuffer() const { return m_index_buffer; }

            operator VkBuffer() { return m_index_buffer; }

            void destroy() {
                m_index_buffer.destroy();
            }

        private:
            VkDevice m_device = nullptr;
            uint32_t m_indices_count = 0;
            buffer_stream32 m_index_buffer{};
        };
    };
};