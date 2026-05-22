module;

#include <vulkan/vulkan.h>
#include <span>
#include <vector>
#include <algorithm>

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
                      const surface_params& p_surface_properties)
              : m_device(p_device)
              , m_surface_handler(p_surface) {

                construct(p_settings, p_surface_properties);
            }

            void construct(const swapchain_params& p_settings,
                           const surface_params& p_surface_properties) {

                VkSwapchainCreateInfoKHR swapchain_ci = {
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .pNext = nullptr,
                    .surface = m_surface_handler,
                    .minImageCount = p_surface_properties.image_size,
                    .imageFormat = p_surface_properties.format.format,
                    .imageColorSpace = p_surface_properties.format.colorSpace,
                    // use physical device surface formats to getting the right
                    // formats in vulkan
                    .imageExtent =
                      p_surface_properties.capabilities.currentExtent,
                    .imageArrayLayers = 1,

                    // Remove COLOR_ATTACHMENT flag because its not needed
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .preTransform =
                      p_surface_properties.capabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode =
                      static_cast<VkPresentModeKHR>(p_settings.present_mode),
                    .clipped = p_settings.clipped,
                };
                VkResult res = vkCreateSwapchainKHR(
                  m_device, &swapchain_ci, nullptr, &m_swapchain_handler);

                vk_check(res, "vkCreateSwapchainKHR");
            }

            /**
             * @brief gets the presentable images from this associated swapchain
             *
             * @return std::span<const VkImage> which are the presentable
             * available images that can be presented to the swapchain
             *
             * ```C++
             *
             * vk::swapchain main_swapchain(logical_device, ...);
             *
             * std::span<const VkImage> images = main_swapchain.get_images();
             *
             * ```
             * TODO: Have this return std::span<const vk::sample_image>
             *
             */
            std::span<const VkImage> get_images() {
                uint32_t image_count = 0;
                vkGetSwapchainImagesKHR(
                  m_device, m_swapchain_handler, &image_count, nullptr);

                m_images.resize(image_count);
                vkGetSwapchainImagesKHR(
                  m_device, m_swapchain_handler, &image_count, m_images.data());

                return m_images;
            }

            void destroy() {
                vkDestroySwapchainKHR(m_device, m_swapchain_handler, nullptr);
            }

            [[nodiscard]] bool alive() const { return m_swapchain_handler; }

            operator VkSwapchainKHR() const { return m_swapchain_handler; }

            operator VkSwapchainKHR() { return m_swapchain_handler; }

        private:
            VkDevice m_device = nullptr;
            VkSwapchainKHR m_swapchain_handler = nullptr;
            VkSurfaceKHR m_surface_handler = nullptr;
            std::vector<VkImage> m_images;
        };
    };
};