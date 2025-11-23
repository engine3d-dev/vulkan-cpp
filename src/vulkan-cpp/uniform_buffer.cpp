#include <vulkan-cpp/uniform_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    uniform_buffer::uniform_buffer(const VkDevice& p_device,
                                   const uniform_buffer_info& p_uniform_info)
      : m_device(p_device)
      , m_size_bytes(p_uniform_info.size_bytes) {
        uint32_t property_flags = memory_property::host_visible_bit |
                                  memory_property::host_coherent_bit;
        buffer_parameters uniform_info = {
            .device_size = m_size_bytes,
            .physical_memory_properties =
              p_uniform_info.phsyical_memory_properties,
            .property_flags = (memory_property)property_flags,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .debug_name = p_uniform_info.debug_name.c_str(),
            .vkSetDebugUtilsObjectNameEXT = p_uniform_info.vkSetDebugUtilsObjectNameEXT
        };
        m_uniform_handle = buffer_stream(m_device, uniform_info);
    }

    void uniform_buffer::update(const void* p_data) {
        m_uniform_handle.write(p_data, m_size_bytes);
    }

    void uniform_buffer::destroy() {
        m_uniform_handle.destroy();
    }

};