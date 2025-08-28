#pragma once
#include <vulkan/vulkan.h>
#include <span>
#include <vector>

namespace vk {

    class index_buffer32 {
    public:
        index_buffer32() = default;

        index_buffer32(const std::span<uint32_t>& p_indices) {}

        void bind(const VkCommandBuffer& p_current);

        void destroy();

    private:
        void invalidate(const std::span<uint32_t>& p_indices);
    };

    template<size_t arr_size>
    class index_buffer32_without {
    public:
        index_buffer32_without() = default;

        // boilerplate without std::span<uint32_t>
        index_buffer32_without(const std::vector<uint32_t>& p_indices) {}
        index_buffer32_without(const uint32_t p_arr[], size_t p_size) {}
        index_buffer32_without(uint32_t* p_arr, size_t p_size) {}
        index_buffer32_without(std::array<uint32_t, arr_size> p_array) {}

        void bind(const VkCommandBuffer& p_current);

        void destroy();
    };


};