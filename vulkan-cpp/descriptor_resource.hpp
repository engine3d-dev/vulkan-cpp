#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vector>
#include <vulkan-cpp/uniform_buffer.hpp>
#include <vulkan-cpp/sample_image.hpp>

namespace vk {

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

        void update(const std::span<write_buffer_descriptor>& p_uniforms, const std::span<write_image_descriptor>& p_texture_image_handles={});

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