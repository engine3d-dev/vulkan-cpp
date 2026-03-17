module;

#include <vulkan/vulkan.h>
#include <span>
#include <array>
#include <unordered_map>

export module vk:descriptor_resource;

export import :types;
export import :utilities;
export import :uniform_buffer;
export import :sample_image;

export namespace vk {
    inline namespace v1 {
        /**
         * @param slot is the slot specific to the number slot for the
         * descriptor. Ex. layout (set = 0)
         */
        struct descriptor_layout {
            uint32_t slot = 0;
            uint32_t max_sets = 0;
            std::span<descriptor_entry> entries;
        };

        /**
         * @brief Descriptor resources are an abstraction class around the
         * descriptor set handles.
         *
         * Shaders are not able to directly have access to CPU-visible uniforms.
         * Therefore, descriptor sets are used for performing the required
         * configuration for descriptor set handles.
         *
         * @brief Additional Considerations:
         * - Block of memory dedicated to holding these handles.
         * - Defines what memory data layout the shader expects to receive.
         * - VkDescriptorSet is the instance handle to used during draw call
         * operations
         *
         */
        class descriptor_resource {
        public:
            descriptor_resource() = default;

            /**
             * @brief Constructs a descriptor resource for configuring the
             * handle
             *
             * @param p_device is the logical device used to initiate these
             * resources.
             * @param p_info is the configuration for creating the descriptor
             * set.
             * - .slot:             specifying the index to this particular
             * resource (layout(set = N, binding = X)).
             * - .max_sets:         Maximum number of descriptors allowed in the
             * pool.
             * - .entries:          Arbitrary of descriptor entries defining in
             * each resource slot.
             * - .type:             Buffer/Image type (e.g. uniform)
             * - .descriptor_count: Count of elements (1 for single, >1 or
             * arrays).
             * - .binding_point:    Actual binding index specified in the
             * shader.
             * - .stage:            Shader stage allowed to access this
             * resource.
             *
             *
             * [ Descriptor Pool ]          [ Descriptor Layout ]
             * +---------------------+      +---------------------------+
             * | [ UBO Memory Slot ] |      | Binding 0: Uniform Buffer |
             * | [ Sampler Slot]     |      | Binding 1: Tex Sampler    |
             * +---------------------+      +---------------------------+
             * |                           |
             * \__________________________/
             * |
             * V
             * [ Descriptor Set Handle]
             *
             *
             * Example Usage:
             *
             * ```C++
             *
             * std::array<vk::descriptor_entry, 2> entries = {
             *  vk::descriptor_entry{
             *         // specifies "layout (set = 0, binding = 0) uniform
             * GlobalUbo" .type = vk::buffer::uniform, .binding_point = {
             *              .binding = 0,
             *              .stage = vk::shader_stage::vertex,
             *          },
             *          .descriptor_count = 1,
             *      },
             *      vk::descriptor_entry{
             *          // layout (set = 0, binding = 1) uniform sampler2D
             *          .type = vk::buffer::combined_image_sampler,
             *          .binding_point = {
             *              .binding = 1,
             *              .stage = vk::shader_stage::fragment,
             *          },
             *          .descriptor_count = 1,
             *      }
             * };
             * vk::descriptor_layout layout = {
             *      .slot = 0,
             *      .max_sets = 2,
             *      .entries = entries,
             * };
             * vk::descriptor_resource set0(logical_device, layout);
             * ```
             *
             */
            descriptor_resource(const VkDevice& p_device,
                                const descriptor_layout& p_info)
              : m_device(p_device)
              , m_slot(p_info.slot) {
                std::vector<VkDescriptorPoolSize> pool_sizes(
                  p_info.entries.size());
                std::vector<VkDescriptorSetLayoutBinding>
                  descriptor_layout_bindings(p_info.entries.size());

                for (size_t i = 0; i < pool_sizes.size(); i++) {
                    VkDescriptorType descriptor_type =
                      static_cast<VkDescriptorType>(p_info.entries[i].type);
                    pool_sizes[i] = {
                        .type = descriptor_type,
                        .descriptorCount =
                          static_cast<uint32_t>(
                            p_info.entries[i].descriptor_count) *
                          p_info.max_sets,
                    };
                }

                for (size_t i = 0; i < descriptor_layout_bindings.size(); i++) {
                    descriptor_entry entry = p_info.entries[i];
                    descriptor_binding_point bind = entry.binding_point;

                    VkDescriptorType type =
                      static_cast<VkDescriptorType>(entry.type);

                    descriptor_layout_bindings[i] = {
                        .binding = bind.binding,
                        .descriptorType = type,
                        .descriptorCount = entry.descriptor_count,
                        .stageFlags =
                          static_cast<VkShaderStageFlags>(bind.stage),
                    };
                }

                VkDescriptorPoolCreateInfo pool_ci = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .maxSets = p_info.max_sets,
                    .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
                    .pPoolSizes = pool_sizes.data()
                };

                vk_check(vkCreateDescriptorPool(
                           m_device, &pool_ci, nullptr, &m_descriptor_pool),
                         "vkCreateDescriptorPool");

                VkDescriptorSetLayoutCreateInfo descriptor_layout_ci = {
                    .sType =
                      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .bindingCount =
                      static_cast<uint32_t>(descriptor_layout_bindings.size()),
                    .pBindings = descriptor_layout_bindings.data()
                };

                vk_check(vkCreateDescriptorSetLayout(m_device,
                                                     &descriptor_layout_ci,
                                                     nullptr,
                                                     &m_descriptor_layout),
                         "vkCreateDescriptorSetLayout");
                VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .pNext = nullptr,
                    .descriptorPool = m_descriptor_pool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = &m_descriptor_layout
                };

                vk_check(vkAllocateDescriptorSets(m_device,
                                                  &descriptor_set_alloc_info,
                                                  &m_descriptor_set),
                         "vkAllocateDescriptorSets");
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
             * | [Image Sampler ]  | --> Bind --> | Slot 1: [Empty]       |
             * +-------------------+              +-----------------------+
             *
             */
            void bind(const VkCommandBuffer& p_current,
                      const VkPipelineLayout& p_pipeline_layout) {
                vkCmdBindDescriptorSets(p_current,
                                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        p_pipeline_layout,
                                        m_slot,
                                        1,
                                        &m_descriptor_set,
                                        0,
                                        nullptr);
            }

            /**
             * @brief Performs the operation to actual update the descriptor set
             * handle with the uniforms data segments.
             *
             * This maps the uniform VkBuffer and VkImage handles to the
             * specific logical bindings that are associated with the shader
             * declarations.
             *
             * Without setting this, the descriptor set would be implied to
             * contain empty slots and pointing (lookup) to nothing.
             *
             * @brief Additional Considerations:
             * - Cannot update a descriptor set that is currently in used by a
             * command buffer that is "in-flight" (executing on the GPU).
             * - Resource type (e.g. combind_image_sampler) must match what has
             * been defined in the `descriptor layout` for that `dst_binding`.
             * - Buffer/Image handles must remain valid until the GPU has
             * finished executing the cmomand buffer that is using this
             * particular descriptor set.
             *
             *
             * [ CPU Uniforms Handles ]                [ GPU Descriptor ]
             * +--------------------+           +----------------------------+
             * | write_buffer       | --maps--> | Binding 0: Buffer Ptr      |
             * | (Handle + Offset)  |           | (Address: 0x00FF...)       |
             * +--------------------+           +----------------------------+
             * | write_image        | --maps--> | Binding 1: Image Ptr       |
             * | (View + Sampler)   |           | (Layout: shader_read_only) |
             * +--------------------+           +----------------------------+
             * (Mapping CPU Data)                   (Active GPU Resources)
             *
             * [GLSL Shader]
             * GLSL equivalent of the active GPU resources of the "GPU
             * Descriptor"
             * +-----------------------------------------------+
             * | layout(set=N, binding = 0) uniform buffer {}; |
             * | layout(set=N, binding = 1) sampler2D texture; |
             * +-----------------------------------------------+
             * (Executing GPU-visible Resources)
             *
             * Example Usage:
             *
             * ```C++
             *
             * vk::descriptor_resource set0(logical_device, layout);
             *
             * // Uniform Buffers Handle
             * std::array<vk::write_buffer, 1> uniforms0 = {
             *  vk::write_buffer{
             *       .buffer = test_ubo,
             *       .offset = 0,
             *      .range = static_cast<uint32_t>(test_ubo.size_bytes()),
             *  },
             * };
             * std::array<vk::write_buffer_descriptor, 1> uniforms = {
             *  vk::write_buffer_descriptor{
             *      .dst_binding = 0,
             *      .uniforms = uniforms0,
             *  },
             * };
             *
             * // View + Samplers Handle
             * std::array<vk::write_image, 1> samplers = {
             *   vk::write_image{
             *      .sampler = sampler,
             *      .view = image_view,
             *      .layout = vk::image_layout::shader_read_only_optimal,
             *   },
             * };
             *
             * // Specify image descriptor images/samplers to the descriptor
             * std::array<vk::write_image_descriptor, 1> sample_images = {
             *   vk::write_image_descriptor{
             *     .dst_binding = 1,
             *     .sample_images = samplers,
             *   }
             * };
             * set0_resource.update(uniforms, sample_images);
             * ```
             *
             */
            void update(std::span<const write_buffer_descriptor> p_uniforms,
                        std::span<const write_image_descriptor> p_images = {}) {
                std::vector<VkWriteDescriptorSet> write_descriptors;

                // uint32_t represent the destination bindings to those
                // resources (uniforms and sample images)
                std::unordered_map<uint32_t,
                                   std::vector<VkDescriptorBufferInfo>>
                  buffer_infos;
                std::unordered_map<uint32_t, std::vector<VkDescriptorImageInfo>>
                  image_infos;

                // handle uniforms
                for (const auto& ubo : p_uniforms) {
                    for (const auto& uniform : ubo.uniforms) {
                        buffer_infos[ubo.dst_binding].emplace_back(
                          uniform.buffer, uniform.offset, uniform.range);
                    }

                    VkWriteDescriptorSet write_descriptor = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = m_descriptor_set,
                        .dstBinding = ubo.dst_binding,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32_t>(
                          buffer_infos[ubo.dst_binding].size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = buffer_infos[ubo.dst_binding].data(),
                    };

                    write_descriptors.emplace_back(write_descriptor);
                }

                for (const auto& ubo : p_images) {

                    for (const auto& sample_image : ubo.sample_images) {
                        image_infos[ubo.dst_binding].emplace_back(
                          sample_image.sampler,
                          sample_image.view,
                          static_cast<VkImageLayout>(sample_image.layout));
                    }

                    VkWriteDescriptorSet write_descriptor = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = m_descriptor_set,
                        .dstBinding = ubo.dst_binding,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32_t>(
                          image_infos[ubo.dst_binding].size()),
                        .descriptorType =
                          VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .pImageInfo = image_infos[ubo.dst_binding].data(),
                    };

                    write_descriptors.emplace_back(write_descriptor);
                }

                vkUpdateDescriptorSets(
                  m_device,
                  static_cast<uint32_t>(write_descriptors.size()),
                  write_descriptors.data(),
                  0,
                  nullptr);

                // Ensures to clear up so we dont have any existing handles
                // because they only need to exist until we've updated the
                // descriptors
                buffer_infos.clear();
                image_infos.clear();
            }

            [[nodiscard]] VkDescriptorSetLayout layout() const {
                return m_descriptor_layout;
            }

            void destroy() {
                if (m_descriptor_pool != nullptr) {
                    vkDestroyDescriptorPool(
                      m_device, m_descriptor_pool, nullptr);
                }

                if (m_descriptor_layout != nullptr) {
                    vkDestroyDescriptorSetLayout(
                      m_device, m_descriptor_layout, nullptr);
                }
            }

        private:
            VkDevice m_device = nullptr;
            uint32_t m_slot;
            VkDescriptorPool m_descriptor_pool = nullptr;
            VkDescriptorSetLayout m_descriptor_layout = nullptr;
            VkDescriptorSet m_descriptor_set = nullptr;
        };
    };
};