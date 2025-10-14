#include <vulkan-cpp/device.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {

    device::device(const VkPhysicalDevice& p_physical,
                   const device_enumeration& p_config) {

        VkDeviceQueueCreateInfo device_queue_ci = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = p_config.queue_family_index,
            .queueCount =
              static_cast<uint32_t>(p_config.queue_priorities.size()),
            .pQueuePriorities = p_config.queue_priorities.data(),
        };

        VkDeviceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &device_queue_ci,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount =
              static_cast<uint32_t>(p_config.extensions.size()),
            .ppEnabledExtensionNames = p_config.extensions.data(),
        };

        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceFeatures(p_physical, &features);
        features.robustBufferAccess = false;
        create_info.pEnabledFeatures = &features;

        vk_check(vkCreateDevice(p_physical, &create_info, nullptr, &m_device),
                 "vkCreateDevice");
    }

    void device::wait() {
        vkDeviceWaitIdle(m_device);
    }

    void device::destroy() {

        if (m_device != nullptr) {
            vkDestroyDevice(m_device, nullptr);
        }
    }
};