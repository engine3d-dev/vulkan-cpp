#pragma once
#include <filesystem>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vulkan-cpp/sample_image.hpp>

namespace vk {
    struct texture_info {
        // for getting image memory requirements for the texture
        VkPhysicalDevice physical;
        std::filesystem::path filepath;
    };
    class texture {
    public:
        texture() = default;
        texture(const VkDevice& p_device, const texture_info& p_filepath);

        [[nodiscard]] bool loaded() const { return m_texture_loaded; }

        // [[nodiscard]] sampled_image data() const { return m_image_handle; }
        [[nodiscard]] sample_image image() const { return m_image; }

        [[nodiscard]] uint32_t width() const {return m_width; }

        [[nodiscard]] uint32_t height() const {return m_height; }

        void destroy();
    private:
        VkDevice m_device=nullptr;
        bool m_texture_loaded=false;
        // sampled_image m_image_handle{};
        sample_image m_image{};
        uint32_t m_width=0;
        uint32_t m_height=0;
    };

};