module;

#include <vulkan/vulkan.h>
#include <print>
#include <span>
#include <vector>

export module vk:renderpass;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        /**
         * @name renderpass
         *
         * @param p_device is the logical device to associate the creation of
         * renderpasses
         * @param p_renderpass_attachment is a vk::attachment to specify the
         * individual attachment operation that handle in creating
         * VkAttachmentDescription, VkAttachmentReference, and
         * VkSubpassDescription
         *
         * @param p_enable_subpass because subpasses are optional, this is a
         * boolean to enable if we want to apply subpasses
         */
        class renderpass {
        public:
            renderpass() = default;
            renderpass(const VkDevice& p_device,
                       std::span<attachment> p_renderpass_attachments,
                       bool p_enable_subpasses = true)
              : m_device(p_device) {
                configure(p_renderpass_attachments, p_enable_subpasses);
            }

            /**
             * @brief Condifures the renderpass pipeline. By defining what kinds
             * of image handles will be used and how their memory should be
             * treated.
             *
             * Which includes how to treat these images for the particular tasks
             * that can be done on those images.
             *
             * @param p_renderpass_attachments
             * - .format: Image bit-depth
             * - .load/store: Configuring for loading/storing pixels
             * - .initial_layout: stage the image starts in.
             * - .final_layout: stage the image is left for the next task.
             *
             * @brief Additional Considerations:
             * - Framebuffers must be synced with the configuration of this
             * renderpass handle when constructed.
             * - .initial_layout and .final_layout must be valid for the image
             * type specified.
             * - Attachment references in subpass must point to a valid indices
             * within the p_renderpass_attachment parameters
             *
             * +---------------------+               +-------------------------+
             * | Attch 0: RGBA8 Color|--- [Ref] ---> | .pColorAttachments      |
             * | Attch 1: D32 Depth  |--- [Ref] ---> | .pDepthStencilAttachment|
             * +---------------------+               +-------------------------+
             * | Load: Clear         |               |                         |
             * | Store: Store        |               |                         |
             * +---------------------+               +-------------------------+
             * |                                    |
             * \___________________________________/
             * |
             * V
             * [ Configure RenderPass Handle]
             */
            void configure(std::span<const attachment> p_renderpass_attachments,
                           bool p_enable_subpasses = true) {
                // 1. Specifically for setting up the attachment description
                std::vector<VkAttachmentDescription> attachment_description(
                  p_renderpass_attachments.size());

                // color attachment reference slots to set to help the
                // VkAttachmentReference know which color attachment they
                // correspond to
                std::vector<uint32_t> color_attachment_indices;

                // depth attachment reference slots to set to help the
                // VkAttachmentReference know which depth attachment they
                // correspond to
                std::vector<uint32_t> depth_attachment_indices;

                for (size_t i = 0; i < attachment_description.size(); i++) {
                    attachment attachment_spec = p_renderpass_attachments[i];
                    attachment_description[i] = {
                        .flags = 0,
                        .format = attachment_spec.format,
                        .samples = static_cast<VkSampleCountFlagBits>(
                          attachment_spec.samples),
                        .loadOp =
                          static_cast<VkAttachmentLoadOp>(attachment_spec.load),
                        .storeOp = static_cast<VkAttachmentStoreOp>(
                          attachment_spec.store),
                        .stencilLoadOp = static_cast<VkAttachmentLoadOp>(
                          attachment_spec.stencil_load),
                        .stencilStoreOp = static_cast<VkAttachmentStoreOp>(
                          attachment_spec.stencil_store),
                        .initialLayout = static_cast<VkImageLayout>(
                          attachment_spec.initial_layout),
                        .finalLayout = static_cast<VkImageLayout>(
                          attachment_spec.final_layout)
                    };

                    // I do a check here to save the slots for specifying the
                    // VkAttachmentReference Since .attachment is the slot index
                    // for corresponding which attachment layout is to which
                    if (has_depth_specified(attachment_spec.layout)) {
                        depth_attachment_indices.emplace_back(i);
                    }
                    else {
                        color_attachment_indices.emplace_back(i);
                    }
                }

                // 2. Setting up the color attachment reference to specifying
                // specific attachments they correspond to (using the indices)
                std::vector<VkAttachmentReference> color_attachment_references(
                  color_attachment_indices.size());

                for (size_t i = 0; i < color_attachment_indices.size(); i++) {
                    uint32_t slot = color_attachment_indices[i];
                    color_attachment_references[i] = {
                        .attachment = slot,
                        // .layout =
                        // to_image_layout(p_renderpass_attachments[slot].layout)
                        .layout = static_cast<VkImageLayout>(
                          p_renderpass_attachments[slot].layout)
                    };
                }

                // 3. Setting up the depth attachment reference to specifying
                // specific attachments they correspond to (using the indices)
                std::vector<VkAttachmentReference> depth_attachment_references(
                  depth_attachment_indices.size());
                for (size_t i = 0; i < depth_attachment_indices.size(); i++) {
                    uint32_t slot = depth_attachment_indices[i];
                    depth_attachment_references[i] = {
                        .attachment = slot,
                        .layout = static_cast<VkImageLayout>(
                          p_renderpass_attachments[slot].layout)
                    };
                }

                // 4. Setting up subpass descriptions that may/may not be
                // applied to this renderpass based on the p_enable_subpass is
                // set to true
                // TODO: VkSubpassDescription is deprecated in vulkan 1.2+, we
                // should change to VkSubpassDescription2
                // TODO: Change from VkRenderPassCreateInfo to
                // VkRenderPassCreateInfo2 since they have different parameter
                // modifications
                VkSubpassDescription subpass_description = {
                    .flags = 0,
                    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .inputAttachmentCount = 0,
                    .pInputAttachments = nullptr,
                    .colorAttachmentCount =
                      static_cast<uint32_t>(color_attachment_references.size()),
                    .pColorAttachments = color_attachment_references.data(),
                    .pResolveAttachments = nullptr,
                    .pDepthStencilAttachment =
                      depth_attachment_references.data(),
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

                vk_check(vkCreateRenderPass(
                           m_device, &renderpass_ci, nullptr, &m_renderpass),
                         "vkCreateRenderPass");
            }

            /**
             * @brief checks if the renderpass handle is valid.
             */
            [[nodiscard]] bool alive() const { return m_renderpass; }

            /**
             * @brief Initiates starting state for the renderpass to begin the
             * rendering operation.
             *
             * Implicitly clears the screen to specific color/depth and sets up
             * the render area (viewport, scissor) for geometry are mapped to
             * the pixels correctly.
             *
             * @param p_command is the command buffer to start recording
             rendering operations with.
             * @param p_begin_info is the specifications for configuring the
             rendering operation
             *
             * @brief Additional Considerations:
             * - Requires valid framebuffer compatible to the renderpass.
             * - Requires to be in recording state of a command buffer to
             * perform this operation.
             * - If framebuffer image is used by the swapchain, it must be
             * acquired before invoking this operation.
             *
             * [ CPU-SIDE Recording ]                  [ GPU-SIDE Executing ]
             * +---------------------+                +---------------------+
             * | renderpass::begin   |                |   Clear/Load Images |
             * |    - Load Operation |                |                     |
             * |    - Render Area    |   (Submit)     | Subpass Execute Cmd |
             * |   [draw commands]   | ============>  |                     |
             * |  [viewport/scissor] |                |   Resolve/Store     |
             * | renderpass::end     |                |   (Store to VRAM)   |
             * +---------------------+                +---------------------+
             *
             */
            void begin(const VkCommandBuffer& p_command,
                       const renderpass_begin_params& p_begin_info) {
                VkViewport viewport = {
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = static_cast<float>(p_begin_info.extent.width),
                    .height = static_cast<float>(p_begin_info.extent.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
                };

                vkCmdSetViewport(p_command, 0, 1, &viewport);

                VkRect2D scissor = {
                    .offset = { 0, 0 },
                    .extent = p_begin_info.extent,
                };

                vkCmdSetScissor(p_command, 0, 1, &scissor);

                // setting color for this specific renderpass
                VkClearColorValue renderpass_color = {};
                std::copy(p_begin_info.color.begin(),
                          p_begin_info.color.end(),
                          renderpass_color.float32);

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

                vkCmdBeginRenderPass(
                  p_command,
                  &renderpass_begin_params,
                  static_cast<VkSubpassContents>(p_begin_info.subpass));
            }

            /**
             * @brief Specified when to stop performing rendering operation.
             *
             * This is for flushing to move to its final layout. Readying to be
             * rendered to the screen or the shader to be read by.
             *
             * @brief Additional Consideration when calling this API
             * - Must follow-up with renderpass::begin with the same command
             * buffer.
             * - Cannot record any more draw operations when this operation is
             * invoked.
             * - Image will auitomatically be moved to its .final_layout
             * transition.
             *
             * @param p_current is the command buffer to stop recording drawing
             * commands to.
             */
            void end(const VkCommandBuffer& p_current) {
                vkCmdEndRenderPass(p_current);
            }

            void destroy() {
                vkDestroyRenderPass(m_device, m_renderpass, nullptr);
            }

            operator VkRenderPass() const { return m_renderpass; }

            operator VkRenderPass() { return m_renderpass; }

        private:
            VkDevice m_device = nullptr;
            VkRenderPass m_renderpass = nullptr;
        };
    };
};