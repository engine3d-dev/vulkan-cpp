#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vector>
#include <vulkan-cpp/device_queue.hpp>

namespace vk {

    class swapchain {
    public:
        swapchain(const VkDevice& p_device, const VkSurfaceKHR& p_surface, const swapchain_enumeration& p_settings, const surface_enumeration& p_surface_properties);

        void create(const swapchain_enumeration& p_settings);

        // VkFramebuffer active_framebuffer(uint32_t p_index);

        void destroy();

        operator VkSwapchainKHR() const { return m_swapchain_handler; }

        operator VkSwapchainKHR() { return m_swapchain_handler; }

    private:
        VkDevice m_device=nullptr;
        VkSwapchainKHR m_swapchain_handler=nullptr;
        VkSurfaceKHR m_surface_handler=nullptr;
        surface_enumeration m_surface_enumeration{};
        uint32_t m_image_size=0;

        device_queue m_present_queue;

        // std::vector<VkFramebuffer> m_swapchain_framebuffers{};
    };
};