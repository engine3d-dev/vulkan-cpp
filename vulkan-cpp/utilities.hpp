#pragma once
#include <vulkan-cpp/types.hpp>
#include <source_location>

namespace vk {

    void vk_check(
      const VkResult& p_result,
      const std::string& p_name,
      const std::source_location& p_location = std::source_location::current());

    /**
     * @brief converts vk::message to VkDebugUtilsMessageSeverityFlagsEXT
     *
     * Allows to set specific bit for setting up the message severity.
     *
     * Types of message severity for the specified debug callback
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
     *
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
     *
     * VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
     */
    VkDebugUtilsMessageSeverityFlagsEXT to_debug_message_severity(
      uint32_t p_flag);

    VkDebugUtilsMessageTypeFlagsEXT to_message_type(uint32_t p_flag);

    uint32_t vk_api_version(const api_version& p_version);

    VkPhysicalDeviceType vk_physical_device_type(physical p_physical_type);

	VkPhysicalDevice enumerate_physical_devices(const VkInstance& p_instance, const physical& p_physical_device_type);

	// TODO: Eventually have a free-standing function to return the enumerated physical devices
	// std::span<vk::physical_device> enumerate_physical_devices(const VkInstance& p_instance);

	std::span<VkQueueFamilyProperties> enumerate_queue_family_properties(const VkPhysicalDevice& p_physical);
};