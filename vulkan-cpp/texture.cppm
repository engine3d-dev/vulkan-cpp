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

            // 1. loading texture
            sample_image texture_image = sample_image(p_device, p_config);

            // 2. transfer data from staging buffer
            uint32_t property_flag = memory_property::host_visible_bit |
                                     memory_property::host_cached_bit;

            buffer_parameters staging_buffer_config = {
                .memory_mask = p_config.memory_mask,
                .property_flags = static_cast<memory_property>(property_flag),
                .usage = static_cast<uint32_t>(buffer_usage::transfer_src_bit),
            };
            buffer_stream staging(
              p_device, p_data.size(), staging_buffer_config);

            // 3. write data to the staging buffer with specific size specified
            staging.transfer(p_data);

            // 4. start recording to this command buffer
            VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            VkFormat texture_format = p_config.format;

            // 5. Creating temporary command buffer for texture
            command_params copy_command_params = {
                .levels = command_levels::primary,
                .queue_index = 0,
                .flags = command_pool_flags::reset,
            };
            command_buffer temp_command_buffer =
              command_buffer(p_device, copy_command_params);

            temp_command_buffer.begin(command_usage::one_time_submit);

            // 6. transition image layout
            // Ensure that we are transferring our image data and correcting the
            // format to ensure we do not lose any data in the process
            texture_image.memory_barrier(
              temp_command_buffer, texture_format, old_layout, new_layout);

            std::array<vk::buffer_image_copy, 1> region_copies = {
                vk::buffer_image_copy{
                    .image_offset = { .width = 0, .height = 0, .depth = 0, },
                    .image_extent = { .width = p_config.extent.width, .height = p_config.extent.height, .depth = 1, },
                }
            };

            // staging.copy_to_image(temp_command_buffer, texture_image,
            // p_config.extent);
            staging.copy_to_image(
              temp_command_buffer, texture_image, region_copies);
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
        struct texture_params {
            uint32_t memory_mask = 0;
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
                    uint32_t p_memory_mask)
              : m_device(p_device)
              , m_extent(p_extent) {

                // 1.) Load in extent dimensions
                // White pixels for storing texture.
                std::array<uint8_t, 4> white_color = { 0xFF, 0xFF, 0xFF, 0xFF };

                image_params config_image = {
                    .extent = m_extent,
                    .format = static_cast<VkFormat>(format::r8g8b8a8_unorm),
                    .memory_mask = p_memory_mask,
                    .usage =
                      static_cast<uint32_t>(image_usage::transfer_dst_bit) |
                      static_cast<uint32_t>(image_usage::sampled_bit),
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

                m_image = create_texture_with_data(
                  m_device,
                  config_image,
                  std::span<const uint8_t>(white_color.data(), image_size));
                m_texture_loaded = true;
            }

            // TODO: Replace these current parameters to using vk::image_params
            // to make the API's consistent.
            texture(const VkDevice& p_device,
                    const std::filesystem::path& p_filename,
                    const texture_params& p_texture_params)
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
                m_extent = {
                    .width = static_cast<uint32_t>(w),
                    .height = static_cast<uint32_t>(h),
                };

                const VkFormat texture_format =
                  static_cast<VkFormat>(format::r8g8b8a8_unorm);
                int bytes_per_pixel = bytes_per_texture_format(texture_format);

                // Ensuring we get pass in the correct image size
                // with bytes per pixel
                // uint32_t layer_size_with_bytes =
                uint32_t image_layer_sizes_with_bytes =
                  m_extent.width * m_extent.height * bytes_per_pixel;

                // uint32_t layer_count = 1;
                uint32_t layer_count = p_texture_params.layer_count;
                uint32_t image_size =
                  image_layer_sizes_with_bytes * layer_count;

                image_params config_image = {
                    .extent = m_extent,
                    .format = texture_format,
                    .memory_mask = p_texture_params.memory_mask,
                    .usage =
                      static_cast<uint32_t>(image_usage::transfer_dst_bit) |
                      static_cast<uint32_t>(image_usage::sampled_bit),
                    .mip_levels = p_texture_params.mip_levels,
                    .layer_count = p_texture_params.layer_count,
                };

                // Ensures the image data is valid before continuing
                // We can check the state of the image if it loaded succesfully
                if (!image_pixel_data) {
                    m_texture_loaded = false;
                    return;
                }

                // Creating Image Handle + Loading Image Pixel Data
                m_image = create_texture_with_data(
                  p_device,
                  config_image,
                  std::span<uint8_t>(image_pixel_data, image_size));

                // Proper cleanup of the image data
                stbi_image_free(image_pixel_data);
                m_texture_loaded = true;
            }

            //! @return true if image loaded, false if texture did not load
            //! correctly
            [[nodiscard]] bool loaded() const { return m_texture_loaded; }

            [[nodiscard]] sample_image image() const { return m_image; }

            [[nodiscard]] image_extent extent() const { return m_extent; }

            void destroy() { m_image.destroy(); }

        private:
            VkDevice m_device = nullptr;
            bool m_texture_loaded = false;
            sample_image m_image{};
            image_extent m_extent{};
        };
    };
};