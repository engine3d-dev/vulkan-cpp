module;

#include <vulkan/vulkan.h>
#include <filesystem>
#include <print>
#include <span>
#include <source_location>
#include <vector>

export module vk:utilities;

import :types;

export namespace vk {
    inline namespace v6 {

        void vk_check(const VkResult& p_result, const std::string& p_name) {
            if (p_result != VK_SUCCESS) {
                std::println("{} VkResult returned: {}",
                             p_name,
                             static_cast<int>(p_result));
            }
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

        inline image_usage operator|(image_usage p_lhs, image_usage p_rhs) {
            // Lets us truncate the underlying type of the enum (class) to allow
            // it to be bitwise OR'd
            using T = std::underlying_type_t<image_usage>;
            return static_cast<image_usage>(static_cast<T>(p_lhs) |
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

        inline descriptor_bind_flags operator|(descriptor_bind_flags p_lhs,
                                               descriptor_bind_flags p_rhs) {
            using T = std::underlying_type_t<descriptor_bind_flags>;
            return static_cast<descriptor_bind_flags>(static_cast<T>(p_lhs) |
                                                      static_cast<T>(p_rhs));
        }

    }; // end of version namespace
};