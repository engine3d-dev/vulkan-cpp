module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <filesystem>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

export module vk:texture;

export import :types;
export import :utilities;
export import :buffer_streams;
export import :sample_image;
export import :command_buffer;

export namespace vk {
    inline namespace v1 {

        sample_image create_texture_with_data(const VkDevice& p_device,
                                              const image_params& p_config,
                                              std::span<const uint8_t> p_data) {
            // 1. Creating temporary command buffer for texture
            command_params copy_command_params = {
                .levels = command_levels::primary,
                .queue_index = 0,
                .flags = command_pool_flags::reset,
            };
            command_buffer temp_command_buffer =
              command_buffer(p_device, copy_command_params);

            // 2. loading texture
            sample_image texture_image = sample_image(p_device, p_config);

            // 3. transfer data from staging buffer
            uint32_t property_flag = memory_property::host_visible_bit |
                                     memory_property::host_cached_bit;

            buffer_parameters staging_buffer_config = {
                .physical_memory_properties =
                  p_config.phsyical_memory_properties,
                .property_flags = static_cast<memory_property>(property_flag),
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            };
            buffer_stream staging(
              p_device, p_data.size(), staging_buffer_config);

            // 4. write data to the staging buffer with specific size specified
            staging.transfer(p_data);

            // 5. start recording to this command buffer
            VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            VkFormat texture_format = p_config.format;

            temp_command_buffer.begin(command_usage::one_time_submit);

            // 6. transition image layout
            // Ensure that we are transferring our image data and correcting the
            // format to ensure we do not lose any data in the process
            texture_image.memory_barrier(
              temp_command_buffer, texture_format, old_layout, new_layout);
            staging.copy_to_image(
              temp_command_buffer, texture_image, p_config.extent);
            old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texture_image.memory_barrier(
              temp_command_buffer, texture_format, old_layout, new_layout);

            temp_command_buffer.end();

            // 8. Getting graphics queue to store the texture data for GPU
            // access
            // TODO: Extend vk::device_queue to enable perform command
            // submission to the GPU
            uint32_t queue_family_index = 0;
            uint32_t queue_index = 0;
            VkQueue temp_graphics_queue;
            vkGetDeviceQueue(
              p_device, queue_family_index, queue_index, &temp_graphics_queue);

            // 8. now submit that texture data to be stored in GPU memory
            const VkCommandBuffer handle = temp_command_buffer;
            VkSubmitInfo submit_info = {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .commandBufferCount = 1,
                .pCommandBuffers = &handle,
            };

            vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
            vkQueueWaitIdle(temp_graphics_queue);

            temp_command_buffer.destroy();
            staging.destroy();

            return texture_image;
        }

        // TODO: Remove redundant struct and replace with vk::image_params
        struct texture_info {
            // for getting image memory requirements for the texture
            VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
            std::filesystem::path filepath;
            uint32_t mip_levels = 1;
            uint32_t layer_count = 1;
        };

        class texture {
        public:
            texture() = default;

            // TODO: Replace these current parameters to using vk::image_params
            // to make the API's consistent.
            texture(const VkDevice& p_device,
                    const image_extent& p_extent,
                    VkPhysicalDeviceMemoryProperties p_property)
              : m_device(p_device) {

                // 1.) Load in extent dimensions
                // White pixels for storing texture.
                std::array<uint8_t, 4> white_color = { 0xFF, 0xFF, 0xFF, 0xFF };

                m_width = p_extent.width;
                m_height = p_extent.height;

                image_params config_image = {
                    .extent = p_extent,
                    // .format = VK_FORMAT_R8G8B8A8_UNORM,
                    .format = static_cast<VkFormat>(format::r8g8b8a8_unorm),
                    .usage =
                      image_usage::transfer_dst_bit | image_usage::sampled_bit,
                    .phsyical_memory_properties = p_property
                };
                int bytes_per_pixel =
                  bytes_per_texture_format(config_image.format);

                // Ensuring we get pass in the correct image size with bytes per
                // pixel
                uint32_t layer_size_with_bytes = config_image.extent.width *
                                                 config_image.extent.height *
                                                 bytes_per_pixel;
                uint32_t layer_count = 1;
                uint32_t image_size = layer_size_with_bytes * layer_count;

                std::span<const uint8_t> bytes(white_color.data(), image_size);
                m_image =
                  create_texture_with_data(m_device, config_image, bytes);
                m_texture_loaded = true;
            }

            // TODO: Replace these current parameters to using vk::image_params
            // to make the API's consistent.
            texture(const VkDevice& p_device,
                    const std::filesystem::path& p_filename,
                    const texture_info& p_texture_info)
              : m_device(p_device) {
                // 1. load from file
                int w, h;
                int channels;
                // TODO: Make passing in the filepath an explicit parameter for
                // loading in a texture
                stbi_uc* image_pixel_data =
                  stbi_load(p_filename.string().c_str(),
                            &w,
                            &h,
                            &channels,
                            STBI_rgb_alpha);

                m_width = w;
                m_height = h;

                if (!image_pixel_data) {
                    m_texture_loaded = false;
                    return;
                }

                // 2. create vulkan image handlers + loading in the image data
                uint32_t property_flag = memory_property::device_local_bit;

                image_params config_image = {
                    .extent = { .width = static_cast<uint32_t>(w),
                                .height = static_cast<uint32_t>(h) },
                    // .format = VK_FORMAT_R8G8B8A8_UNORM,
                    .format = static_cast<VkFormat>(format::r8g8b8a8_unorm),
                    .usage =
                      image_usage::transfer_dst_bit | image_usage::sampled_bit,
                    .mip_levels = p_texture_info.mip_levels,
                    .layer_count = p_texture_info.layer_count,
                    .phsyical_memory_properties =
                      p_texture_info.phsyical_memory_properties,
                };
                int bytes_per_pixel =
                  bytes_per_texture_format(config_image.format);

                // Ensuring we get pass in the correct image size with bytes per
                // pixel
                uint32_t layer_size_with_bytes = config_image.extent.width *
                                                 config_image.extent.height *
                                                 bytes_per_pixel;
                uint32_t layer_count = 1;
                uint32_t image_size = layer_size_with_bytes * layer_count;

                // Validating the correct amount of data to creating the texture
                // with
                std::span<const uint8_t> bytes(
                  as_bytes(image_pixel_data, image_size));
                m_image =
                  create_texture_with_data(p_device, config_image, bytes);

                m_texture_loaded = true;
            }

            [[nodiscard]] bool loaded() const { return m_texture_loaded; }

            [[nodiscard]] sample_image image() const { return m_image; }

            [[nodiscard]] uint32_t width() const { return m_width; }

            [[nodiscard]] uint32_t height() const { return m_height; }

            void destroy() { m_image.destroy(); }

        private:
            VkDevice m_device = nullptr;
            bool m_texture_loaded = false;
            sample_image m_image{};
            uint32_t m_width = 0;
            uint32_t m_height = 0;
        };
    };
};