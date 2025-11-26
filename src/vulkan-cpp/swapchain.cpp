#include <vulkan-cpp/imports.hpp>
#include <vulkan-cpp/swapchain.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <print>

namespace vk {
    swapchain::swapchain(const VkDevice& p_device,
                         const VkSurfaceKHR& p_surface,
                         const swapchain_enumeration& p_settings,
                         const surface_params& p_surface_properties)
      : m_device(p_device)
      , m_surface_handler(p_surface)
      , m_surface_params(p_surface_properties) {

        m_image_size = surface_image_size(m_surface_params.capabilities);

        std::println("Surface Image Size = {}", m_image_size);

        create(p_settings);
    }

    void swapchain::create(const swapchain_enumeration& p_settings) {
        VkSwapchainCreateInfoKHR swapchain_ci = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_surface_handler,
            .minImageCount = m_image_size,
            .imageFormat = m_surface_params.format.format,
            .imageColorSpace = m_surface_params.format.colorSpace,
            // use physical device surface formats to getting the right formats
            // in vulkan
            .imageExtent = m_surface_params.capabilities.currentExtent,
            .imageArrayLayers = 1,
            .imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT),
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &p_settings.present_index,
            .preTransform = m_surface_params.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
            .clipped = true
        };

        vk_check(vkCreateSwapchainKHR(
                   m_device, &swapchain_ci, nullptr, &m_swapchain_handler),
                 "vkCreateSwapchainKHR");
    }

    std::span<const VkImage> swapchain::presentable_images() {
        uint32_t image_count=0;
        vkGetSwapchainImagesKHR(m_device, m_swapchain_handler, &image_count, nullptr);

        m_images.resize(image_count);
        vkGetSwapchainImagesKHR(m_device, m_swapchain_handler, &image_count, m_images.data());

        return m_images;
    }

    void swapchain::destroy() {
        vkDestroySwapchainKHR(m_device, m_swapchain_handler, nullptr);
    }
};