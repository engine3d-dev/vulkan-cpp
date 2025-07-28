#include <vulkan-cpp/physical_device.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <stdexcept>
#include <print>

namespace vk {
    physical_device::physical_device(const VkInstance& p_instance, const physical_enumeration& p_physical_enumeration) {
        m_physical_device = enumerate_physical_devices(p_instance, p_physical_enumeration.device_type);

        if(m_physical_device == nullptr) {
            throw std::runtime_error("m_physical_device was nullptr!!!");
        }

        m_queue_family_properties = enumerate_queue_family_properties(m_physical_device);

        // This makes sure that we get the graphics, compute, and transfer queue indices from the physical queue family assigned
        uint32_t queue_index=0;
        for(const auto& queue_family : m_queue_family_properties) {
            if(queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_queue_family_indices.graphics = queue_index;
                break;
            }

            queue_index++;
        }
        queue_index=0;

        for(const auto& queue_family : m_queue_family_properties) {
            if(queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                m_queue_family_indices.compute = queue_index;
            }
            queue_index++;
        }
        queue_index=0;

        for(const auto& queue_family : m_queue_family_properties) {
            if(queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                m_queue_family_indices.transfer = queue_index;
            }
            queue_index++;
        }
        queue_index=0;

    }

    uint32_t physical_device::queue_present_index(const VkSurfaceKHR& p_surface) {
        uint32_t presentation_index=0;
        uint32_t compatible = false;
        uint32_t i = 0;
        for(const auto& queue_family : m_queue_family_properties) {
            vk_check(vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, i, p_surface, &compatible), "vkGetPhysicalDeviceSurfaceSupportKHR");

            if(compatible) {
                presentation_index = i;
            }
        }

        return presentation_index;
    }
};