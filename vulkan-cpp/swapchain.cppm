module;

#include <vulkan/vulkan.h>
#include <print>
#include <span>
#include <vector>

export module vk:swapchain;

export import :types;
export import :utilities;
export import :device_queue;

export namespace vk {
    inline namespace v1 {
        class swapchain {
        public:
            swapchain(const VkDevice& p_device,
                    const VkSurfaceKHR& p_surface,
                    const swapchain_params& p_settings,
                    const surface_params& p_surface_properties) : m_device(p_device), m_surface_handler(p_surface), m_surface_params(p_surface_properties) {
                m_image_size = surface_image_size(m_surface_params.capabilities);

                std::println("Surface Image Size = {}", m_image_size);

                create(p_settings);
            }

            void create(const swapchain_params& p_settings) {
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
                    .presentMode = static_cast<VkPresentModeKHR>(p_settings.present_mode),
                    .clipped = true
                };

                vk_check(vkCreateSwapchainKHR(
                        m_device, &swapchain_ci, nullptr, &m_swapchain_handler),
                        "vkCreateSwapchainKHR");
            }
            

            /**
             * @brief gets the presentable images from this associated swapchain
             * 
             * @return std::span<const VkImage> which are the presentable available images that can be presented to the swapchain
             * 
             * ```C++
             * 
             * vk::swapchain main_swapchain(logical_device, ...);
             * 
             * std::span<const VkImage> images = main_swapchain.enumerate_images();
             * 
             * ```
             * 
            */
            std::span<const VkImage> presentable_images() {
                uint32_t image_count=0;
                vkGetSwapchainImagesKHR(m_device, m_swapchain_handler, &image_count, nullptr);

                m_images.resize(image_count);
                vkGetSwapchainImagesKHR(m_device, m_swapchain_handler, &image_count, m_images.data());

                return m_images;
            }

            void destroy() {
                vkDestroySwapchainKHR(m_device, m_swapchain_handler, nullptr);
            }

            operator VkSwapchainKHR() const { return m_swapchain_handler; }

            operator VkSwapchainKHR() { return m_swapchain_handler; }

        private:
            VkDevice m_device = nullptr;
            VkSwapchainKHR m_swapchain_handler = nullptr;
            VkSurfaceKHR m_surface_handler = nullptr;
            surface_params m_surface_params{};
            uint32_t m_image_size = 0;

            device_queue m_present_queue;

            std::vector<VkImage> m_images;
        };
    };
};