#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>
#include <string>
#include <span>

namespace vk {

    /**
     * @brief message sevierity explicitly to max size of a byte
     */
    enum message : uint8_t { verbose = 0x1, warning = 0x2, error = 0x3 };

    enum debug : uint8_t { general = 0x1, validation = 0x2, performance = 0x3 };

    enum class api_version : uint32_t {
        vk_1_2,
        vk_1_3, // vulkan version 1.3
        // vk_1_2, // vulkan version 1.4
    };

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


    //! @brief vk::physical defines what kinds of physical device specification to use that is available based on your current physical hardware specifications.
    enum class physical {
        integrated,
        discrete,
        virtualized,
        cpu,
        max_enum,
        other
    };

    /**
     * @brief Enumeration represents configuration for the physical device
     * 
     * Defines the configuration for the VkPhysicalDevice handler to be created with
    */
    struct physical_enumeration {
        physical device_type;
    };

    struct physical_queue_enumeration {
        uint32_t queue_count=0;
    };
};