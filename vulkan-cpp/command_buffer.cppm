module;

#include <span>
#include <vulkan/vulkan.h>
#include <vector>
#include <print>

export module vk:command_buffer;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
        struct command_inherit_info {
            VkRenderPass renderpass = nullptr;
            uint32_t subpass_index = 0;
            VkFramebuffer framebuffer = nullptr;
        };

        /**
         * @brief vk::command_buffer is an abstraction around the
         * VkCommandBuffer.
         *
         * vk::command_buffer can be represented as a VkCommandBuffer
         * if the user decides to use the raw Vulkan API.
         *
         */
        class command_buffer {
        public:
            command_buffer() = default;
            command_buffer(const VkDevice& p_device,
                           const command_params& p_enumerate_command_info)
              : m_device(p_device) {
                VkCommandPoolCreateInfo pool_ci = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    // .flags =
                    // (VkCommandPoolCreateFlags)p_enumerate_command_info.pool_flag,
                    .flags = static_cast<VkCommandPoolCreateFlags>(
                      p_enumerate_command_info.flags),
                    .queueFamilyIndex = p_enumerate_command_info.queue_index
                };

                vk_check(vkCreateCommandPool(
                           m_device, &pool_ci, nullptr, &m_command_pool),
                         "vkCreateCommandPool");

                VkCommandBufferAllocateInfo command_buffer_alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .commandPool = m_command_pool,
                    .level = static_cast<VkCommandBufferLevel>(
                      p_enumerate_command_info.levels),
                    .commandBufferCount = 1
                };

                vk_check(vkAllocateCommandBuffers(m_device,
                                                  &command_buffer_alloc_info,
                                                  &m_command_buffer),
                         "vkAllocateCommandBuffers");
            }

            /**
             *
             * @brief Begin operation for GPU-specific work and where it is
             * defined. A common GPU workflow is queueing up tasks, in this case
             * commands.
             *
             * @param p_usage are flags to specify recording behavior
             * - ::one_time_submit: commands recorded once, then reset.
             * - ::simultaneous_use: Commands that can be re-submitted while
             * still running.
             *
             * @param p_inherit_info are used for command buffers specified as
             * secondary
             * - .renderpass: Inheriting renderpass handle from primary command.
             * - .framebuffer: Inherit the "image" target from primary command.
             *
             * The CPU cannot directly offload tasks to the GPU, therefore this
             * must be represented using command buffers which are queue'd up
             * during recording states of the command buffer.
             *
             * @brief Additional Considerations:
             * - Command buffer must be in 'initial' or 'executable' state
             * before calling .begin/end. You cannot call `begin()` on a buffer
             * that is already recording.
             * - If a command buffer was already recording, it must be reset
             * (manually or via
             * VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT).
             * - Every begin() call must follow-up with an end() call
             * afterwards.
             * - If using p_inherit_info, the command buffer must be created
             * with the 'secondary' level flag.
             *
             *
             * [ CPU Command ]                  [Record Stream]
             * +----------------------+         +------------------------+
             * |   .begin() <- start  |         |  [Header: Usage Flags] |
             * |   [Draw Triangle]    | =====>  |  [Op: Bind Pipeline]   |
             * |   [Copy Image]       |         |                        |
             * |   .end()             |         |  [Op: Draw Call]       |
             * |                      |         |  [Footer: End Tag]     |
             * +----------------------+         +------------------------+
             *
             * Example Usage:
             *
             * ```C++
             *
             * vk::command_buffer primary_command(logical_device, ...);
             *
             * primary_command.begin(vk::command_usage::one_time_submit);
             * // perform GPU-specific work
             * primary_command.end();
             *
             * ```
             *
             *
             */
            void begin(
              command_usage p_usage,
              std::span<const command_inherit_info> p_inherit_info = {}) {
                // Resets to zero if get called every frame
                if (m_begin_end_count == 2) {
                    m_begin_end_count = 0;
                }
                m_begin_end_count++;

                std::vector<VkCommandBufferInheritanceInfo> inheritance_infos(
                  p_inherit_info.size());

                for (size_t i = 0; i < inheritance_infos.size(); i++) {
                    inheritance_infos[i] = {
                        .sType =
                          VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                        .renderPass = p_inherit_info[i].renderpass,
                        .subpass = p_inherit_info[i].subpass_index,
                        .framebuffer = p_inherit_info[i].framebuffer
                    };
                }

                VkCommandBufferBeginInfo command_begin_info = {
                    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pNext = nullptr,
                    .flags = static_cast<VkCommandBufferUsageFlags>(p_usage)
                };
                vk_check(
                  vkBeginCommandBuffer(m_command_buffer, &command_begin_info),
                  "vkBeginCommandBuffer");
            }

            /**
             * @brief Stops the recording process, command moves from
             * 'recording' to 'executing' state. Which implies the command is
             * ready to be submitted to the vk::device_queue handle.
             *
             * @brief Additional Considerations:
             * - Cannot end a command buffer if a renderpass is still active.
             * (must invoked renderpass::end beforehand).
             * - If the command_buffer::end returns an error, command handle
             * becomes invalid and cannot be submitted.
             *
             */
            void end() {
                m_begin_end_count++;
                vkEndCommandBuffer(m_command_buffer);
            }

            /**
             * @brief Begin recording dynamic rendering pass instance
             *
             * Example Usage:
             * ```C++
             *
             * vk::command_buffer current_command = ...;
             *
             * std::array<vk::rendering_attachment, 1> color_attachments = {
             *      vk::rendering_attachment{
             *          .image_view = ...,
             *          .image_layout = ...,
             *          .resolve_mode = ...,
             *          .resolve_image_layout = ...,
             *          .load = ...,
             *          .store = ...,
             *          .clear_values = {...},
             *      }
             * };
             *
             * std::array<vk::rendering_attachment, 1> depth_attachments = {
             *      vk::rendering_attachment{
             *          .image_view = ...,
             *          .image_layout = ...,
             *          .resolve_mode = ...,
             *          .resolve_image_layout = ...,
             *          .load = ...,
             *          .store = ...,
             *          .clear_values = {...},
             *      }
             * };
             *
             * vk;:rendering_begin_parameters begin_params = {
             *      .render_area = {x, y},
             *      .color_attachments = color_attachments,
             *      .depth_attachments = depth_attachments,
             * };
             * current_command.begin_rendering(begin_params);
             *
             * current_command.end();
             *
             * ```
             *
             */
            void begin_rendering(const rendering_begin_parameters& p_parameters,
                                 subpass_contents p_subpass) {
                std::vector<VkRenderingAttachmentInfo> color_attachments(
                  p_parameters.color_attachments.size());
                // std::vector<VkRenderingAttachmentInfo> depth_attachments(
                //   p_parameters.depth_attachments.size());
                // std::vector<VkRenderingAttachmentInfo> stencil_attachments(
                //   p_parameters.stencil_attachments.size());

                std::println("color_attachments.size() = {}",
                             color_attachments.size());

                // Loading and setting color attachments (if any are set)
                for (size_t i = 0; i < color_attachments.size(); i++) {
                    rendering_attachment color_attach =
                      p_parameters.color_attachments[i];
                    color_attachments[i] = {
                        .sType =
                          VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                        .pNext = nullptr,
                        .imageView = color_attach.image_view,
                        .imageLayout =
                          static_cast<VkImageLayout>(color_attach.layout),
                        .resolveMode = static_cast<VkResolveModeFlagBits>(
                          color_attach.resolve_mode),
                        .resolveImageView = color_attach.resolve_image_view,
                        .resolveImageLayout = static_cast<VkImageLayout>(
                          color_attach.resolve_image_layout),
                        .loadOp =
                          static_cast<VkAttachmentLoadOp>(color_attach.load),
                        .storeOp =
                          static_cast<VkAttachmentStoreOp>(color_attach.store),
                        .clearValue = color_attach.clear_values,
                    };
                }

                // Loading and setting depth attachments (if any are set)
                rendering_attachment depth_attach =
                  p_parameters.depth_attachment;
                VkRenderingAttachmentInfo depth_attachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = depth_attach.image_view,
                    .imageLayout =
                      static_cast<VkImageLayout>(depth_attach.layout),
                    .resolveMode = static_cast<VkResolveModeFlagBits>(
                      depth_attach.resolve_mode),
                    .resolveImageView = depth_attach.resolve_image_view,
                    .resolveImageLayout = static_cast<VkImageLayout>(
                      depth_attach.resolve_image_layout),
                    .loadOp =
                      static_cast<VkAttachmentLoadOp>(depth_attach.load),
                    .storeOp =
                      static_cast<VkAttachmentStoreOp>(depth_attach.store),
                    .clearValue = depth_attach.depth_values,
                };

                // Loading and setting stencil attachments (if any are set)
                rendering_attachment stencil_attach =
                  p_parameters.stencil_attachment;
                VkRenderingAttachmentInfo stencil_attachment = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                    .pNext = nullptr,
                    .imageView = stencil_attach.image_view,
                    .imageLayout =
                      static_cast<VkImageLayout>(stencil_attach.layout),
                    .resolveMode = static_cast<VkResolveModeFlagBits>(
                      stencil_attach.resolve_mode),
                    .resolveImageView = stencil_attach.resolve_image_view,
                    .resolveImageLayout = static_cast<VkImageLayout>(
                      stencil_attach.resolve_image_layout),
                    .loadOp =
                      static_cast<VkAttachmentLoadOp>(stencil_attach.load),
                    .storeOp =
                      static_cast<VkAttachmentStoreOp>(stencil_attach.store),
                    .clearValue = stencil_attach.depth_values,
                };

                VkRenderingInfo rendering_begin_info = {
                    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                    .pNext = nullptr,
                    .flags = static_cast<VkRenderingFlags>(
                      p_parameters.rendering_flags),
                    .renderArea = p_parameters.render_area,
                    .layerCount = p_parameters.layer_count,
                    .viewMask = p_parameters.view_mask,
                    .colorAttachmentCount =
                      static_cast<uint32_t>(color_attachments.size()),
                    .pColorAttachments = color_attachments.data(),
                    .pDepthAttachment = &depth_attachment,
                    .pStencilAttachment = &stencil_attachment,
                };

                vkCmdBeginRendering(m_command_buffer, &rendering_begin_info);
            }

            /**
             * @brief End recording for dynamic rendering pass
             */
            void end_rendering() { vkCmdEndRendering(m_command_buffer); }

            /**
             * @brief Transferring the raw geomtric data of vertices to the
             * vertex input stage of the graphics pipeline.
             *
             * This implies when starting to draw, to look at specific buffers
             * to find the vertex attributes (position, normals, and color for
             * each vertex)
             *
             * Unlike descriptor sets, vertex buffers are high-frequent data
             * reads automatically by the hardware input's assembler for every
             * vertex being processed.
             *
             * @param p_buffers an arbitrary span of buffers containg vertex
             * attribute data.
             * @param p_first_binding is the index of the first vertex binding
             * to update (usually 0).
             * @param p_offsets is a span of byte offsets specifying where the
             * actual data begins inside each buffer.
             *
             * @brief Additional Consideration:
             * - Number of buffers and their order MUST match what is specified
             * with using `VkVertexInputBindingDescription` or
             * `vk::vertex_attribute` specifications.
             * - Size of `p_buffers` and `p_offsets` must be identical . Where
             * each buffer needs a starting offset.
             * - Every buffer MUST need to be created using
             * `VK_BUFFER_USAGE_VERTEX_BUFFER_BIT`.
             * - Data in these transfers
             *
             *
             * Example Usage:
             *
             * ```C++
             *
             * vk::command_buffer current = ...;
             *
             * std::array<VkBuffer, 1> vertex = { vbo };
             * uint64_t offset = 0;
             * current.bind_vertex_buffers(vertex, std::span<uint64_t>(&offset,
             * 1));
             * ```
             *
             */
            void bind_vertex_buffers(std::span<const VkBuffer> p_buffers,
                                     std::span<const uint64_t> p_offsets = {},
                                     const uint32_t p_first_binding = 0) {
                vkCmdBindVertexBuffers(m_command_buffer,
                                       p_first_binding,
                                       static_cast<uint32_t>(p_buffers.size()),
                                       p_buffers.data(),
                                       p_offsets.data());
            }

            /**
             * @brief Command tells the GPU what amount of bytes are stored in
             * the indices for rendering.
             *
             * @param p_index_buffer is the handle containing the array of
             * indices
             * @param p_offset is the byte starting point (usually 0 unless you
             * store multiple meshes in one buffer).
             *
             * @brief Additional Consideration:
             * - Chosen API (8-bit, 16-bit, 32-bit) must match the bit-width of
             * your data. If you bind a 32-bit buffer as 16-bit, the GPU will
             * read twice as many indices, but each will be complete bad data.
             * - .p_offset: must be aligned to size of the index type (e.g.
             * p_offset % 4 == 0 for uint32).
             *
             * [ Index Buffer (memory) ]            [ GPU Input Assembly ]
             * +-----------------------+            +--------------------+
             * | [0] [1] [2]           | --Bind-->  | Indices Sequencer, |
             * | [2] [3] [0]           |            | State              |
             * +-----------------------+            +--------------------+
             * (Indices 0, 1, 2, ... 5)              (Select Vertices Order)
             *
             */

            /**
             * @brief Used 1 byte per index.
             *
             * Minimal memory, limited to 256 vertices.
             *
             * Example Usage:
             * ```C++
             *
             * vk::command_buffer current = ...;
             *
             * vk::index_buffer8 ibo = ...;
             * current.bind_index_buffer8(ibo);
             * ```
             *
             */
            void bind_index_buffers8(const VkBuffer& p_index_buffer,
                                     const uint64_t p_offset = 0) {
                vkCmdBindIndexBuffer(m_command_buffer,
                                     p_index_buffer,
                                     p_offset,
                                     VK_INDEX_TYPE_UINT8);
            }

            /**
             * @brief Use 2 bytes per index.
             *
             * Can be used for most 3D models up to 65k verties.
             *
             * ```C++
             *
             * vk::command_buffer current = ...;
             *
             * vk::index_buffer16 ibo = ...;
             * current.bind_index_buffer16(ibo);
             * ```
             *
             */
            void bind_index_buffers16(const VkBuffer& p_index_buffer,
                                      const uint64_t p_offset = 0) {
                vkCmdBindIndexBuffer(m_command_buffer,
                                     p_index_buffer,
                                     p_offset,
                                     VK_INDEX_TYPE_UINT16);
            }

            /**
             * @brief Use 4 bytes per index.
             *
             * Used for more higher-density terrain for complex meshes such as
             * terrain.
             *
             * ```C++
             *
             * vk::command_buffer current = ...;
             *
             * vk::index_buffer32 ibo = ...;
             * current.bind_index_buffer32(ibo);
             * ```
             *
             */
            void bind_index_buffers32(const VkBuffer& p_index_buffer,
                                      const uint64_t p_offset = 0) {
                vkCmdBindIndexBuffer(m_command_buffer,
                                     p_index_buffer,
                                     p_offset,
                                     VK_INDEX_TYPE_UINT32);
            }

            /**
             * @brief Bind the current data that stored in memory to the
             * active descriptor set for execution.
             *
             * This function records instructions into the command buffer to
             * "map" that data into the GPU's register file.
             *
             * Specifically any addresses within the shader that have variables
             * assigned to set = N.
             *
             * @param p_current is the active command recording to perform draw
             * calls.
             * @param p_pipeline_layout is the layout describing descriptor set
             * resources are mapped to.
             *
             * @brief Additional Considerations:
             * - `p_pipeline_layout` MUST be the same layout used to create the
             * currently bound pipeline.
             * - `VkDescriptorSetLayout` used to create the descriptors MUST be
             * included in the graphics pipeline layout configuration.
             * - `m_slot` must match the `set = N` declaration in your shader
             * code.
             * - The descriptor set must have been created with a layout that is
             * "compatible" with the pipeline layout.
             * - This must be invoke within a command buffer recording via
             * `.begin()`.
             *
             * [ Descriptor Set (Data) ]           [ Pipleine Layout ]
             * +-------------------+              +-----------------------+
             * | [Uniform Buffer ]  |             | Slot 0: [ Attached ]  |
             * | [Image Sampler ]   | -> Bind --> | Slot 1: [Empty]       |
             * +-------------------+              +-----------------------+
             *
             */
            void bind_descriptors(
              const VkPipelineLayout& p_pipeline_layout,
              uint64_t p_pipeline_bind_point,
              std::span<const VkDescriptorSet> p_descriptors,
              std::span<const uint32_t> p_dynamic_offsets = {},
              const uint32_t p_starting_slot = 0) {
                vkCmdBindDescriptorSets(
                  m_command_buffer,
                  static_cast<VkPipelineBindPoint>(p_pipeline_bind_point),
                  p_pipeline_layout,
                  p_starting_slot,
                  static_cast<uint32_t>(p_descriptors.size()),
                  p_descriptors.data(),
                  static_cast<uint32_t>(p_dynamic_offsets.size()),
                  p_dynamic_offsets.data());
            }

            /**
             * @brief Performs high-speed raw memory transfers between two
             * buffer handles.
             *
             * Commonly used for uploading data by copying
             * between CPU-visible staging buffers to GPU-local buffers. (faster
             * for GPU to read)
             *
             * @param p_src is the handle to source buffer (incoming data)
             * @param p_dst is the handle to destination buffer (data
             * transfering to)
             * @param p_size_bytes are the total amount of memory to copy in
             * bytes.
             *
             * @brief Additional Considerations:
             * - p_src must have vk::buffer_usage::transfer_src_bit set
             * - p_dst must have vk::buffer_usage::transfer_dst_bit set.
             * - p_size_bytes must be less or equals to size of both source and
             * destination buffers.
             * - Must be called between '.begin()' and '.end()' of a command
             * buffer.
             * - If when you plan to using p_dst after performing this
             * operation, you need to make sure to perform a buffer memory
             * barrier to ensure the copy is finished beforehand.
             *
             * [CPU-Visible Staging (buffer)]        [Dst: GPU-local Buffer]
             * +-------------------------+         +------------------------+
             * | [Data Segment]          |         |    [Identical Copy]    |
             * | [Size: size_bytes]      | =====>  |    [Size: size_bytes]  |
             * |                         |         |                        |
             * +-------------------------+         +------------------------+
             *
             * Example Usage:
             *
             * ```C++
             * vk::buffer_stream staging_buffer(logical_device, ...);
             * vk::buffer_stream vertex_buffer(logical_device, ...);
             *
             * vk::command_buffer primary_command(logical_device, ...);
             *
             * primary_command.begin(vk::command_usage::one_time_submit);
             *
             * // Transferring data from staging to GPU-accessible buffer handle
             * const auto size_bytes = vertices.size_bytes();
             * primary_command.copy(staging_buffer, vertex_buffer, size_bytes)
             *
             * primary_command.end();
             * ```
             *
             */
            void copy_buffer(const VkBuffer& p_src,
                             const VkBuffer& p_dst,
                             uint64_t p_size_bytes) {
                VkBufferCopy copy_region{};
                copy_region.size = p_size_bytes;
                vkCmdCopyBuffer(
                  m_command_buffer, p_src, p_dst, 1, &copy_region);
            }

            [[nodiscard]] bool alive() const { return m_command_buffer; }

            /**
             * @brief Primary command buffer calls batch of secondary command
             * buffers.
             *
             * Useful for multithreaded rendering: Recording to different
             * secondary commands with responsibilities of your scenes (UI,
             * Shadow, Geometry, etc).
             *
             * Then being able to link the secondary command buffers to the
             * primary command buffer altogether.
             *
             * @param p_commands are arbitrary secondary command buffers to be
             * executed part of the primary command buffer, if m_command_buffer
             * is a primary command.
             *
             * @brief Additional Considerations:
             * - m_command_buffer must be a primary command buffer.
             * - 'p_commands' MUST be secondary command buffers that have
             * already been closed with the `.end()` API.
             * - Inheritance info used in the secondary `.begin()` MUST match
             * the current state of the primary command buffer.
             *
             *
             * [ PRIMARY COMMAND ]                      [ Secondary Commands ]
             * +--------------------+                   +--------------------+
             * | Begin Command      |                   | [Secondary Cmd A]  |
             * |                    |                   | (Draw UI)          |
             * | Begin RenderPass   | -- [execute] -->  +--------------------+
             * |                    |                   | [Secondary Cmd B] |
             * |    .execute(A, B)  |                   | (Draw Particles)   |
             * +--------------------+                   +--------------------+
             *
             *
             */
            void execute(std::span<const VkCommandBuffer> p_commands) {
                vkCmdExecuteCommands(m_command_buffer,
                                     static_cast<uint32_t>(p_commands.size()),
                                     p_commands.data());
            }

            /**
             * @brief Explicitly API to properly do command buffer cleanup
             */
            void destroy() {
                vkFreeCommandBuffers(
                  m_device, m_command_pool, 1, &m_command_buffer);
                vkDestroyCommandPool(m_device, m_command_pool, nullptr);
            }

            operator VkCommandBuffer() const { return m_command_buffer; }

            operator VkCommandBuffer() { return m_command_buffer; }

        private:
            VkDevice m_device = nullptr;
            uint32_t m_begin_end_count = 0;
            VkCommandPool m_command_pool = nullptr;
            VkCommandBuffer m_command_buffer = nullptr;
        };
    };
};