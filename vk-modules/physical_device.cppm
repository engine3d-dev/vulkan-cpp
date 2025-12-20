module;

#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

export module vk:physical_device;

export import :utilities;


export namespace vk {
    inline namespace v1 {
        class physical_device {
        public:
            physical_device() = default;

            physical_device(const VkInstance& p_instance, const physical_enumeration& p_physical_enumeration) {
                m_physical_device = enumerate_physical_devices(p_instance, p_physical_enumeration.device_type);

                if (m_physical_device == nullptr) {
                    throw std::runtime_error("m_physical_device was nullptr!!!");
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
            VkPhysicalDevice m_physical_device = nullptr;
            std::vector<VkQueueFamilyProperties> m_queue_family_properties;
            queue_indices m_queue_family_indices{};
        };
    };
};