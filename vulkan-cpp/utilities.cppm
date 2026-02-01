module;

#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>
#include <span>
#include <source_location>
#include <vector>

export module vk:utilities;

export import :types;

export namespace vk {
    inline namespace v1 {

    void vk_check(const VkResult& p_result,
                  const std::string& p_name,
                  const std::source_location& p_source={}) {
        if (p_result != VK_SUCCESS) {
            std::println(
              "File {} on line {} failed VkResult check",
              std::filesystem::relative(p_source.file_name()).string(),
              p_source.line());
            std::println("Current Function Location = {}",
                         p_source.function_name());
            std::println("{} VkResult returned: {}", p_name, (int)p_result);
        }
    }

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(
      const VkPhysicalDevice& p_physical) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
          p_physical, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(
          queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(
          p_physical, &queue_family_count, queue_family_properties.data());

        return queue_family_properties;
    }

    VkFormat select_compatible_formats(
      const VkPhysicalDevice& p_physical,
      std::span<const vk::format> p_format_selection,
      VkImageTiling p_tiling,
      VkFormatFeatureFlags p_feature_flag) {
        VkFormat format = VK_FORMAT_UNDEFINED;

        for (size_t i = 0; i < p_format_selection.size(); i++) {
            VkFormat current_format = static_cast<VkFormat>(p_format_selection[i]);
            VkFormatProperties format_properties;
            vkGetPhysicalDeviceFormatProperties(
              p_physical, current_format, &format_properties);

            if (p_tiling == VK_IMAGE_TILING_LINEAR) {
                if (format_properties.linearTilingFeatures & p_feature_flag) {
                    format = current_format;
                }
            }
            else if (p_tiling == VK_IMAGE_TILING_OPTIMAL and
                     format_properties.optimalTilingFeatures & p_feature_flag) {
                format = current_format;
            }
        }

        return format;
    }

    VkFormat select_depth_format(const VkPhysicalDevice& p_physical,
                                 std::span<const vk::format> p_format_selection) {

        VkFormat format = select_compatible_formats(
          p_physical,
          p_format_selection,
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return format;
    }

    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical,
                                        uint32_t p_type_filter,
                                        VkMemoryPropertyFlags p_property_flag) {
        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(p_physical, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((p_type_filter & (1 << i)) and
                (mem_props.memoryTypes[i].propertyFlags & p_property_flag) ==
                  p_property_flag) {
                return i;
            }
        }

        return -1;
    }

    // VkMemoryPropertyFlags to_memory_property_flags(memory_property p_flag) {
    //     VkMemoryPropertyFlags flags = 0;
    //     if (p_flag & memory_property::device_local_bit) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    //     }
    //     if (p_flag & memory_property::host_visible_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    //     }
    //     if (p_flag & memory_property::host_coherent_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    //     }
    //     if (p_flag & memory_property::host_cached_bit) {
    //         flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    //     }
    //     if (p_flag & memory_property::lazily_allocated_bit) {
    //         flags |= VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;
    //     }
    //     if (p_flag & memory_property::device_protected_bit) {
    //         flags |= VK_MEMORY_PROPERTY_PROTECTED_BIT;
    //     }
    //     if (p_flag & memory_property::device_coherent_bit_amd) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;
    //     }
    //     if (p_flag & memory_property::device_uncached_bit_amd) {
    //         flags |= VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD;
    //     }
    //     if (p_flag & memory_property::rdma_capable_bit_nv) {
    //         flags |= VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV;
    //     }

    //     return flags;
    // }

    surface_params enumerate_surface(const VkPhysicalDevice& p_physical,
                                          const VkSurfaceKHR& p_surface) {
        surface_params enumerate_surface_properties{};
        vk_check(
          vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            p_physical, p_surface, &enumerate_surface_properties.capabilities),
          "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

        uint32_t format_count = 0;
        std::vector<VkSurfaceFormatKHR> formats;
        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
                   p_physical, p_surface, &format_count, nullptr),
                 "vkGetPhysicalDeviceSurfaceFormatsKHR");

        formats.resize(format_count);

