#include <vulkan-cpp/uniform_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    uniform_buffer::uniform_buffer(const VkDevice& p_device, const uniform_buffer_info& p_uniform_info) : m_device(p_device), m_size_bytes(p_uniform_info.size_bytes) {
        uint32_t property_flags = memory_property::host_visible_bit | memory_property::host_coherent_bit;
        buffer_settings uniform_info = {
            .device_size = m_size_bytes,
            .physical_memory_properties = p_uniform_info.phsyical_memory_properties,
            .property_flags = (memory_property)property_flags,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        };
        m_uniform_handle = buffer_handler(m_device, uniform_info);
    }

    void uniform_buffer::update(const void* p_data) {
        // void* mapped = nullptr;
        // vk_check(vkMapMemory(m_device,
        //                      m_uniform_buffer.device_memory,
        //                      0,
        //                      m_size_bytes,
        //                      0,
        //                      &mapped),
        //          "vkMapMemory");
        // memcpy(mapped, p_data, m_size_bytes);
        // vkUnmapMemory(m_device, m_uniform_buffer.device_memory);
        m_uniform_handle.write(p_data, m_size_bytes);
    }

    void uniform_buffer::destroy() {
        // free_buffer(m_device, m_uniform_buffer);
        m_uniform_handle.destroy();
    }

};