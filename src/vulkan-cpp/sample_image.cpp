#include <vulkan-cpp/sample_image.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <stdexcept>
#include <print>

namespace vk {
    sample_image::sample_image(const VkDevice& p_device, const image_configuration_information& p_image_properties) : m_device(p_device) {

        // 1. creating VkImage handle
        VkImageCreateInfo image_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = p_image_properties.format,
            .extent = { .width = p_image_properties.extent.width, .height = p_image_properties.extent.height, .depth = 1 },
            .mipLevels = p_image_properties.mip_levels,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = p_image_properties.usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };

        vk_check(vkCreateImage(p_device, &image_ci, nullptr, &m_image),
                 "vkCreateImage");

        // 2. get image memory requirements from physical device

        if(p_image_properties.physical_device == nullptr) {
            throw std::runtime_error("sample_image tried to invoke physical device failed!!");
        }
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(p_device, m_image, &memory_requirements);
        uint32_t memory_type_index = vk::image_memory_requirements(p_image_properties.physical_device, p_device, m_image);



        // 4. Allocate info
        VkMemoryAllocateInfo memory_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex = memory_type_index
        };

        vk_check(vkAllocateMemory(
                   p_device, &memory_alloc_info, nullptr, &m_device_memory),
                 "vkAllocateMemory");

        // 5. bind image memory
        vk_check(vkBindImageMemory(p_device, m_image, m_device_memory, 0),
                 "vkBindImageMemory");
        
        // Needs to create VkImageView after VkImage
        // because VkImageView expects a VkImage to be binded to a singl VkDeviceMemory beforehand
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_image_properties.format,
            .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .aspectMask = to_image_aspect_flags(p_image_properties.aspect),
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = p_image_properties.layer_count },
        };

        vk_check(vkCreateImageView(p_device, &image_view_ci, nullptr, &m_image_view),"vkCreateImage");

        // Create VkSampler handler
        VkSamplerCreateInfo sampler_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = p_image_properties.range.min,
            .minFilter = p_image_properties.range.max,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = p_image_properties.addrses_mode_u,
            .addressModeV = p_image_properties.addrses_mode_v,
            .addressModeW = p_image_properties.addrses_mode_w,
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

        vk_check(vkCreateSampler(p_device, &sampler_info, nullptr, &m_sampler), "vkCreateSampler");
    }

    sample_image::sample_image(const VkDevice& p_device, const VkImage& p_image, const image_configuration_information& p_image_properties) : m_device(p_device), m_image(p_image) {
        
        if(m_image == nullptr) {
            std::println("VkImage is nullptr");
        }
        else {
            std::println("VkImage not nullptr");
        }
        // Needs to create VkImageView after VkImage
        // because VkImageView expects a VkImage to be binded to a singl VkDeviceMemory beforehand
        VkImageViewCreateInfo image_view_ci = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_image_properties.format,
            .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .aspectMask = to_image_aspect_flags(p_image_properties.aspect),
                                  .baseMipLevel = 0,
                                  .levelCount = p_image_properties.mip_levels,
                                  .baseArrayLayer = 0,
                                  .layerCount = p_image_properties.layer_count },
        };

        vk_check(vkCreateImageView(p_device, &image_view_ci, nullptr, &m_image_view),"vkCreateImage");

        // Create VkSampler handler
        VkSamplerCreateInfo sampler_info = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = p_image_properties.range.min,
            .minFilter = p_image_properties.range.max,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = p_image_properties.addrses_mode_u,
            .addressModeV = p_image_properties.addrses_mode_v,
            .addressModeW = p_image_properties.addrses_mode_w,
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

        vk_check(vkCreateSampler(p_device, &sampler_info, nullptr, &m_sampler), "vkCreateSampler");

        m_only_destroy_image_view = true;
    }

    void sample_image::destroy() {
        if (m_image_view != nullptr) {
            vkDestroyImageView(m_device, m_image_view, nullptr);
        }

        // Boolean check is to make sure we might only want
        // to destroy vk::sample_image resources.
        
        // Example of this is the swapchain may pass in
        // its images and we should only destruct the VkImageView
        // and not the swapchain's images directly
        if (m_image != nullptr and !m_only_destroy_image_view) {
            vkDestroyImage(m_device, m_image, nullptr);
        }
        
        if (m_sampler != nullptr) {
            vkDestroySampler(m_device, m_sampler, nullptr);
        }

        if (m_device_memory != nullptr) {
            vkFreeMemory(m_device, m_device_memory, nullptr);
        }
    }
};