#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {
    struct image_extent {
        uint32_t width=1;
        uint32_t height=1;
    };

    struct image_configuration_information {
        image_extent extent;
        VkFormat format;
        memory_property property=memory_property::device_local_bit;
        image_aspect_flags aspect=image_aspect_flags::color_bit;
        VkImageUsageFlags usage;
        uint32_t mip_levels=1;
        uint32_t layer_count=1;
        uint32_t array_layers=1;
        VkPhysicalDevice physical_device=nullptr;
        filter_range range{.min = VK_FILTER_LINEAR, .max = VK_FILTER_LINEAR};
        VkSamplerAddressMode addrses_mode_u=VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addrses_mode_v=VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addrses_mode_w=VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };

    class sample_image {
    public:
        sample_image() = default;
        sample_image(const VkDevice& p_device, const image_configuration_information& p_image_properties);
        sample_image(const VkDevice& p_device, const VkImage& p_image, const image_configuration_information& p_image_properties);

        [[nodiscard]] VkSampler sampler() const { return m_sampler; }

        [[nodiscard]] VkImageView image_view() const { return m_image_view; }

        void destroy();

        operator VkImage() const { return m_image; }

        operator VkImage() { return m_image; }

    private:
        VkDevice m_device=nullptr;
        VkImage m_image=nullptr;
        VkImageView m_image_view=nullptr;
        VkSampler m_sampler=nullptr;
        VkDeviceMemory m_device_memory=nullptr;
    };
};