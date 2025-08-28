#pragma once
#include <vulkan-cpp/types.hpp>
#include <string>
#include <array>
#include <vulkan/vulkan.h>
#include <vulkan-cpp/sample_image.hpp>

namespace vk {
    struct skybox_texture_info {
        VkPhysicalDevice physical_handle=nullptr;
        const std::array<std::string, 6>& faces;
    };
    class skybox_texture {
    public:
        skybox_texture(const VkDevice& p_device, const skybox_texture_info& p_config);

        [[nodiscard]] bool loaded() const { return m_is_loaded; }

        void destroy();

    private:
        VkDevice m_device=nullptr;
        bool m_is_loaded=false;

        std::array<sample_image, 6> m_cubemap_images{};
        int m_width=0;
        int m_height=0;
        sample_image m_skybox_image;
        // sample_image m_cube_image;
    };
};