#include <vulkan-cpp/framebuffer.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {
    framebuffer::framebuffer(const VkDevice& p_device,
                             const framebuffer_settings& p_setting)
      : m_device(p_device) {
        VkFramebufferCreateInfo framebuffer_ci = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = p_setting.renderpass,
            .attachmentCount = static_cast<uint32_t>(p_setting.views.size()),
            .pAttachments = p_setting.views.data(),
            .width = p_setting.extent.width,
            .height = p_setting.extent.height,
            .layers = 1
        };

        vk::vk_check(vkCreateFramebuffer(
                       m_device, &framebuffer_ci, nullptr, &m_framebuffer),
                     "vkCreateFramebuffer");
    }

    void framebuffer::destroy() {
        if (m_framebuffer != nullptr) {
            vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
        }
    }
};