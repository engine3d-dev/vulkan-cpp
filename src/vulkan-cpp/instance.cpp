#define GLFW_INCLUDE_VULKAN
#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vulkan/vulkan.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#endif
#include <vulkan-cpp/instance.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <vector>

namespace vk {

    instance::instance(const application_configuration& p_config,
                       const debug_message_utility& p_debug_message_utils) {
        VkApplicationInfo app_info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .applicationVersion = 1,
            .pEngineName = p_config.name.c_str(),
            .engineVersion = 1,
            .apiVersion = vk_api_version(p_config.version),
        };

        VkInstanceCreateInfo instance_ci = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &app_info
        };

        // Setting up validation layers properties
        uint32_t layer_count = 0;
        std::vector<VkLayerProperties> layer_properties;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        // std::vector<VkLayerProperties> layer_properties(layer_count);
        layer_properties.resize(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count,
                                           layer_properties.data());

        for (const VkLayerProperties property : layer_properties) {
            m_layer_properties.emplace_back(property.layerName,
                                            property.specVersion,
                                            property.implementationVersion,
                                            property.description);
        }

        // Setting up instance extensions
        instance_ci.enabledExtensionCount =
          static_cast<uint32_t>(p_config.extensions.size());
        instance_ci.ppEnabledExtensionNames = p_config.extensions.data();

#if _DEBUG
        // Setting up validation layers
        instance_ci.enabledLayerCount =
          static_cast<uint32_t>(p_config.validations.size());
        instance_ci.ppEnabledLayerNames = p_config.validations.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            // .messageSeverity =
            // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            //                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            //                    |
            //                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageSeverity =
              to_debug_message_severity(p_debug_message_utils.severity),
            // .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            //                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            //                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .messageType = to_message_type(p_debug_message_utils.message_type),
            .pfnUserCallback = p_debug_message_utils.callback,
        };

        instance_ci.pNext =
          (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
#else
        instance_ci.enabledLayerCount = 0;
        instance_ci.ppEnabledLayerNames = nullptr;
        instance_ci.pNext = nullptr;
#endif
        vk_check(vkCreateInstance(&instance_ci, nullptr, &m_instance),
                 "vkCreateInstance");
    }

    void instance::destroy() {
        // only destroy VkInstance if it is valid
        if (alive()) {
            vkDestroyInstance(m_instance, nullptr);
        }
    }
};