        vk_check(vkGetPhysicalDeviceSurfaceFormatsKHR(
                   p_physical, p_surface, &format_count, formats.data()),
                 "vkGetPhysicalDeviceSurfaceFormatsKHR");

        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                enumerate_surface_properties.format = format;
            }
        }

        if(enumerate_surface_properties.format.format == VK_FORMAT_UNDEFINED) {
            enumerate_surface_properties.format = formats[0];
        }

        return enumerate_surface_properties;
    }

    uint32_t surface_image_size(
      const VkSurfaceCapabilitiesKHR& p_capabilities) {
        uint32_t requested_images = p_capabilities.minImageCount + 1;

        uint32_t final_image_count = 0;

        if ((p_capabilities.maxImageCount > 0) and
            (requested_images > p_capabilities.maxImageCount)) {
            final_image_count = p_capabilities.maxImageCount;
        }
        else {
            final_image_count = requested_images;
        }

        return final_image_count;
    }

    VkSampler create_sampler(const VkDevice& p_device,
                             const filter_range& p_range,
                             VkSamplerAddressMode p_address_mode) {
        VkSamplerCreateInfo sampler_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = p_range.min,
            .minFilter = p_range.max,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = p_address_mode,
            .addressModeV = p_address_mode,
            .addressModeW = p_address_mode,
            .mipLodBias = 0.0f,
            .anisotropyEnable = false,
            .maxAnisotropy = 1,
            .compareEnable = false,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = false
        };

        VkSampler sampler = nullptr;
        VkResult res =
          vkCreateSampler(p_device, &sampler_info, nullptr, &sampler);
        vk_check(res, "vkCreateSampler");
        return sampler;
    }

    VkSemaphore create_semaphore(const VkDevice& p_device) {
        // creating semaphores
        VkSemaphoreCreateInfo semaphore_ci = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        VkSemaphore semaphore;
        vk_check(
          vkCreateSemaphore(p_device, &semaphore_ci, nullptr, &semaphore),
          "vkCreateSemaphore");
        return semaphore;
    }

    uint32_t image_memory_requirements(const VkPhysicalDevice& p_physical,
                                       const VkDevice& p_device,
                                       const VkImage& p_image,
                                       memory_property p_property) {
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(p_device, p_image, &memory_requirements);

        uint32_t type_filter = memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag = static_cast<VkMemoryPropertyFlags>(p_property);

        VkPhysicalDeviceMemoryProperties mem_props;
        vkGetPhysicalDeviceMemoryProperties(p_physical, &mem_props);

        for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) and
                (mem_props.memoryTypes[i].propertyFlags & property_flag) ==
                  property_flag) {
                return i;
            }
        }

        return -1;
    }

    VkVertexInputRate to_input_rate(input_rate p_input_rate) {
        switch (p_input_rate) {
            case input_rate::vertex:
                return VK_VERTEX_INPUT_RATE_VERTEX;
            case input_rate::instance:
                return VK_VERTEX_INPUT_RATE_INSTANCE;
            default:
                return VK_VERTEX_INPUT_RATE_MAX_ENUM;
        }
    }

    bool has_depth_specified(image_layout p_layout) {
        if (p_layout == image_layout::depth_stencil_optimal) {
            return true;
        }

        if (p_layout == image_layout::depth_stencil_read_only_optimal) {
            return true;
        }
        return false;
    }

    int bytes_per_texture_format(VkFormat p_format) {
        switch (p_format) {
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R8_UNORM:
                return 1;
            case VK_FORMAT_R16_SFLOAT:
                return 2;
            case VK_FORMAT_R16G16_SFLOAT:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_UNORM:
                return 4;
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return 4 * sizeof(uint16_t);
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 4 * sizeof(float);
            case VK_FORMAT_R8G8B8A8_SRGB:
                return 4 * sizeof(uint8_t);
            default:
                return 0;
        }

        return 0;
    }

    bool has_stencil_attachment(VkFormat p_format) {
        return ((p_format == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
                (p_format == VK_FORMAT_D24_UNORM_S8_UINT));
    }

    uint32_t select_memory_requirements(
      VkPhysicalDeviceMemoryProperties p_physical_memory_props,
      VkMemoryRequirements p_memory_requirements,
      memory_property p_property) {
        uint32_t memory_bits = p_memory_requirements.memoryTypeBits;
        VkMemoryPropertyFlags property_flag =
          static_cast<VkMemoryPropertyFlags>(p_property);

        for (uint32_t i = 0; i < p_physical_memory_props.memoryTypeCount; i++) {
            if ((memory_bits & (1 << i)) and
                (p_physical_memory_props.memoryTypes[i].propertyFlags &
                 property_flag) == property_flag) {
                return i;
            }
        }

        return -1;
    }

}; // end of v1 namespace
};