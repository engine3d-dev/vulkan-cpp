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

	void renderpass::begin(const renderpass_begin_info& p_begin_info) {
		VkViewport viewport = {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(p_begin_info.extent.width),
            .height = static_cast<float>(p_begin_info.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };

        vkCmdSetViewport(p_begin_info.current_command, 0, 1, &viewport);

        VkRect2D scissor = {
            .offset = { 0, 0 },
            .extent = { p_begin_info.extent.width, p_begin_info.extent.height },
        };

        vkCmdSetScissor(p_begin_info.current_command, 0, 1, &scissor);

		// setting color for this specific renderpass
		VkClearColorValue renderpass_color = {
			{ p_begin_info.color.at(0), p_begin_info.color.at(1), p_begin_info.color.at(2), p_begin_info.color.at(3) }
		};
		std::array<VkClearValue, 2> clear_values = {};

        clear_values[0].color = renderpass_color;
        clear_values[1].depthStencil = { 1.f, 0 };

		VkRenderPassBeginInfo renderpass_begin_info = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = m_renderpass,
			.framebuffer = p_begin_info.current_framebuffer,
			.renderArea = {
				.offset = {
					.x = 0,
					.y = 0
				},
				.extent = {
					.width = p_begin_info.extent.width,
					.height = p_begin_info.extent.height
				},
			},
			.clearValueCount = static_cast<uint32_t>(clear_values.size()),
			.pClearValues = clear_values.data(),
        };

		vkCmdBeginRenderPass(p_begin_info.current_command, &renderpass_begin_info, to_subpass_contents(p_begin_info.subpass));
	}

	void renderpass::end(const VkCommandBuffer& p_current) {
		vkCmdEndRenderPass(p_current);
	}

    void renderpass::destroy() {
        vkDestroyRenderPass(m_device,m_renderpass, nullptr);
    }


};