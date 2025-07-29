#include <vulkan-cpp/renderpass.hpp>
#include <vulkan-cpp/utilities.hpp>

namespace vk {
    renderpass::renderpass(const VkDevice& p_device, const renderpass_attachments& p_attachemnts) : m_device(p_device) {
        create(p_attachemnts);
    }

    void renderpass::create(const renderpass_attachments& p_attachemnts) {
        VkRenderPassCreateInfo renderpass_ci = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount =
              static_cast<uint32_t>(p_attachemnts.attachments.size()),
            .pAttachments = p_attachemnts.attachments.data(),
            .subpassCount = static_cast<uint32_t>(
              p_attachemnts.subpass_descriptions.size()),
            .pSubpasses = p_attachemnts.subpass_descriptions.data(),
            .dependencyCount =
              static_cast<uint32_t>(p_attachemnts.dependencies.size()),
            .pDependencies = p_attachemnts.dependencies.data()
        };

        vk_check(vkCreateRenderPass(m_device, &renderpass_ci, nullptr, &m_renderpass), "vkCreateRenderPass");
    }

    void renderpass::destroy() {
        vkDestroyRenderPass(m_device,m_renderpass, nullptr);
    }


};