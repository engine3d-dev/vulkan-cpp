#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vector>
#include <vulkan-cpp/uniform_buffer.hpp>

namespace vk {
    /*
        std::vector<descriptor_entry> entries = {
            descriptor_binding_entry{
                // specifies "layout (set = 0, binding = 0) uniform GlobalUbo"
                .type = vk::buffer::uniform,
                .binding_point = { .binding = 0, .stage = shader_stage::vertex },
                .descriptor_count = 1,
            }
        };

        descriptor_layout set0_layout = {
            .slot = 0, // layout(set = 0)
            .allocate_count = m_image_count, // the count how many descriptor
                                                // set layout able to be allocated
            .max_sets =
                m_image_count, // max of descriptor sets able to allocate
            .size_bytes =
                sizeof(camera_ubo), // size of bytes of the uniforms utilized by
                                    // this descriptor sets
            .entry = entries      // specifies pool sizes and descriptor layout
        };
        vk::descriptor_resource set_resource0(logical_device, set0_layout);
    */

    /**
     * @param slot is the slot specific to the number slot for the descriptor. Ex. layout (set = 0)
    */
    struct descriptor_layout {
        uint32_t slot=0;
        uint32_t allocate_count=0;
        uint32_t max_sets=0;
        uint32_t size_bytes=0;
        std::span<descriptor_entry> entries;
    };

    class descriptor_resource {
    public:
        descriptor_resource() = default;
        descriptor_resource(const VkDevice& p_device, const descriptor_layout& p_info);

        void bind(const VkCommandBuffer& p_current, uint32_t p_frame_idx, const VkPipelineLayout& p_layout);

        void update(const std::span<uniform_buffer>& p_uniforms);

        [[nodiscard]] VkDescriptorSetLayout layout() const { return m_descriptor_layout; }

        void destroy();

    private:
        VkDevice m_device=nullptr;
        uint32_t m_slot;
        uint32_t m_size_bytes = 0;
        uint32_t m_allocated_count=0;
        VkDescriptorPool m_descriptor_pool=nullptr;
        VkDescriptorSetLayout m_descriptor_layout=nullptr;
        std::vector<VkDescriptorSet> m_descriptor_sets{};
    };

};