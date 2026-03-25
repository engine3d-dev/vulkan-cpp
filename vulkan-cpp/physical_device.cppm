module;

#include <vulkan/vulkan.h>
#include <vector>
#include <stdexcept>
#include <span>

export module vk:physical_device;

export import :utilities;
export import :types;

export namespace vk {
    inline namespace v1 {
        class physical_device {
        public:
            physical_device() = default;

            physical_device(
              const VkInstance& p_instance,
              const physical_enumeration& p_physical_enumeration) {
                m_physical_device = enumerate_physical_devices(
                  p_instance, p_physical_enumeration.device_type);

                if (m_physical_device == nullptr) {
                    return;
                }

                m_queue_family_properties =
                  enumerate_queue_family_properties(m_physical_device);

                // This makes sure that we get the graphics, compute, and
                // transfer queue indices from the physical queue family
                // assigned
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
            [[nodiscard]] VkPhysicalDeviceMemoryProperties memory_properties()
              const {
                VkPhysicalDeviceMemoryProperties physical_memory_properties;
                vkGetPhysicalDeviceMemoryProperties(
                  m_physical_device, &physical_memory_properties);
                return physical_memory_properties;
            }

            [[nodiscard]] uint32_t memory_properties(
              memory_property p_property_required) const {
                allocation_params return_params = {};

                VkPhysicalDeviceMemoryProperties memory_properties;
                vkGetPhysicalDeviceMemoryProperties(m_physical_device,
                                                    &memory_properties);

                uint32_t mask = 0;
                for (uint32_t i = 0; i < memory_properties.memoryTypeCount;
                     i++) {
                    auto type_flags =
                      memory_properties.memoryTypes[i].propertyFlags;

                    if ((type_flags & p_property_required) ==
                        p_property_required) {
                        mask |= ((1 << i));
                    }
                }
                return mask;
            }

            /**
             * @brief Requests the depth format from the physical device
             *
             * @param p_format_supported are the arbitrary depth format
             * selections to select if either are available.
             * @return the format that is the one of the requested depth
             * formats.
             */
            [[nodiscard]] VkFormat request_depth_format(
              std::span<const format> p_format_supported) {

                return request_compatible_formats(
                  p_format_supported,
                  VK_IMAGE_TILING_OPTIMAL,
                  VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
            }

            /**
             * @brief Requests a format and select an arbitrary format if those
             * are available to select those
             *
             * @param p_tiling is selecting arrangements of the data format
             * @param p_feature_flag is the bitmask selection for the image
             * format
             */
            [[nodiscard]] VkFormat request_format(
              std::span<const format> p_format_supported,
              uint32_t p_tiling,
              uint32_t p_feature_flag) {
                return request_compatible_formats(
                  p_format_supported,
                  static_cast<VkImageTiling>(p_tiling),
                  static_cast<VkFormatFeatureFlags>(p_feature_flag));
            }

            [[nodiscard]] surface_params request_surface(
              const VkSurfaceKHR& p_surface,
              uint32_t p_format = VK_FORMAT_B8G8R8A8_SRGB,
              uint32_t p_colorspace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surface_params enumerate_surface_properties{};
                vk_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                           m_physical_device,
                           p_surface,
                           &enumerate_surface_properties.capabilities),
                         "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

                uint32_t format_count = 0;
                std::vector<VkSurfaceFormatKHR> formats;
                vk_check(
                  vkGetPhysicalDeviceSurfaceFormatsKHR(
                    m_physical_device, p_surface, &format_count, nullptr),
                  "vkGetPhysicalDeviceSurfaceFormatsKHR");

                formats.resize(format_count);

                vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device,
                                                              p_surface,
                                                              &format_count,
                                                              formats.data()),
                         "vkGetPhysicalDeviceSurfaceFormatsKHR");

                // These are format and colorspaces selected by the user.
                VkFormat selected_format = static_cast<VkFormat>(p_format);
                VkColorSpaceKHR color_space =
                  static_cast<VkColorSpaceKHR>(p_colorspace);

                for (const auto& format : formats) {
                    if (format.format == selected_format &&
                        format.colorSpace == color_space) {
                        enumerate_surface_properties.format = format;
                    }
                }

                if (enumerate_surface_properties.format.format ==
                    VK_FORMAT_UNDEFINED) {
                    enumerate_surface_properties.format = formats[0];
                }

                return enumerate_surface_properties;
            }

            operator VkPhysicalDevice() { return m_physical_device; }

            operator VkPhysicalDevice() const { return m_physical_device; }

        private:
            VkFormat request_compatible_formats(
              std::span<const format> p_format_supported,
              VkImageTiling p_tiling,
              VkFormatFeatureFlags p_feature_flag) {
                VkFormat format = VK_FORMAT_UNDEFINED;

                for (uint32_t i = 0; i < p_format_supported.size(); i++) {
                    VkFormat current =
                      static_cast<VkFormat>(p_format_supported[i]);
                    VkFormatProperties format_properties;
                    vkGetPhysicalDeviceFormatProperties(
                      m_physical_device, current, &format_properties);

                    switch (p_tiling) {
                        case VK_IMAGE_TILING_LINEAR:
                            if (format_properties.linearTilingFeatures &
                                p_feature_flag) {
                                format = current;
                            }
                            break;
                        case VK_IMAGE_TILING_OPTIMAL:
                            if (format_properties.optimalTilingFeatures &
                                p_feature_flag) {
                                format = current;
                            }
                            break;
                        default:
                            break;
                    }
                }

                return format;
            }

            VkPhysicalDevice enumerate_physical_devices(
              const VkInstance& p_instance,
              const physical_gpu& p_physical_device_type) {
                uint32_t device_count = 0;
                vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

                if (device_count == 0) {
                    return nullptr;
                }

                // TODO: Turn this into map<VkDescriptorDeviceType,
                // VkPhysicalDevice>
                std::vector<VkPhysicalDevice> physical_devices(device_count);
                vkEnumeratePhysicalDevices(
                  p_instance, &device_count, physical_devices.data());
                VkPhysicalDevice physical_device = nullptr;

                for (const auto& device : physical_devices) {
                    VkPhysicalDeviceProperties device_properties;
                    vkGetPhysicalDeviceProperties(device, &device_properties);

                    if (device_properties.deviceType ==
                        static_cast<VkPhysicalDeviceType>(
                          p_physical_device_type)) {
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