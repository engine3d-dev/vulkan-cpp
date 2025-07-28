#pragma once
#include <vulkan-cpp/types.hpp>

namespace vk {
    /**
     * @name
     * @brief Represents a queue part of a specific logical device created
    */
    class device_queue {
    public:
        device_queue() = default;
        device_queue(const VkDevice& p_device, const queue_enumeration& p_config);

        [[nodiscard]] bool alive() const { return m_queue_handler; }

        operator VkQueue() const { return m_queue_handler; }

        operator VkQueue() { return m_queue_handler; }


    private:
        VkQueue m_queue_handler=nullptr;
    };
};