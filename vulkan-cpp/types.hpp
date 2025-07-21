#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <span>

namespace vk {

    /**
     * @brief message sevierity explicitly to max size of a byte
     */
    enum message : uint8_t { verbose=0x1, warning=0x2, error=0x3 };

    enum debug : uint8_t { general=0x1, validation=0x2, performance=0x3 };

    enum class api_version : uint32_t {
        vk_1_2,
        vk_1_3, // vulkan version 1.3
        // vk_1_2, // vulkan version 1.4
    };

    template<typename T>
    concept IsVkMessage = std::is_same_v<std::remove_cvref_t<T>, vk::message>;

    struct debug_message_utility {
        uint32_t severity;
        uint32_t message_type;
        uint32_t (*callback)(VkDebugUtilsMessageSeverityFlagBitsEXT,
                             VkDebugUtilsMessageTypeFlagsEXT,
                             const VkDebugUtilsMessengerCallbackDataEXT*,
                             void*) = nullptr;
    };

    struct application_configuration {
        std::string name = "";
        api_version version;
        std::span<const char*> validations;
        std::span<const char*> extensions;
    };

    struct layer_properties {
        std::string name;
        uint32_t version;
        uint32_t impl_version;
        std::string description;
    };

};