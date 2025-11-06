#pragma once
#include <vulkan/vulkan.h>
#include <span>
#include <vector>

namespace vk {

    class index_buffer32 {
    public:
        index_buffer32() = default;

        index_buffer32(std::span<const uint32_t> p_indices) {}

        void bind(const VkCommandBuffer& p_current);

        void destroy();

    private:
        void invalidate(std::span<const uint32_t> p_indices);
    };

};