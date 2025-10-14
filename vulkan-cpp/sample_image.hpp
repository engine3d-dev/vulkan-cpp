#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {

    class sample_image {
    public:
        sample_image() = default;
        sample_image(const VkDevice& p_device,
                     const image_configuration_information& p_image_properties);
        sample_image(const VkDevice& p_device,
                     const VkImage& p_image,
                     const image_configuration_information& p_image_properties);

        [[nodiscard]] VkSampler sampler() const { return m_sampler; }

        [[nodiscard]] VkImageView image_view() const { return m_image_view; }

        void destroy();

        operator VkImage() const { return m_image; }

        operator VkImage() { return m_image; }

    private:
        bool m_only_destroy_image_view = false;
        VkDevice m_device = nullptr;
        VkImage m_image = nullptr;
        VkImageView m_image_view = nullptr;
        VkSampler m_sampler = nullptr;
        VkDeviceMemory m_device_memory = nullptr;
    };
};