#include <vulkan-cpp/utilities.hpp>
#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>

namespace vk {

    void vk_check(const VkResult& p_result,
                  const std::string& p_name,
                  const std::source_location& p_source) {
        if (p_result != VK_SUCCESS) {
            std::println("File {} on line {} failed VkResult check",
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
        switch (p_version)
        {
        case api_version::vk_1_2:
            return VK_API_VERSION_1_2;
        case api_version::vk_1_3: return VK_API_VERSION_1_3;
        }
    }

}