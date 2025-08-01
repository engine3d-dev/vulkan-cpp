#include <vulkan-cpp/texture.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <vulkan-cpp/command_buffer.hpp>
#include <print>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

namespace vk {

    static sampled_image create_texture_with_data(const VkDevice& p_device, const image_configuration& p_config, const void* p_data) {
        // 1. Creating temporary command buffer for texture
        command_enumeration copy_command_enumeration = {
            .levels = command_levels::primary,
            .queue_index = 0,
            .flags = command_pool_flags::reset,
        };
        command_buffer temp_command_buffer = command_buffer(p_device, copy_command_enumeration);

        // 2. loading texture
        std::println("Created 2d image handlers");
        sampled_image texture_image = create_sample_image2d(p_device, p_config);
        std::println("Searched for bytes per pixel format!!");
        int bytes_per_pixel = bytes_per_texture_format(p_config.format);

        // 3. getting layer size
        uint32_t layer_size_with_bytes = p_config.width * p_config.height * bytes_per_pixel;
        uint32_t layer_count = 1;
        uint32_t image_size = layer_size_with_bytes * layer_count;

        // 4. transfer data from staging buffer
        uint32_t property_flag = memory_property::host_visible_bit | memory_property::host_cached_bit;
        buffer_configuration staging_buffer_config = {
            .device_size = (uint32_t)image_size,
            .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            .property_flags = (memory_property)property_flag,
            .physical = p_config.physical_device
        };

        buffer_handle staging_buffer = create_buffer(p_device, staging_buffer_config);
        std::println("Created staging buffer");

        // 5. write data to the staging buffer with specific size specified
        write(p_device, staging_buffer, p_data, image_size);
        std::println("Wrote staging buffer");

        // 6. start recording to this command buffer
        VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        VkFormat texture_format = p_config.format;

        std::println("before command record!!!");
        temp_command_buffer.begin(command_usage::one_time_submit);

        // 6.1 -- transition image layout
        image_memory_barrier(temp_command_buffer, texture_image.image, texture_format, old_layout, new_layout);

        // 6.2 -- copy buffer to image handles
        copy(temp_command_buffer, texture_image, staging_buffer, p_config.width, p_config.height);

        // 6.3 -- transition image layout back to the layout specification
        old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_memory_barrier(temp_command_buffer, texture_image.image, texture_format, old_layout, new_layout);

        temp_command_buffer.end();


        // 7. Create temporary graphics queue to offload the texture image into GPU memory
        //! TODO: Do this better then just retrieving graphics queue 0
        uint32_t queue_family_index = 0;
        uint32_t queue_index = 0;
        VkQueue temp_graphics_queue;
        vkGetDeviceQueue(p_device, queue_family_index, queue_index, &temp_graphics_queue);

        // 8. now submit that texture data to be stored in GPU memory
        VkCommandBuffer handle = temp_command_buffer;
        VkSubmitInfo submit_info = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &handle,
        };

        vkQueueSubmit(temp_graphics_queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(temp_graphics_queue);

        temp_command_buffer.destroy();
        free_buffer(p_device, staging_buffer);

        return texture_image;
    }

    texture::texture(const VkDevice& p_device, const texture_info& p_texture_info) : m_device(p_device) {

        // 1. load from file
        int w, h;
        int channels;
        stbi_uc* image_pixel_data = stbi_load(p_texture_info.filepath.string().c_str(), &w, &h, &channels, STBI_rgb_alpha);

        m_width = w;
        m_height = h;

        if (!image_pixel_data) {
            m_texture_loaded = false;
            return;
        }
        else {
            std::println("{} loaded!!!", p_texture_info.filepath.string());
        }

        // 2. create vulkan image handlers + loading in the image data

        uint32_t property_flag = memory_property::device_local_bit;

        image_configuration config_image = {
            .width = (uint32_t)w,
            .height = (uint32_t)h,
            .format = VK_FORMAT_R8G8B8A8_UNORM,
            .usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
            .property = (memory_property)property_flag,
            .physical_device = p_texture_info.physical
        };

        m_image_handle = create_texture_with_data(p_device, config_image, image_pixel_data);

        // 3.) Create Image View
        VkImageAspectFlags aspect_flags = VK_IMAGE_ASPECT_COLOR_BIT;
        // m_image_handle.view = create_image2d_view(m_image_handle.image, config_image.format, aspect_flags);
        m_image_handle.view = create_image2d_view(p_device, m_image_handle.image, config_image.format, aspect_flags);

        filter_range sampler_range = { .min = VK_FILTER_LINEAR,
                                          .max = VK_FILTER_LINEAR };

        VkSamplerAddressMode addr_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        m_image_handle.sampler = create_sampler(p_device, sampler_range, addr_mode);

        m_texture_loaded = true;
    }

    void texture::destroy() {
        free_image(m_device, m_image_handle);
    }
};