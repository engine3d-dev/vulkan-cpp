module;

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>

export module vk:physical_device;

export import :utilities;
export import :types;


export namespace vk {
    inline namespace v1 {
        class physical_device {
        public:
            physical_device() = default;

            physical_device(const VkInstance& p_instance, const physical_enumeration& p_physical_enumeration) {
                m_physical_device = enumerate_physical_devices(p_instance, p_physical_enumeration.device_type);

                if (m_physical_device == nullptr) {
                    return;
                }

                m_queue_family_properties =
                enumerate_queue_family_properties(m_physical_device);

                // This makes sure that we get the graphics, compute, and transfer queue
                // indices from the physical queue family assigned
                uint32_t queue_index = 0;
                for (const auto& queue_family : m_queue_family_properties) {
                    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        m_queue_family_indices.graphics = queue_index;
                        break;
                    }

                    queue_index++;
                }
                queue_index = 0;

                for (const auto& queue_family : m_queue_family_properties) {
                    if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
                        m_queue_family_indices.compute = queue_index;
                    }
                    queue_index++;
                }
                queue_index = 0;

                for (const auto& queue_family : m_queue_family_properties) {
                    if (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) {
                        m_queue_family_indices.transfer = queue_index;
                    }
                    queue_index++;
                }
                queue_index = 0;
            }

            //! @return true if physical device is valid
            [[nodiscard]] bool alive() const { return m_physical_device; }

            //! @return queue family indices for graphics, compute, and transfer
            //! operations
            [[nodiscard]] queue_indices family_indices() const {
                return m_queue_family_indices;
            }

            //! @return the presentation index for the presentation queue
            uint32_t queue_present_index(const VkSurfaceKHR& p_surface) {
                uint32_t presentation_index = 0;
                uint32_t compatible = false;
                uint32_t i = 0;
                for (const auto& queue_family : m_queue_family_properties) {
                    vk_check(vkGetPhysicalDeviceSurfaceSupportKHR(
                            m_physical_device, i, p_surface, &compatible),
                            "vkGetPhysicalDeviceSurfaceSupportKHR");

                    if (compatible) {
                        presentation_index = i;
                    }
                }

                return presentation_index;
            }

            //! @return physical device memory requirements
            [[nodiscard]] VkPhysicalDeviceMemoryProperties memory_properties() const {
                VkPhysicalDeviceMemoryProperties physical_memory_properties;
                vkGetPhysicalDeviceMemoryProperties(m_physical_device,
                                                    &physical_memory_properties);
                return physical_memory_properties;
            }

            operator VkPhysicalDevice() { return m_physical_device; }

            operator VkPhysicalDevice() const { return m_physical_device; }


        private:
            VkPhysicalDevice enumerate_physical_devices(
                const VkInstance& p_instance,
                const physical_gpu& p_physical_device_type) {
                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

                if (device_count == 0) {
                    return nullptr;
                }


                // TODO: Turn this into map<VkDescriptorDeviceType, VkPhysicalDevice>
                std::vector<VkPhysicalDevice> physical_devices(device_count);
                vkEnumeratePhysicalDevices(
                p_instance, &device_count, physical_devices.data());
                VkPhysicalDevice physical_device = nullptr;

                for (const auto& device : physical_devices) {
                    VkPhysicalDeviceProperties device_properties;
                    vkGetPhysicalDeviceProperties(device, &device_properties);

                    if (device_properties.deviceType ==
                        static_cast<VkPhysicalDeviceType>(p_physical_device_type)) {
                        physical_device = device;
                    }
                }
                return physical_device;
        }

        private:
            VkPhysicalDevice m_physical_device = nullptr;
            std::vector<VkQueueFamilyProperties> m_queue_family_properties;
            queue_indices m_queue_family_indices{};
        };
    };
};