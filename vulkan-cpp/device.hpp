#pragma once
#include <vulkan/vulkan.h>
#include <vulkan-cpp/types.hpp>

namespace vk {
    /**
     * @name device
     * @brief represents a vulkan logical device
     */
    class device {
        struct queue_family {
            VkQueue graphics = nullptr;
            VkQueue compute = nullptr;
            VkQueue transfer = nullptr;
        };

    public:
        device(const VkPhysicalDevice& p_physical,
               const device_enumeration& p_config);

        [[nodiscard]] queue_family family() const { return m_queue_family; }

        void destroy();

        void wait();

        operator VkDevice() const { return m_device; }

        operator VkDevice() { return m_device; }

    private:
        VkDevice m_device = nullptr;
        queue_family m_queue_family{};
    };
};