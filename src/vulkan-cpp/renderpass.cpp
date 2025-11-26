#include <vulkan-cpp/renderpass.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <print>

namespace vk {

    renderpass::renderpass(const VkDevice& p_device,
                           std::span<attachment> p_renderpass_attachments,
                           bool p_enable_subpasses)
      : m_device(p_device) {

        create(p_renderpass_attachments, p_enable_subpasses);
    }

    void renderpass::create(
      std::span<const attachment> p_renderpass_attachments,
      bool p_enable_subpasses) {
        // 1. Specifically for setting up the attachment description
        std::vector<VkAttachmentDescription> attachment_description(
          p_renderpass_attachments.size());

        // color attachment reference slots to set to help the
        // VkAttachmentReference know which color attachment they correspond to
        std::vector<uint32_t> color_attachment_indices;

        // depth attachment reference slots to set to help the
        // VkAttachmentReference know which depth attachment they correspond to
        std::vector<uint32_t> depth_attachment_indices;

        for (size_t i = 0; i < attachment_description.size(); i++) {
            attachment attachment_spec = p_renderpass_attachments[i];
            attachment_description[i] = {
                .flags = 0,
                .format = attachment_spec.format,
                // .samples = VK_SAMPLE_COUNT_1_BIT,
                .samples = static_cast<VkSampleCountFlagBits>(attachment_spec.samples),
                // .loadOp = to_attachment_load(attachment_spec.load),
				.loadOp = static_cast<VkAttachmentLoadOp>(attachment_spec.load),
                // .storeOp = to_attachment_store(attachment_spec.store),
				.storeOp = static_cast<VkAttachmentStoreOp>(attachment_spec.store),
                .stencilLoadOp =
                  static_cast<VkAttachmentLoadOp>(attachment_spec.stencil_load),
                .stencilStoreOp =
                  static_cast<VkAttachmentStoreOp>(attachment_spec.stencil_store),
                .initialLayout = static_cast<VkImageLayout>(attachment_spec.initial_layout),
                .finalLayout = static_cast<VkImageLayout>(attachment_spec.final_layout)
            };

            // I do a check here to save the slots for specifying the
            // VkAttachmentReference Since .attachment is the slot index for
            // corresponding which attachment layout is to which
            if (has_depth_specified(attachment_spec.layout)) {
                depth_attachment_indices.emplace_back(i);
            }
            else {
                color_attachment_indices.emplace_back(i);
            }
        }

        // 2. Setting up the color attachment reference to specifying specific
        // attachments they correspond to (using the indices)
        std::vector<VkAttachmentReference> color_attachment_references(
          color_attachment_indices.size());

        for (size_t i = 0; i < color_attachment_indices.size(); i++) {
            uint32_t slot = color_attachment_indices[i];
            color_attachment_references[i] = {
                .attachment = slot,
                // .layout = to_image_layout(p_renderpass_attachments[slot].layout)
				.layout = static_cast<VkImageLayout>(p_renderpass_attachments[slot].layout)
            };
        }

        // 3. Setting up the depth attachment reference to specifying specific
        // attachments they correspond to (using the indices)
        std::vector<VkAttachmentReference> depth_attachment_references(
          depth_attachment_indices.size());
        for (size_t i = 0; i < depth_attachment_indices.size(); i++) {
            uint32_t slot = depth_attachment_indices[i];
            depth_attachment_references[i] = {
                .attachment = slot,
                // .layout = to_image_layout(p_renderpass_attachments[slot].layout)
				.layout = static_cast<VkImageLayout>(p_renderpass_attachments[slot].layout)
            };
        }

        // 4. Setting up subpass descriptions that may/may not be applied to
        // this renderpass based on the p_enable_subpass is set to true
        // TODO: VkSubpassDescription is deprecated in vulkan 1.2+, we should
        // change to VkSubpassDescription2
        // TODO: Change from VkRenderPassCreateInfo to VkRenderPassCreateInfo2
        // since they have different parameter modifications
        VkSubpassDescription subpass_description = {
            .flags = 0,
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount =
              static_cast<uint32_t>(color_attachment_references.size()),
            .pColorAttachments = color_attachment_references.data(),
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = depth_attachment_references.data(),
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr
        };

        std::array<VkSubpassDescription, 1> subpasses;

        if (p_enable_subpasses) {
            subpasses = { subpass_description };
        }

        VkRenderPassCreateInfo renderpass_ci = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount =
              static_cast<uint32_t>(attachment_description.size()),
            .pAttachments = attachment_description.data(),
            .subpassCount = static_cast<uint32_t>(subpasses.size()),
            .pSubpasses = subpasses.data(),
            .dependencyCount = 0,
            .pDependencies = nullptr
        };

        vk_check(
          vkCreateRenderPass(m_device, &renderpass_ci, nullptr, &m_renderpass),
          "vkCreateRenderPass");
    }

    void renderpass::begin(const renderpass_begin_params& p_begin_info) {
        // TODO: Move VkViewport and VkScissor to vk::swapchain since these are
        // information more closely set by the swapchain
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
        VkClearColorValue renderpass_color = { { p_begin_info.color.at(0),
                                                 p_begin_info.color.at(1),
                                                 p_begin_info.color.at(2),
                                                 p_begin_info.color.at(3) } };
        std::array<VkClearValue, 2> clear_values = {};

        clear_values[0].color = renderpass_color;
        clear_values[1].depthStencil = { 1.f, 0 };

        VkRenderPassBeginInfo renderpass_begin_params = {
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

        vkCmdBeginRenderPass(p_begin_info.current_command,
                             &renderpass_begin_params,
                             static_cast<VkSubpassContents>(p_begin_info.subpass));
    }

    void renderpass::end(const VkCommandBuffer& p_current) {
        vkCmdEndRenderPass(p_current);
    }

    void renderpass::destroy() {
        vkDestroyRenderPass(m_device, m_renderpass, nullptr);
    }

};