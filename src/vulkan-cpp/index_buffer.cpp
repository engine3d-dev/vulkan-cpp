#include <vulkan-cpp/index_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    index_buffer::index_buffer(const VkDevice& p_device,
                               const index_params& p_info)
      : m_device(p_device) {
        m_indices_count = p_info.indices.size();

        uint32_t property_flags =
          memory_property::host_visible_bit | memory_property::host_cached_bit;
        buffer_parameters index_params = {
            .device_size = p_info.indices.size_bytes(),
            .physical_memory_properties = p_info.phsyical_memory_properties,
            .property_flags = (memory_property)property_flags,
            .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.debug_name = p_info.debug_name.c_str(),
            .vkSetDebugUtilsObjectNameEXT = p_info.vkSetDebugUtilsObjectNameEXT
        };

        m_index_buffer = buffer_stream32(m_device, index_params);

        m_index_buffer.write(p_info.indices);
    }

    void index_buffer::bind(const VkCommandBuffer& p_current) {
        vkCmdBindIndexBuffer(
          p_current, m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void index_buffer::destroy() {
        m_index_buffer.destroy();
    }
}