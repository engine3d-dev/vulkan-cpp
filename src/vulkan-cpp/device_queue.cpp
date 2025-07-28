#include <vulkan-cpp/device_queue.hpp>
#include <vulkan-cpp/utilities.hpp>


namespace vk {
    device_queue::device_queue(const VkDevice& p_device, const queue_enumeration& p_config) {
        vkGetDeviceQueue(p_device, p_config.family, p_config.index, &m_queue_handler);
    }
};