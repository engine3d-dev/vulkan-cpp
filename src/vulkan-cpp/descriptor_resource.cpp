#include <vulkan-cpp/descriptor_resource.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <print>

namespace vk {

    descriptor_resource::descriptor_resource(const VkDevice& p_device, const descriptor_layout& p_info) : m_device(p_device), m_slot(p_info.slot) {
        m_size_bytes = p_info.size_bytes;
        m_allocated_count = p_info.allocate_count;
        std::vector<VkDescriptorPoolSize> pool_sizes(p_info.entries.size());
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings(p_info.entries.size());

        for(size_t i = 0; i < pool_sizes.size(); i++) {
            VkDescriptorType descriptor_type = to_descriptor_type(p_info.entries[i].type);
            std::println("descriptor_count = {}", p_info.entries[i].descriptor_count);
            pool_sizes[i] = {
                .type = descriptor_type,
                .descriptorCount = static_cast<uint32_t>(p_info.entries[i].descriptor_count) * p_info.max_sets,
            };
        }

        for (size_t i = 0; i < descriptor_layout_bindings.size(); i++) {
            descriptor_entry entry = p_info.entries[i];
            descriptor_binding_point bind = entry.binding_point;

            VkDescriptorType type = to_descriptor_type(entry.type);

            descriptor_layout_bindings[i] = {
                .binding = bind.binding,
                .descriptorType = type,
                .descriptorCount = entry.descriptor_count,
                .stageFlags = to_shader_stage(bind.stage),
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

        vk_check(vkCreateDescriptorPool(m_device, &pool_ci, nullptr, &m_descriptor_pool), "vkCreateDescriptorPool");

        VkDescriptorSetLayoutCreateInfo descriptor_layout_ci = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount =
              static_cast<uint32_t>(descriptor_layout_bindings.size()),
            .pBindings = descriptor_layout_bindings.data()
        };

        vk_check(vkCreateDescriptorSetLayout(m_device, &descriptor_layout_ci, nullptr, &m_descriptor_layout),"vkCreateDescriptorSetLayout");

        std::vector<VkDescriptorSetLayout> layouts(m_allocated_count,
                                                   m_descriptor_layout);
        VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = m_descriptor_pool,
            .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
            .pSetLayouts = layouts.data()
        };

        std::println("descriptor layout size = {}", layouts.size());

        m_descriptor_sets = std::vector<VkDescriptorSet>(m_allocated_count);

        vk_check(vkAllocateDescriptorSets(m_device,&descriptor_set_alloc_info,m_descriptor_sets.data()), "vkAllocateDescriptorSets");
    }

    void descriptor_resource::update(const std::span<uniform_buffer>& p_uniforms, const std::span<sampled_image>& p_texture_image_handles) {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;

        for(const auto& uniform : p_uniforms) {
            // uniform, offste, and range
            buffer_infos.emplace_back(uniform, 0, p_uniforms.size_bytes());
        }

        for(const auto& sample_image : p_texture_image_handles) {
            // VkSampler, VkImageView, VkImageLayout
            image_infos.emplace_back(sample_image.sampler, sample_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        /*
        
            vk::write_descriptor set0_writes = {
                .dst_binding = 0,
                .uniforms = some_uniform, // std::span<vk::uniform_buffer>
                // or
                .image_uniforms = some_image_uniform // std::span<vk::texture> or std::span<vk::image_handle> or std::span<vk::image_source>
            };
        
        */
        for (size_t i = 0; i < m_descriptor_sets.size(); i++) {
            std::vector<VkWriteDescriptorSet> write_descriptors;
            VkWriteDescriptorSet write_buffer{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_sets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(buffer_infos.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = buffer_infos.data(),
            };

            write_descriptors.push_back(write_buffer);

            // TODO: Probably have this handle no textures bit better...
            // For now this'll check if there are any textures, if not. Then do not add anything to writable textures
            if(!p_texture_image_handles.empty()) {
                VkWriteDescriptorSet write_image{
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .pNext = nullptr,
                    .dstSet = m_descriptor_sets[i],
                    .dstBinding = 1,
                    .dstArrayElement = 0,
                    .descriptorCount = static_cast<uint32_t>(image_infos.size()),
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .pImageInfo = image_infos.data()
                };

                write_descriptors.push_back(write_image);
            }

            vkUpdateDescriptorSets(
              m_device,
              static_cast<uint32_t>(write_descriptors.size()),
              write_descriptors.data(),
              0,
              nullptr);
        }
    }

    void descriptor_resource::bind(const VkCommandBuffer& p_current, uint32_t p_frame_idx, const VkPipelineLayout& p_pipeline_layout) {
        if (m_descriptor_sets.size() <= 0) {
            return;
        }

        vkCmdBindDescriptorSets(p_current,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                p_pipeline_layout,
                                m_slot,
                                1,
                                &m_descriptor_sets[p_frame_idx],
                                0,
                                nullptr);
    }

    void descriptor_resource::destroy() {
        if (m_descriptor_pool != nullptr) {
            vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
        }

        if (m_descriptor_layout != nullptr) {
            vkDestroyDescriptorSetLayout(m_device, m_descriptor_layout, nullptr);
        }
    }
};