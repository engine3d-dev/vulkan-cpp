module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>
#include <bit>
#include <memory>

export module vk:sample_image;

import :types;
import :utilities;
import :device;

export namespace vk {
    inline namespace v1 {
        /**
         * We shift 32-bits to the high 32-bits for the old layout and OR the
         * new layout lsb 32-bits additional to the shifted 32-bits
         *
         * This is to ensure the image layouts do not overlap and can be used to
         * directly jump to set specific image loyouts rather then doing an
         * if-statement originally to check for that
         */
        constexpr uint64_t image_layout(VkImageLayout p_old,
                                        VkImageLayout p_new) {
            // Shift the old_layout into the high 32 bits, and combine with
            // new_layout in the low 32 bits.
            return (static_cast<uint64_t>(p_old) << 32) |
                   static_cast<uint64_t>(p_new);
        }
        class sample_image {
        public:
            sample_image() = default;
            sample_image(std::shared_ptr<device> p_device,
                         const image_params& p_image_params) {
                m_device = p_device;

                // 1. creating VkImage handle
                VkImageCreateInfo image_ci = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = p_image_params.image_flags,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = p_image_params.format,
                    .extent = { .width = p_image_params.extent.width,
                                .height = p_image_params.extent.height,
                                .depth = 1, },
                    .mipLevels = p_image_params.mip_levels,
                    .arrayLayers = p_image_params.array_layers,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage =
                      static_cast<VkImageUsageFlags>(p_image_params.usage),
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
                };

                vk_check(vkCreateImage(*m_device, &image_ci, nullptr, &m_image),
                         "vkCreateImage");

                // 2. get image memory requirements from physical device
                VkMemoryRequirements memory_requirements;
                vkGetImageMemoryRequirements(
                  *m_device, m_image, &memory_requirements);

                uint32_t mapped_memory_requirements =
                  memory_requirements.memoryTypeBits &
                  p_image_params.memory_mask;

                // Retrieving the next available bits that have been mapped
                uint32_t memory_index =
                  std::countr_zero(mapped_memory_requirements);

                // 4. Allocate info
                VkMemoryAllocateInfo memory_alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .allocationSize = memory_requirements.size,
                    .memoryTypeIndex = memory_index
                };

                vk_check(
                  vkAllocateMemory(
                    *m_device, &memory_alloc_info, nullptr, &m_device_memory),
                  "vkAllocateMemory");

                // 5. bind image memory
                vk_check(
                  vkBindImageMemory(*m_device, m_image, m_device_memory, 0),
                  "vkBindImageMemory");

                // Needs to create VkImageView after VkImage
                // because VkImageView expects a VkImage to be binded to a singl
                // VkDeviceMemory beforehand
                VkImageViewCreateInfo image_view_ci = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = m_image,
                    // .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .viewType = p_image_params.view_type,
                    .format = p_image_params.format,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                        .aspectMask = static_cast<VkImageAspectFlags>(p_image_params.aspect),
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = p_image_params.layer_count,
                    },
                };

                vk_check(vkCreateImageView(
                           *m_device, &image_view_ci, nullptr, &m_image_view),
                         "vkCreateImage");

                // Create VkSampler handler
                VkSamplerCreateInfo sampler_info = {
                    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .magFilter = p_image_params.range.min,
                    .minFilter = p_image_params.range.max,
                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    .addressModeU = static_cast<VkSamplerAddressMode>(
                      p_image_params.address_mode_u),
                    .addressModeV = static_cast<VkSamplerAddressMode>(
                      p_image_params.addrses_mode_v),
                    .addressModeW = static_cast<VkSamplerAddressMode>(
                      p_image_params.addrses_mode_w),
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

                vk_check(
                  vkCreateSampler(*m_device, &sampler_info, nullptr, &m_sampler),
                  "vkCreateSampler");
            }

            sample_image(std::shared_ptr<device> p_device,
                         const VkImage& p_image,
                         const image_params& p_image_params)
              : m_device(p_device)
              , m_image(p_image) {

                VkImageViewCreateInfo image_view_ci = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .image = m_image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = p_image_params.format,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                        .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                    },
                    .subresourceRange = {
                        .aspectMask = static_cast<VkImageAspectFlags>(p_image_params.aspect),
                        .baseMipLevel = 0,
                        .levelCount = p_image_params.mip_levels,
                        .baseArrayLayer = 0,
                        .layerCount = p_image_params.layer_count,
                    },
                };

                vk_check(vkCreateImageView(
                           *m_device, &image_view_ci, nullptr, &m_image_view),
                         "vkCreateImage");

                // Create VkSampler handler
                VkSamplerCreateInfo sampler_info = {
                    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .magFilter = p_image_params.range.min,
                    .minFilter = p_image_params.range.max,
                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    .addressModeU = static_cast<VkSamplerAddressMode>(
                      p_image_params.address_mode_u),
                    .addressModeV = static_cast<VkSamplerAddressMode>(
                      p_image_params.addrses_mode_v),
                    .addressModeW = static_cast<VkSamplerAddressMode>(
                      p_image_params.addrses_mode_w),
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

                vk_check(
                  vkCreateSampler(*m_device, &sampler_info, nullptr, &m_sampler),
                  "vkCreateSampler");

                m_only_destroy_image_view = true;
            }

            [[nodiscard]] VkSampler sampler() const { return m_sampler; }

            [[nodiscard]] VkImageView image_view() const {
                return m_image_view;
            }

            /**
             * @brief performs vkCmdPipelineBarrier to handle transitioning
             * image layouts
             *
             * @param p_command is the current command buffer to record the
             * image layout transition
             * @param p_format is the image format to make sure if there is a
             * depth format available then request the aspect mask to include
             * the stencil bit
             * @param p_old is the source image layout transition from
             * @param p_new is the destination image layout transition to.
             *
             *
             * ```C++
             *
             * sample_image texture_image(logical_device, ...);
             *
             *
             * texture_image.memory_barrier(temp_command, some_vk_format,
             * VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
             *
             * // do some operation during this transition image layout
             * // such as copying from an buffer data to an image
             * // staging_buffer.copy_to_image(...);
             *
             * texture_image.memory_barrier(temp_command, some_vk_format,
             * VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
             * VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
             *
             * ```
             *
             */
            void memory_barrier(
              const VkCommandBuffer& p_command,
              VkFormat p_format,
              VkImageLayout p_old,
              VkImageLayout p_new,
              uint32_t p_aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT) {

                // 1. Image Memory Barrier Initialization (using C++ Designated
                // Initializers - C++20)
                VkImageMemoryBarrier image_memory_barrier = {
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = 0,
                    .dstAccessMask = 0,
                    .oldLayout = p_old,
                    .newLayout = p_new,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = m_image,
                    .subresourceRange = { .aspectMask =
                                            static_cast<VkImageAspectFlags>(
                                              p_aspect_mask),
                                          .baseMipLevel = 0,
                                          .levelCount = 1,
                                          .baseArrayLayer = 0,
                                          .layerCount = 1 }
                };

                VkPipelineStageFlags source_stage = VK_PIPELINE_STAGE_NONE;
                VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_NONE;

                // 2. Aspect Mask Logic (Keep as if/else, but use helper
                // function)
                if (p_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
                    has_stencil_attachment(p_format)) {

                    image_memory_barrier.subresourceRange.aspectMask =
                      VK_IMAGE_ASPECT_DEPTH_BIT;

                    // Assuming has_stencil_attachment(p_format) is defined
                    // elsewhere works as the same as the if-statement, leaving
                    // it here for testing purposes
                    // image_memory_barrier.subresourceRange.aspectMask |=
                    // has_stencil_attachment(p_format) ?
                    // VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
                    if (has_stencil_attachment(p_format)) {
                        image_memory_barrier.subresourceRange.aspectMask |=
                          VK_IMAGE_ASPECT_STENCIL_BIT;
                    }
                }
                else {
                    image_memory_barrier.subresourceRange.aspectMask =
                      VK_IMAGE_ASPECT_COLOR_BIT;
                }

                // 3. Main Transition Logic using Combined Switch
                const uint64_t current_layout = image_layout(p_old, p_new);

                switch (current_layout) {

                    // UNDEFINED -> SHADER_READ_ONLY_OPTIMAL
                    case image_layout(
                      VK_IMAGE_LAYOUT_UNDEFINED,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): {
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // UNDEFINED -> GENERAL
                    case image_layout(VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_GENERAL): {
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // UNDEFINED -> TRANSFER_DST_OPTIMAL
                    case image_layout(VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL): {
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_TRANSFER_WRITE_BIT;
                        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        break;
                    }

                    // SHADER_READ_ONLY_OPTIMAL -> TRANSFER_DST_OPTIMAL (Convert
                    // back from read-only to transferr)
                    case image_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_TRANSFER_WRITE_BIT;
                        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dst_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        break;
                    }

                    // TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL (Convert
                    // from updateable texture to shader read-only)
                    case image_layout(
                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_TRANSFER_WRITE_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // UNDEFINED -> DEPTH_STENCIL_ATTACHMENT_OPTIMAL (Convert
                    // depth texture from undefined state)
                    case image_layout(
                      VK_IMAGE_LAYOUT_UNDEFINED,
                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL): {
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dst_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
                        break;
                    }

                    // SHADER_READ_ONLY_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
                    // (Wait for render pass to complete - Note: This case is
                    // unusual but kept as per your original logic)
                    case image_layout(
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): {
                        // Note: Your original code had conflicting
                        // re-assignments for source_stage/dst_stages here. The
                        // last pair of assignments is used.
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask = 0;
                        source_stage =
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // SHADER_READ_ONLY_OPTIMAL -> COLOR_ATTACHMENT_OPTIMAL
                    // (Convert back from read-only to color attachment)
                    case image_layout(
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dst_stages =
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        break;
                    }

                    // COLOR_ATTACHMENT_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
                    // (Convert from updateable color to shader read-only)
                    case image_layout(
                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        source_stage =
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // SHADER_READ_ONLY_OPTIMAL ->
                    // DEPTH_STENCIL_ATTACHMENT_OPTIMAL (Convert back from
                    // read-only to depth attachment)
                    case image_layout(
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        source_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        dst_stages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        break;
                    }

                    // DEPTH_STENCIL_ATTACHMENT_OPTIMAL ->
                    // SHADER_READ_ONLY_OPTIMAL (Convert from updateable depth
                    // texture to shader read-only)
                    case image_layout(
                      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_SHADER_READ_BIT;
                        source_stage =
                          VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        dst_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        break;
                    }

                    // LAYOUT_UNDEFINED -> COLOR_ATTACHMENT_OPTIMAL
                    // Transition to a color attachment
                    case image_layout(
                      VK_IMAGE_LAYOUT_UNDEFINED,
                      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL): {
                        image_memory_barrier.srcAccessMask = 0;
                        image_memory_barrier.dstAccessMask =
                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

                        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        dst_stages =
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    }

                    // COLOR_ATTACHMENT_OPTIMAL -> PRESENT_SRC_KHR
                    // Transition from being a color attachment to being a
                    // presentable operation
                    case image_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR): {
                        image_memory_barrier.srcAccessMask =
                          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                        image_memory_barrier.dstAccessMask = 0;

                        source_stage =
                          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        dst_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                    }

                    default: {
                        // Unhandled Transition
                        break;
                    }
                }

                vkCmdPipelineBarrier(p_command,
                                     source_stage,
                                     dst_stages,
                                     0, // dependencyFlags
                                     0,
                                     nullptr,
                                     0,
                                     nullptr,
                                     1,
                                     &image_memory_barrier);
            }

            void destruct() {
                if (m_image_view != nullptr) {
                    vkDestroyImageView(*m_device, m_image_view, nullptr);
                }

                // Boolean check is to make sure we might only want
                // to destroy vk::sample_image resources.

                // Example of this is the swapchain may pass in
                // its images and we should only destruct the VkImageView
                // and not the swapchain's images directly
                if (m_image != nullptr and !m_only_destroy_image_view) {
                    vkDestroyImage(*m_device, m_image, nullptr);
                }

                if (m_sampler != nullptr) {
                    vkDestroySampler(*m_device, m_sampler, nullptr);
                }

                if (m_device_memory != nullptr) {
                    vkFreeMemory(*m_device, m_device_memory, nullptr);
                }
            }

            operator VkImage() const { return m_image; }

            operator VkImage() { return m_image; }

        private:
            bool m_only_destroy_image_view = false;
            std::shared_ptr<device> m_device = nullptr;
            VkImage m_image = nullptr;
            VkImageView m_image_view = nullptr;
            VkSampler m_sampler = nullptr;
            VkDeviceMemory m_device_memory = nullptr;
        };
    };
};