#include <vulkan-cpp/skybox_texture.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <vulkan-cpp/command_buffer.hpp>
#include <print>
#include <stb_image.h>

namespace vk {

    skybox_texture::skybox_texture(const VkDevice& p_device, const skybox_texture_info& p_config) : m_device(p_device){
        if(p_config.physical_handle == nullptr) {
            std::println("Cannot utilize a nullptr physical handle!!!");
            return;
        }

        
        // uint32_t bytes_per_pixel = bytes_per_texture_format(cubemap_config.format);
        // uint32_t layer_size_with_bytes = cubemap_config.extent.width * cubemap_config.extent.height * bytes_per_pixel;
        // uint32_t layer_count = 1;
        // uint32_t image_size = layer_size_with_bytes * layer_count;
        

        // 1. Creating temporary command buffer for texture
        command_enumeration copy_command_enumeration = {
            .levels = command_levels::primary,
            .queue_index = 0,
            .flags = command_pool_flags::reset,
        };
        command_buffer temp_command_buffer = command_buffer(p_device, copy_command_enumeration);
        // --------------------------------------
        // BEGIN COMMAND BUFFER RECORDING
        // --------------------------------------
        temp_command_buffer.begin(command_usage::one_time_submit);


        // Cubemap implementation below

        // 1. Load Faces stb image

        stbi_uc* image_pixel_data[6];
        for(size_t i = 0; i < p_config.faces.size(); i++) {
            int channels;
            image_pixel_data[i] = stbi_load(p_config.faces[i].c_str(), &m_width, &m_height, &channels, STBI_rgb_alpha);

            // int bytes_per_pixel = bytes_per_pixel_format(p_config.format)
            // int image_size = m_width * m_height * 4;

            if(!image_pixel_data[i]) {
                std::println("Cannot load face[{}] = filanme with {}", i, p_config.faces[i]);
                return;
            }

            image_configuration_information skybox_image_info = {
                .extent = {(uint32_t)m_width, (uint32_t)m_height},
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .usage = (VkImageUsageFlagBits)VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                .image_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
                .array_layers = 6,
                .physical_device = p_config.physical_handle
            };
            m_cubemap_images[i] = vk::sample_image(m_device, skybox_image_info);

            int bytes_per_pixel = bytes_per_texture_format(skybox_image_info.format);
            uint64_t layer_size_with_bytes = (uint64_t)skybox_image_info.extent.width * skybox_image_info.extent.height * bytes_per_pixel;
            int layer_count = 1;
            uint64_t image_size = layer_size_with_bytes * layer_count;

            int property_flag = memory_property::host_visible_bit | memory_property::host_cached_bit;
            buffer_configuration staging_buffer_config = {
                .device_size = (VkDeviceSize)image_size,
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                .property_flags = (memory_property)property_flag,
                .physical = skybox_image_info.physical_device
            };

            buffer_handle staging_buffer = create_buffer(m_device, staging_buffer_config);
            write_info write_staging_buffer = {
                .offset = i * image_size,
                .size_bytes = layer_size_with_bytes
            };

            write(m_device, staging_buffer, image_pixel_data, write_staging_buffer);

            free_buffer(m_device, staging_buffer);
        }

        // 2. creating staging buffer

        // 2. Load each face with vk::sample_image
        // image_configuration_information skybox_image_info = {
        //     .extent = {(uint32_t)m_width, (uint32_t)m_height},
        //     .format = VK_FORMAT_R8G8B8A8_UNORM,
        //     .usage = (VkImageUsageFlagBits)VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        //     .image_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
        //     .array_layers = 6,
        //     .physical_device = p_config.physical_handle
        // };
        // m_skybox_image = vk::sample_image(m_device, skybox_image_info);

        // int bytes_per_pixel = bytes_per_texture_format(skybox_image_info.format);
        // int layer_size_with_bytes = static_cast<int>(skybox_image_info.extent.width * skybox_image_info.extent.height * bytes_per_pixel);
        // std::println("layer_size = {}", layer_size_with_bytes);
        // int layer_count = 6;
        // int image_size = layer_size_with_bytes * layer_count;
        // std::println("image_size = {}", image_size);

        // int property_flag = memory_property::host_visible_bit | memory_property::host_cached_bit;
        // buffer_configuration staging_buffer_config = {
        //     .device_size = (VkDeviceSize)image_size,
        //     .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        //     .property_flags = (memory_property)property_flag,
        //     .physical = skybox_image_info.physical_device
        // };

        // buffer_handle staging_buffer = create_buffer(m_device, staging_buffer_config);
        
        // for(size_t i = 0; i < 6; i++) {
        //     std::println("i * image_size = {}", (i * image_size));
        //     write(m_device, staging_buffer, image_pixel_data, i * image_size);
        //     // std::println("Mapping i = {}", i);
        //     // void* mapped = nullptr;
        //     // vk_check(
        //     // vkMapMemory(
        //     //     p_device, staging_buffer.device_memory, i * image_size, image_size, 0, &mapped),
        //     // "vkMapMemory");

        //     // memcpy(mapped, image_pixel_data[i], image_size);
        //     // vkUnmapMemory(p_device, staging_buffer.device_memory);
        // }
        
        // 4. Load single command buffer to copy all 6 images

        // 5. Offloading all 6 images to the device_queue



            // Order of Operation for the faces

            // [per-face filename] => [staging buffer] => [image] -> image_layout_transition => copy operation => transfer to color attachment layout
        
        
        // --------------------------------------
        // END COMMAND BUFFER RECORDING
        // --------------------------------------
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
    }

    void skybox_texture::destroy() {
        // m_skybox_image.destroy();
        for(auto& image : m_cubemap_images) {
            image.destroy();
        }
    }

};