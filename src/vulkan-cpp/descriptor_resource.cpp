#include <vulkan-cpp/descriptor_resource.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <print>

namespace vk {

    descriptor_resource::descriptor_resource(const VkDevice& p_device,
                                             const descriptor_layout& p_info)
      : m_device(p_device)
      , m_slot(p_info.slot) {
        std::vector<VkDescriptorPoolSize> pool_sizes(p_info.entries.size());
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings(
          p_info.entries.size());

        for (size_t i = 0; i < pool_sizes.size(); i++) {
            VkDescriptorType descriptor_type =
              to_descriptor_type(p_info.entries[i].type);
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

            VkDescriptorType type = to_descriptor_type(entry.type);

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

    void descriptor_resource::update(
      std::span<write_buffer_descriptor> p_uniforms,
      std::span<write_image_descriptor> p_texture_image_handles) {
        std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;

        for (const auto& uniform : p_uniforms) {
            // uniform, offste, and range
            buffer_infos.emplace_back(
              uniform.buffer, uniform.offset, uniform.range);
        }

        for (const auto& sample_image : p_texture_image_handles) {
            // VkSampler, VkImageView, VkImageLayout
            image_infos.emplace_back(sample_image.sampler,
                                     sample_image.view,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        /*

            vk::write_descriptor set0_writes = {
                .dst_binding = 0,
                .uniforms = some_uniform, // std::span<vk::uniform_buffer>
                // or
                .image_uniforms = some_image_uniform // std::span<vk::texture>
           or std::span<vk::image_handle> or std::span<vk::image_source>
            };

        */
        std::vector<VkWriteDescriptorSet> write_descriptors;
        VkWriteDescriptorSet write_buffer{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(buffer_infos.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = buffer_infos.data(),
        };

        write_descriptors.push_back(write_buffer);

        // TODO: Probably have this handle no textures bit better...
        // For now this'll check if there are any textures, if not. Then do
        // not add anything to writable textures
        if (!p_texture_image_handles.empty()) {
            VkWriteDescriptorSet write_image{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_set,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount =
                    static_cast<uint32_t>(image_infos.size()),
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

	void descriptor_resource::update(std::span<const write_buffer_descriptor2> p_uniforms, std::span<const write_image_descriptor2> p_images) {
		/*
		std::vector<VkDescriptorBufferInfo> buffer_infos;
        std::vector<VkDescriptorImageInfo> image_infos;

        for (const auto& uniform : p_uniforms) {
            // uniform, offste, and range
            buffer_infos.emplace_back(
              uniform.buffer, uniform.offset, uniform.range);
        }

        for (const auto& sample_image : p_texture_image_handles) {
            // VkSampler, VkImageView, VkImageLayout
            image_infos.emplace_back(sample_image.sampler,
                                     sample_image.view,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        std::vector<VkWriteDescriptorSet> write_descriptors;
        VkWriteDescriptorSet write_buffer{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<uint32_t>(buffer_infos.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = buffer_infos.data(),
        };

        write_descriptors.push_back(write_buffer);

        // TODO: Probably have this handle no textures bit better...
        // For now this'll check if there are any textures, if not. Then do
        // not add anything to writable textures
        if (!p_texture_image_handles.empty()) {
            VkWriteDescriptorSet write_image{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_set,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount =
                    static_cast<uint32_t>(image_infos.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = image_infos.data()
            };

            write_descriptors.push_back(write_image);
        }
		*/

		std::vector<VkWriteDescriptorSet> write_descriptors;
		std::vector<VkDescriptorBufferInfo> buffer_infos;
		std::vector<VkDescriptorImageInfo> image_infos;

		for(size_t i = 0; i < p_uniforms.size(); i++) {
			auto object = p_uniforms[i];

			for(size_t j = 0; j < object.uniforms.size(); j++) {
				auto ubo = object.uniforms[j];
				buffer_infos.emplace_back(ubo.buffer, ubo.offset, ubo.range);
			}

			VkWriteDescriptorSet write_descriptor = {
                . sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_set,
                .dstBinding = object.dst_binding,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(buffer_infos.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pBufferInfo = buffer_infos.data(),
            };

			write_descriptors.emplace_back(write_descriptor);
		}

		for(size_t i = 0; i < p_images.size(); i++) {
			auto object = p_images[i];

			for(size_t j = 0; j < object.sample_images.size(); j++) {
				auto ubo = object.sample_images[j];
				image_infos.emplace_back(ubo.sampler, ubo.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}

			VkWriteDescriptorSet write_descriptor = {
                . sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = m_descriptor_set,
                .dstBinding = object.dst_binding,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(image_infos.size()),
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = image_infos.data(),
            };

			write_descriptors.emplace_back(write_descriptor);
		}

        vkUpdateDescriptorSets(
            m_device,
            static_cast<uint32_t>(write_descriptors.size()),
            write_descriptors.data(),
            0,
            nullptr);
	}

    void descriptor_resource::bind(const VkCommandBuffer& p_current,
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

    void descriptor_resource::destroy() {
        if (m_descriptor_pool != nullptr) {
            vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
        }

        if (m_descriptor_layout != nullptr) {
            vkDestroyDescriptorSetLayout(
              m_device, m_descriptor_layout, nullptr);
        }
    }
};