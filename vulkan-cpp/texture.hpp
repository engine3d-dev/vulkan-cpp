#pragma once
#include <filesystem>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vulkan-cpp/buffer.hpp>
#include <vulkan-cpp/sample_image.hpp>

namespace vk {
    struct texture_info {
        // for getting image memory requirements for the texture
        VkPhysicalDeviceMemoryProperties phsyical_memory_properties;
        std::filesystem::path filepath;
    };

    struct texture_extent {
        uint32_t width=0;
        uint32_t height=0;
    };

    class texture {
    public:
        texture() = default;
        texture(const VkDevice& p_device, const image_extent& p_extent, VkPhysicalDeviceMemoryProperties p_property);
        texture(const VkDevice& p_device, const texture_info& p_filepath);

        [[nodiscard]] bool loaded() const { return m_texture_loaded; }

        [[nodiscard]] sample_image image() const { return m_image; }

        [[nodiscard]] uint32_t width() const { return m_width; }

        [[nodiscard]] uint32_t height() const { return m_height; }

        void destroy();

    private:
        VkDevice m_device = nullptr;
        bool m_texture_loaded = false;
        // sampled_image m_image_handle{};
        sample_image m_image{};
        uint32_t m_width = 0;
        uint32_t m_height = 0;
    };

};