#include <vulkan-cpp/index_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    index_buffer::index_buffer(const VkDevice& p_device, const index_buffer_info& p_index_info) : m_device(p_device) {
        m_indices_count = p_index_info.indices.size();

        uint32_t property_flags = memory_property::host_visible_bit | memory_property::host_cached_bit;
        
        buffer_configuration config = {
            .device_size = p_index_info.indices.size_bytes(),
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            // .property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            .property_flags = (memory_property)property_flags,
            .physical = p_index_info.physical_handle
        };

        m_index_buffer = create_buffer(m_device, config);

        write(m_device, m_index_buffer, p_index_info.indices);
    }

    void index_buffer::destroy() {
        free_buffer(m_device, m_index_buffer);
    }
}