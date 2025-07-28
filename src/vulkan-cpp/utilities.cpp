#include <vulkan-cpp/utilities.hpp>
#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>

namespace vk {

    void vk_check(const VkResult& p_result,
                  const std::string& p_name,
                  const std::source_location& p_source) {
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

    VkDebugUtilsMessageSeverityFlagsEXT to_debug_message_severity(
      uint32_t p_flag) {
        VkDebugUtilsMessageSeverityFlagsEXT flag;

        if (p_flag & vk::message::verbose) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        }
        else if (p_flag & vk::message::warning) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        }
        else if (p_flag & vk::message::error) {
            flag |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        }

        return flag;
    }

    VkDebugUtilsMessageTypeFlagsEXT to_message_type(uint32_t p_flag) {
        VkDebugUtilsMessageTypeFlagsEXT flag;

        if (p_flag & vk::debug::general) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }
        else if (p_flag & vk::debug::validation) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        }
        else if (p_flag & vk::debug::performance) {
            flag |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        }

        return flag;
    }

    uint32_t vk_api_version(const api_version& p_version) {
        switch (p_version) {
            case api_version::vk_1_2:
                return VK_API_VERSION_1_2;
            case api_version::vk_1_3:
                return VK_API_VERSION_1_3;
        }
    }

    VkPhysicalDeviceType vk_physical_device_type(physical p_physical_type) {
        switch (p_physical_type){
        case physical::integrated:
            return VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
        case physical::discrete:
            return VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        case physical::virtualized:
            return VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU;
        case physical::cpu:
            return VK_PHYSICAL_DEVICE_TYPE_CPU;
        case physical::max_enum:
            return VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
        case physical::other:
            return VK_PHYSICAL_DEVICE_TYPE_OTHER;
        }

        throw std::runtime_error("Invalid physical device!");
    }

    struct physical_device_handler {
        VkPhysicalDevice handler=nullptr;
    };

    VkPhysicalDevice enumerate_physical_devices(const VkInstance& p_instance, const physical& p_physical_device_type) {
        uint32_t device_count=0;
        vkEnumeratePhysicalDevices(p_instance, &device_count, nullptr);

        if(device_count == 0) {
            throw std::runtime_error("device_count is zero!");
        }

        // std::vector<vk::physical_device> hardware_physical_devices(device_count);

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(p_instance, &device_count, physical_devices.data());
        VkPhysicalDevice physical_device=nullptr;

        for(const auto& device : physical_devices) {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(device, &device_properties);

            if (device_properties.deviceType == vk_physical_device_type(p_physical_device_type)) {
                physical_device = device;
            }
        }
        return physical_device;
    }

    std::vector<VkQueueFamilyProperties> enumerate_queue_family_properties(const VkPhysicalDevice& p_physical) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(p_physical, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);

        vkGetPhysicalDeviceQueueFamilyProperties(p_physical,&queue_family_count,queue_family_properties.data());

        return queue_family_properties;
    }

    VkFormat select_compatible_formats(const VkPhysicalDevice& p_physical, const std::span<VkFormat>& p_format_selection, VkImageTiling p_tiling, VkFormatFeatureFlags p_feature_flag) {
        VkFormat format = VK_FORMAT_UNDEFINED;

        for (size_t i = 0; i < p_format_selection.size(); i++) {
            VkFormat current_format = p_format_selection[i];
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

    VkFormat select_depth_format(const VkPhysicalDevice& p_physical, const std::span<VkFormat>& p_format_selection) {

        VkFormat format = select_compatible_formats(
          p_physical,
          p_format_selection,
          VK_IMAGE_TILING_OPTIMAL,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        return format;
    }

    uint32_t physical_memory_properties(const VkPhysicalDevice& p_physical, uint32_t p_type_filter, VkMemoryPropertyFlags p_property_flag) {
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

}