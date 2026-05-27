module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <filesystem>

export module vk:texture;

import :types;
import :utilities;
import :buffer;
import :sample_image;
import :command_buffer;
import :image;

export namespace vk {
    inline namespace v6 {

        class texture {
        public:
            texture() = default;

            texture(const VkDevice& p_device,
                    const image_extent& p_extent,
                    std::span<const uint8_t> p_color,
                    uint32_t p_memory_mask,
                    uint32_t p_mip_levels = 1,
                    uint32_t p_layer_count = 1)
              : m_device(p_device)
              , m_extent(p_extent) {

                construct(p_extent,
                          p_color,
                          p_memory_mask,
                          p_mip_levels,
                          p_layer_count);
                m_texture_loaded = true;
            }

            texture(const VkDevice& p_device,
                    image* p_image,
                    const texture_params& p_texture_params)
              : m_device(p_device) {
                construct(p_image, p_texture_params);
            }

            void construct(image_extent p_extent,
                           std::span<const uint8_t> p_data,
                           uint32_t p_memory_mask,
                           uint32_t p_mip_levels = 1,
                           uint32_t p_layer_count = 1) {
                m_extent = p_extent;

                const VkFormat texture_format =
                  static_cast<VkFormat>(format::r8g8b8a8_unorm);

                image_params img_options = {
                    .extent = p_extent,
                    .format = texture_format,
                    .memory_mask = p_memory_mask,
                    .usage =
                      image_usage::transfer_dst_bit | image_usage::sampled_bit,
                    .mip_levels = p_mip_levels,
                    .layer_count = p_layer_count,
                };

                m_image = sample_image(m_device, img_options);

                // Performing staging transfers
                buffer_parameters staging_options = {
                    .memory_mask = img_options.memory_mask,
                    .property_flags = memory_property::host_visible_bit |
                                      memory_property::host_cached_bit,
                    .usage = buffer_usage::transfer_src_bit,
                };
                buffer staging(m_device, p_data.size(), staging_options);

                staging.transfer(p_data);

                // Performing transfers as a command to GPU memory for
                // preparations
                command_params copy_command_params = {
                    .levels = command_levels::primary,
                    .queue_index = 0,
                    .flags = command_pool_flags::reset,
                };
                command_buffer temp_command_buffer =
                  command_buffer(m_device, copy_command_params);

                temp_command_buffer.begin(command_usage::one_time_submit);

                // Performing image layouts
                VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                m_image.memory_barrier(temp_command_buffer,
                                       img_options.format,
                                       old_layout,
                                       new_layout);

                std::array<vk::buffer_image_copy, 1> region_copies = {
                    vk::buffer_image_copy{
                        .image_offset = { .width = 0, .height = 0, .depth = 0, },
                        .image_extent = { .width = img_options.extent.width, .height = img_options.extent.height, .depth = 1, },
                    }
                };

                staging.copy_to_image(
                  temp_command_buffer, m_image, region_copies);

                old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                m_image.memory_barrier(temp_command_buffer,
                                       img_options.format,
                                       old_layout,
                                       new_layout);

                temp_command_buffer.end();

                uint32_t queue_family = 0;
                uint32_t queue_index = 0;
                VkQueue temp_graphics_queue = nullptr;
                vkGetDeviceQueue(
                  m_device, queue_family, queue_index, &temp_graphics_queue);

                const VkCommandBuffer handle = temp_command_buffer;
                VkSubmitInfo submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &handle,
                };

                vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
                vkQueueWaitIdle(temp_graphics_queue);

                temp_command_buffer.destruct();
                staging.destruct();
            }

            void construct(image* p_image,
                           const texture_params& p_texture_params) {
                m_extent = p_image->extent();

                const VkFormat texture_format =
                  static_cast<VkFormat>(format::r8g8b8a8_unorm);

                image_params img_options = {
                    .extent = p_image->extent(),
                    .format = texture_format,
                    .memory_mask = p_texture_params.memory_mask,
                    .usage =
                      image_usage::transfer_dst_bit | image_usage::sampled_bit,
                    .mip_levels = p_texture_params.mip_levels,
                    .layer_count = p_texture_params.layer_count,
                };

                m_image = sample_image(m_device, img_options);

                // Setup staging buffer
                uint32_t property_flag = memory_property::host_visible_bit |
                                         memory_property::host_cached_bit;

                buffer_parameters staging_options = {
                    .memory_mask = img_options.memory_mask,
                    .property_flags =
                      static_cast<memory_property>(property_flag),
                    .usage = buffer_usage::transfer_src_bit,
                };
                buffer staging(
                  m_device, p_image->read().size(), staging_options);

                staging.transfer(p_image->read());

                // 5. Creating temporary command buffer for texture
                command_params copy_command_params = {
                    .levels = command_levels::primary,
                    .queue_index = 0,
                    .flags = command_pool_flags::reset,
                };
                command_buffer temp_command_buffer =
                  command_buffer(m_device, copy_command_params);

                temp_command_buffer.begin(command_usage::one_time_submit);

                // Performing image layouts
                VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
                VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                m_image.memory_barrier(temp_command_buffer,
                                       img_options.format,
                                       old_layout,
                                       new_layout);

                std::array<vk::buffer_image_copy, 1> region_copies = {
                    vk::buffer_image_copy{
                        .image_offset = { .width = 0, .height = 0, .depth = 0, },
                        .image_extent = { .width = img_options.extent.width, .height = img_options.extent.height, .depth = 1, },
                    }
                };

                staging.copy_to_image(
                  temp_command_buffer, m_image, region_copies);

                old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                m_image.memory_barrier(temp_command_buffer,
                                       img_options.format,
                                       old_layout,
                                       new_layout);

                temp_command_buffer.end();

                uint32_t queue_family = 0;
                uint32_t queue_index = 0;
                VkQueue temp_graphics_queue = nullptr;
                vkGetDeviceQueue(
                  m_device, queue_family, queue_index, &temp_graphics_queue);

                const VkCommandBuffer handle = temp_command_buffer;
                VkSubmitInfo submit_info = {
                    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers = &handle,
                };

                vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
                vkQueueWaitIdle(temp_graphics_queue);

                temp_command_buffer.destruct();
                staging.destruct();
            }

            //! @return true if image loaded, false if texture did not load
            //! correctly
            [[nodiscard]] bool loaded() const { return m_texture_loaded; }

            [[nodiscard]] sample_image image() const { return m_image; }

            [[nodiscard]] image_extent extent() const { return m_extent; }

            void destruct() { m_image.destruct(); }

        private:
            VkDevice m_device = nullptr;
            bool m_texture_loaded = false;
            sample_image m_image{};
            image_extent m_extent;
            class image* m_image_loader = nullptr;
        };
    };
};