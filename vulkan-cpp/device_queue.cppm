module;

#include <vulkan/vulkan.h>
#include <memory>

export module vk:device_queue;

import :types;
import :utilities;
import :device;

export namespace vk {
    inline namespace v1 {
        /**
         * @name
         * @brief Represents a queue part of a specific logical device created
         */
        class device_queue {
        public:
            device_queue() = delete;

            device_queue(std::shared_ptr<device> p_device,
                         const queue_params& p_config) : m_device(p_device) {
                vkGetDeviceQueue(
                  *m_device, p_config.family, p_config.index, &m_queue_handler);
            }

            [[nodiscard]] bool alive() const { return m_queue_handler; }

            operator VkQueue() const { return m_queue_handler; }

            operator VkQueue() { return m_queue_handler; }

        private:
            VkQueue m_queue_handler = nullptr;
            std::shared_ptr<device> m_device=nullptr;
        };
    };
};