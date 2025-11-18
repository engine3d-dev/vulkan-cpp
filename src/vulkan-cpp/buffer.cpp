#include <vulkan-cpp/buffer.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <print>

namespace vk {

    buffer_handler::buffer_handler(const VkDevice& p_device,
                                   const buffer_settings& p_settings)
      : m_device(p_device) {
        m_allocation_size = p_settings.device_size;

        VkBufferCreateInfo buffer_ci = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = m_allocation_size, // size in bytes
            .usage = p_settings.usage,
            .sharingMode = p_settings.share_mode,
        };

        vk_check(vkCreateBuffer(p_device, &buffer_ci, nullptr, &m_handle),
                 "vkCreateBuffer");

        // 2. retrieving buffer memory requirements
        VkMemoryRequirements memory_requirements = {};
        vkGetBufferMemoryRequirements(p_device, m_handle, &memory_requirements);

        // 3. selects the required memory requirements for this specific buffer
        // allocations
        uint32_t memory_index =
          select_memory_requirements(p_settings.physical_memory_properties,
                                     memory_requirements,
                                     p_settings.property_flags);

        // 4. allocatring the necessary memory based on memory requirements for
        // the buffer handles
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_index
        };

#if _DEBUG
        // 1. Define the structure
        VkDebugUtilsObjectNameInfoEXT debug_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .pNext = nullptr,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = (uint64_t)m_handle, // specify vulkan to what object handle this is
            .pObjectName = p_settings.debug_name // specify what type of buffer this is
        };

        if(p_settings.vkSetDebugUtilsObjectNameEXT != nullptr) {
            // vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
            p_settings.vkSetDebugUtilsObjectNameEXT(m_device, &debug_info);
        }
#endif
        vk_check(vkAllocateMemory(
                   p_device, &memory_alloc_info, nullptr, &m_device_memory),
                 "vkAllocateMemory");

        // 5. bind memory resource of this buffer handle
        vk_check(vkBindBufferMemory(p_device, m_handle, m_device_memory, 0),
                 "vkBindBufferMemory");
    }

    void buffer_handler::destroy() {
        if (m_handle != nullptr) {
            vkDestroyBuffer(m_device, m_handle, nullptr);
        }

        if (m_device_memory != nullptr) {
            vkFreeMemory(m_device, m_device_memory, nullptr);
        }
    }

};