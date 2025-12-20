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
         * @param slot is the slot specific to the number slot for the descriptor.
         * Ex. layout (set = 0)
         */
        struct descriptor_layout {
            uint32_t slot = 0;
            uint32_t max_sets = 0;
            std::span<descriptor_entry> entries;
        };

        class descriptor_resource {
        public:
            descriptor_resource() = default;
            descriptor_resource(const VkDevice& p_device, const descriptor_layout& p_info) : m_device(p_device), m_slot(p_info.slot) {
                std::vector<VkDescriptorPoolSize> pool_sizes(p_info.entries.size());
                std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings(p_info.entries.size());

                for (size_t i = 0; i < pool_sizes.size(); i++) {
                    VkDescriptorType descriptor_type = static_cast<VkDescriptorType>(p_info.entries[i].type);
                    pool_sizes[i] = {
                        .type = descriptor_type,
                        .descriptorCount =
                        static_cast<uint32_t>(p_info.entries[i].descriptor_count) *
                        p_info.max_sets,
                    };
                }

                for (size_t i = 0; i < descriptor_layout_bindings.size(); i++) {
                    descriptor_entry entry = p_info.entries[i];
                    descriptor_binding_point bind = entry.binding_point;

                    VkDescriptorType type = static_cast<VkDescriptorType>(entry.type);

                    descriptor_layout_bindings[i] = {
                        .binding = bind.binding,
                        .descriptorType = type,
                        .descriptorCount = entry.descriptor_count,
                        .stageFlags = static_cast<VkShaderStageFlags>(bind.stage),
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
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .bindingCount =
                    static_cast<uint32_t>(descriptor_layout_bindings.size()),
                    .pBindings = descriptor_layout_bindings.data()
                };

                vk_check(
                vkCreateDescriptorSetLayout(
                    m_device, &descriptor_layout_ci, nullptr, &m_descriptor_layout),
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

            void bind(const VkCommandBuffer& p_current, const VkPipelineLayout& p_pipeline_layout) {
                vkCmdBindDescriptorSets(p_current,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                p_pipeline_layout,
                                m_slot,
                                1,
                                &m_descriptor_set,
                                0,
                                nullptr);
            }
            
            void update(std::span<const write_buffer_descriptor> p_uniforms, std::span<const write_image_descriptor> p_images={}) {
                std::vector<VkWriteDescriptorSet> write_descriptors;

                // uint32_t represent the destination bindings to those resources (uniforms and sample images)
                std::unordered_map<uint32_t, std::vector<VkDescriptorBufferInfo>> buffer_infos;
                std::unordered_map<uint32_t, std::vector<VkDescriptorImageInfo>> image_infos;

                // handle uniforms
                for(const auto& ubo : p_uniforms) {
                    for(const auto& uniform : ubo.uniforms) {
                        buffer_infos[ubo.dst_binding].emplace_back(uniform.buffer, uniform.offset, uniform.range);
                    }

                    VkWriteDescriptorSet write_descriptor = {
                        . sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = m_descriptor_set,
                        .dstBinding = ubo.dst_binding,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32_t>(buffer_infos[ubo.dst_binding].size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pBufferInfo = buffer_infos[ubo.dst_binding].data(),
                    };

                    write_descriptors.emplace_back(write_descriptor);
                }

                for(const auto& ubo : p_images) {

                    for(const auto& sample_image : ubo.sample_images) {
                        image_infos[ubo.dst_binding].emplace_back(sample_image.sampler, sample_image.view, static_cast<VkImageLayout>(sample_image.layout));
                    }

                    VkWriteDescriptorSet write_descriptor = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .pNext = nullptr,
                        .dstSet = m_descriptor_set,
                        .dstBinding = ubo.dst_binding,
                        .dstArrayElement = 0,
                        .descriptorCount = static_cast<uint32_t>(image_infos[ubo.dst_binding].size()),
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
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
                
                // Ensures to clear up so we dont have any existing handles because they only need to exist until we've updated the descriptors
                buffer_infos.clear();
                image_infos.clear();
            }

            [[nodiscard]] VkDescriptorSetLayout layout() const {
                return m_descriptor_layout;
            }

            void destroy() {
                if (m_descriptor_pool != nullptr) {
                    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
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