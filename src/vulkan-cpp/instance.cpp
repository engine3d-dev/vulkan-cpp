#define GLFW_INCLUDE_VULKAN
#include <vulkan-cpp/imports.hpp>
#include <vulkan-cpp/instance.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <vector>
#include <print>

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

        // Only run validation layers if we are running vulkan-cpp in debug mode
#if _DEBUG
        // Setting up validation layers
        instance_ci.enabledLayerCount =
          static_cast<uint32_t>(p_config.validations.size());
        instance_ci.ppEnabledLayerNames = p_config.validations.data();

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity =
              to_debug_message_severity(p_debug_message_utils.severity),
            .messageType = to_message_type(p_debug_message_utils.message_type),
            .pfnUserCallback = p_debug_message_utils.callback,
        };

        // This is to invoke the vulkan debug utils if it is a valid callback
        // To ensure that we are not using an invalid debug callback
        if (p_debug_message_utils.callback != nullptr) {
            instance_ci.pNext =
              (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
        }
        else {
            instance_ci.pNext = nullptr;
        }
#else
        instance_ci.enabledLayerCount = 0;
        instance_ci.ppEnabledLayerNames = nullptr;
        instance_ci.pNext = nullptr;
#endif
        vk_check(vkCreateInstance(&instance_ci, nullptr, &m_instance),
                 "vkCreateInstance");
        
#if _DEBUG
        // This needs to be created after the VkInstance is or else it wont be applied the debug information during validation layer error message execution
        m_vk_set_debug_utils_object_name_ext = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT"));
#endif
    }

    void instance::destroy() {
        // only destroy if VkInstance is valid
        if (alive()) {
            vkDestroyInstance(m_instance, nullptr);
        }
    }
};