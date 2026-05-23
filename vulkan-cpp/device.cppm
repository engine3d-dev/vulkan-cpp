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
        public:
            device() = delete;
            device(const VkPhysicalDevice& p_physical,
                   const device_params& p_params, const VkPhysicalDeviceFeatures& p_features) {

                construct(p_physical, p_params, p_features);
            }

            ~device() {
                wait();
                destruct();
            }

            device(const device&) = default;
            device(device&&) = delete;
            device& operator=(const device&) = default;
            device& operator=(device&&) = delete;


            void construct(const VkPhysicalDevice p_physical, const device_params& p_params, const VkPhysicalDeviceFeatures& p_features) {
                VkDeviceQueueCreateInfo device_queue_ci = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = p_params.queue_family_index,
                    .queueCount =
                      static_cast<uint32_t>(p_params.queue_priorities.size()),
                    .pQueuePriorities = p_params.queue_priorities.data(),
                };

                VkDeviceCreateInfo create_info = {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = p_params.features,
                    .flags = 0,
                    .queueCreateInfoCount = 1,
                    .pQueueCreateInfos = &device_queue_ci,
                    .enabledLayerCount = 0,
                    .ppEnabledLayerNames = nullptr,
                    .enabledExtensionCount =
                      static_cast<uint32_t>(p_params.extensions.size()),
                    .ppEnabledExtensionNames = p_params.extensions.data(),
                };

                // VkPhysicalDeviceFeatures features;
                // vkGetPhysicalDeviceFeatures(p_physical, &features);
                // features.robustBufferAccess = false;
                create_info.pEnabledFeatures = &p_features;

                vk_check(
                  vkCreateDevice(p_physical, &create_info, nullptr, &m_device),
                  "vkCreateDevice");
            }

            void destruct() {
                if (m_device != nullptr) {
                    vkDestroyDevice(m_device, nullptr);
                }
            }

            void wait() { vkDeviceWaitIdle(m_device); }

            operator VkDevice() const { return m_device; }

            operator VkDevice() { return m_device; }

        private:
            VkDevice m_device = nullptr;
        };

    };
};