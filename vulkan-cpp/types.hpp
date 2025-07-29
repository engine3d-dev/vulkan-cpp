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

    struct swapchain_enumeration {
        uint32_t width;
        uint32_t height;
        uint32_t present_index=-1;

        VkFormat depth; // depth format
    };

    struct filter_range {
        VkFilter min;
        VkFilter max;
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

    struct surface_enumeration {
        VkSurfaceCapabilitiesKHR capabilities;
        VkSurfaceFormatKHR format;
    };

    struct queue_enumeration {
        uint32_t family=-1;
        uint32_t index=-1;
    };

    struct queue_indices {
        uint32_t graphics=-1;
        uint32_t compute=-1;
        uint32_t transfer=-1;
    };

    struct device_enumeration {
        std::span<float> queue_priorities{};
        std::span<const char*> extensions{}; // Can add VK_KHR_SWAPCHAIN_EXTENSION_NAME to this extension
        uint32_t queue_family_index=0;
    };

    // raw image handlers
    struct image {
        VkImage image=nullptr;
        VkImageView view=nullptr;
    };

    // sampler + raw image handlers
    struct sampled_image {
        VkImage image=nullptr;
        VkImageView view=nullptr;
        VkSampler sampler=nullptr;
        VkDeviceMemory device_memory=nullptr;
    };

    //! @brief enumeration if an image is provided
    struct swapchain_image_enumeration {
        VkImage image=nullptr;
        VkFormat format;
        VkImageAspectFlags aspect;
        uint32_t layer_count=0;
        uint32_t mip_levels=1;
    };

    // Image enumeration for creating a brand new VkImage/VkImageView handlers
    struct image_enumeration {
        uint32_t width=-1;
        uint32_t height=-1;
        VkFormat format;
        VkImageAspectFlags aspect;
        uint32_t layer_count=1;
        uint32_t mip_levels=1;
    };

    /**
     * @param Primary
     * submission: immediately to vulkan queue for executing on device
     * execution: vkQueueSubmit primarily submits primary command buffers
     *
     * @param Seconary
     * submission: Cannot be submitted directly to vulkan queue
     * execution: Indirect execution by being called from primary command
     * buffers using `vkCmdExecuteCommands`
     *
     * When executed within render-pass instance, they inherit
     * renderpass/subpass state from primary command buffer
     *
     * Can be allocated and recorded in parallel, which can leverage
     * devices that support multi-core processing
     *
     * Have independent lifetimes that can be managed independently of
     * primary command buffers, allowing for more flexible resource management
     */
    enum command_levels : uint8_t {
        primary = 0,
        secondary = 1,
        max_enum = 2
    };

    /**
     * @brief settings for specifying command buffers to construct
     *
     * @param Levels are for specifying the kind of command buffer being
     * constructed
     *
     * queue_index specifies the queue family associated with this
     * command buffer
     *
     * pool_flag in the context of the command buffers are used to
     * control memory allocation behavior within the command buffer pool to
     * allocate
     *
     * @param VK_COMMAND_POOL_CREATE_TRANSIENT_BIT Indicates command buffers
     * allocated from pool will be short-lived, meaning they'll reset or freed
     * quickly
     * @param VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT Allows individual
     * command buffers allocated from the pool to be reset to their initial
     * state using `vkResetCommandBuffer`
     * @param VK_COMMAND_POOL_CREATE_PROTECTED_BIT Designates command buffers
     * allocated from the pool as protective command buffers, implying they are
     * stored in protected memory and access to them is restricted to authorized
     * operations
     */
    struct command_enumeration {
        command_enumeration(uint32_t p_queue_family,
                                const command_levels& p_levels,
                                const VkCommandPoolCreateFlagBits& p_pool_flags)
          : levels(p_levels)
          , queue_index(p_queue_family)
          , pool_flag(p_pool_flags) {}

        command_levels levels;
        uint32_t queue_index = -1;
        VkCommandPoolCreateFlagBits pool_flag;
    };

    struct renderpass_attachments {
        std::span<VkClearColorValue> clear_values{};
        std::span<VkAttachmentDescription> attachments{};
        std::span<VkSubpassDescription> subpass_descriptions{};
        std::span<VkSubpassDependency> dependencies{};
    };
};