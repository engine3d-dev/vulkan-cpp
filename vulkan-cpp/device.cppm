module;

#include <vulkan/vulkan.h>

export module vk:device;

export import :types;
export import :utilities;


export namespace vk {
    inline namespace v1 {
        /**
         * @name device
         * @brief represents a vulkan logical device
         */
        class device {
            struct queue_family {
                VkQueue graphics = nullptr;
                VkQueue compute = nullptr;
                VkQueue transfer = nullptr;
            };

        public:
            device(const VkPhysicalDevice& p_physical, const device_params& p_config) {
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

            [[nodiscard]] queue_family family() const { return m_queue_family; }

            void destroy() {
                if (m_device != nullptr) {
                    vkDestroyDevice(m_device, nullptr);
                }
            }

            void wait() {
                vkDeviceWaitIdle(m_device);
            }

            operator VkDevice() const { return m_device; }

            operator VkDevice() { return m_device; }

        private:
            VkDevice m_device = nullptr;
            queue_family m_queue_family{};
        };

    };
};