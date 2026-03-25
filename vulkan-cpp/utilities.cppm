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
                      const std::source_location& p_source = {}) {
            if (p_result != VK_SUCCESS) {
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

        /**
         * @brief Used to convert a given set of types T into chunks of bytes.
         *
         * This is used for the vulkan-cpp API's that expect to take in a
         * span<const uint8_t> to transfer data over to the Vulkan C API, that
         * expects a void*.
         */
        template<typename T>
        std::span<uint8_t> to_bytes(T p_data) {
            return std::span<uint8_t>(reinterpret_cast<uint8_t*>(&p_data),
                                      sizeof(p_data));
        }

        /**
         * @brief Bitwise OR Overloads Adapters
         *
         * These operator overloads allow for `enum class`'s which are strongly
         * typed to be temporarily truncates the types to their original type.
         *
         * Performing bitwise OR operation to those particular enum class types
         * that can perform those operations.
         *
         */
        inline memory_property operator|(memory_property p_lhs,
                                         memory_property p_rhs) {
            // Lets us truncate the underlying type of the enum (class) to allow
            // it to be bitwise OR'd
            using T = std::underlying_type_t<memory_property>;
            return static_cast<memory_property>(static_cast<T>(p_lhs) |
                                                static_cast<T>(p_rhs));
        }

        inline buffer_usage operator|(buffer_usage p_lhs, buffer_usage p_rhs) {
            // Lets us truncate the underlying type of the enum (class) to allow
            // it to be bitwise OR'd
            using T = std::underlying_type_t<buffer_usage>;
            return static_cast<buffer_usage>(static_cast<T>(p_lhs) |
                                             static_cast<T>(p_rhs));
        }

        inline image_aspect_flags operator|(image_aspect_flags p_lhs,
                                            image_aspect_flags p_rhs) {
            // Lets us truncate the underlying type of the enum (class) to allow
            // it to be bitwise OR'd
            using T = std::underlying_type_t<image_aspect_flags>;
            return static_cast<image_aspect_flags>(static_cast<T>(p_lhs) |
                                                   static_cast<T>(p_rhs));
        }

        /**
         * @brief GPU memory is optimized differently for different tasks. An
         * image optimized for 'Transfer Destination' (filling with bytes) can
         * be look different in memory then an image configured for 'Shader Read
         * Only'.
         *
         * This API performs a transition operation which 'reformats' the image
         * data at the hardware-level.
         *
         * Usually acts as a syncing point, ensuring the GPU finishes writing to
         * the image before it makes an attempt to read from it.
         *
         * [ Image Memory ]           [ Transition ]         [ Image Memory ]
         * +------------------+    +----------------+    +---------------------+
         * | Layout: Undefined|    | Memory Flush   |    | Layout: Shader Read |
         * | Access: 0        | => | Layout Reformat| => | Access: Shader Read |
         * | (Initial/Invalid)|    | Exec Stall     |    | (For Sampling       |
         * +------------------+    +----------------+    +---------------------+
         *
         * @brief Additional Consideration:
         * - uses vkQueueWaitIdle, which stalls the CPU until the GPU is idle.
         * - Only used for initialization or infrequent updates.
         * - Use vk::image_layout::undefined as the old layout is faster but
         * discards pixel data.
         * - Transition applies to the entire subresource range (all
         * mips/layers) defined within the p_image barrier logic.
         *
         */
        /*
        void transition_image_layout(VkDevice p_device,
                                     vk::sample_image& p_image,
                                     VkFormat p_format,
                                     VkImageLayout p_old,
                                     VkImageLayout p_new) {
            vk::command_params copy_command_params = {
                .levels = vk::command_levels::primary,
                .queue_index = 0,
                .flags = vk::command_pool_flags::reset,
            };
            vk::command_buffer temp_command_buffer =
              vk::command_buffer(p_device, copy_command_params);

            temp_command_buffer.begin(vk::command_usage::one_time_submit);

            p_image.memory_barrier(temp_command_buffer, p_format, p_old, p_new);

            temp_command_buffer.end();

            VkCommandBuffer handle = temp_command_buffer;
            VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &handle,
            };

            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
            VkQueue temp_graphics_queue;
            vkGetDeviceQueue(
              p_device, queue_family_index, queue_index, &temp_graphics_queue);

            vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
            vkQueueWaitIdle(temp_graphics_queue);

            temp_command_buffer.destroy();
        }
        */

    }; // end of v1 namespace
};