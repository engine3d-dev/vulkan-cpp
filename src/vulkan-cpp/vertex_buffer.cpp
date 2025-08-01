#include <vulkan-cpp/vertex_buffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    vertex_buffer::vertex_buffer(const VkDevice& p_device, const vertex_buffer_info& p_vertex_info) : m_device(p_device) {
        m_size = p_vertex_info.vertices.size();
        m_size_bytes = p_vertex_info.vertices.size_bytes();

        VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

        // 1. creating staging buffer
        uint32_t property_flags = memory_property::host_visible_bit | memory_property::host_cached_bit;
        buffer_configuration config = {
            .device_size = m_size_bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            // .property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
            .property_flags = (memory_property)property_flags,
            .physical = p_vertex_info.physical_handle
        };
        buffer_handle staging_buffer = create_buffer(m_device, config);
        write(m_device, staging_buffer, p_vertex_info.vertices);

        // 3.) Now creating our actual vertex buffer handler
        buffer_configuration vertex_info = {
            .device_size = m_size_bytes,
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            // .property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .property_flags = memory_property::device_local_bit,
            .physical = p_vertex_info.physical_handle
        };
        m_vertex_buffer = create_buffer(m_device, vertex_info);

        // 4. Copy data from staging buffer to the actual vertex buffer itself!
        buffer_copy_info info = {
            .src = staging_buffer,
            .dst = m_vertex_buffer
        };
        copy(m_device, info, m_size_bytes);

        // 5. cleanup staging buffer -- no longer used
        free_buffer(m_device, staging_buffer);
    }

    void vertex_buffer::bind(const VkCommandBuffer& p_current) {
        std::array<VkBuffer, 1> handlers = { m_vertex_buffer.handle };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(p_current, 0, 1, handlers.data(), offsets);
    }

    void vertex_buffer::destroy() {
        free_buffer(m_device, m_vertex_buffer);
    }

};