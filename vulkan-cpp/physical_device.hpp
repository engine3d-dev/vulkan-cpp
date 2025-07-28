#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>
#include <vector>

namespace vk {

    class physical_device {
        struct queue_indices {
            uint32_t graphics=-1;
            uint32_t compute=-1;
            uint32_t transfer=-1;
        };
    public:
        physical_device() = default;

        physical_device(const VkInstance& p_instance, const physical_enumeration& p_physical_enumeration);

        //! @return true if physical device is valid
        [[nodiscard]] bool alive() const { return m_physical_device; }

        //! @return queue family indices for graphics, compute, and transfer operations
        [[nodiscard]] queue_indices family_indices() const { return m_queue_family_indices; }

        //! @return the presentation index for the presentation queue
        uint32_t queue_present_index(const VkSurfaceKHR& p_surface);


        operator VkPhysicalDevice() { return m_physical_device; }

        operator VkPhysicalDevice() const { return m_physical_device; }

    private:
        VkPhysicalDevice m_physical_device=nullptr;
        std::span<VkQueueFamilyProperties> m_queue_family_properties;
        queue_indices m_queue_family_indices{};
    };
};