module;

#include <vulkan/vulkan.h>

export module vk:device_queue;

export import :types;
export import :utilities;


export namespace vk {
    inline namespace v1 {
        /**
         * @name
         * @brief Represents a queue part of a specific logical device created
         */
        class device_queue {
        public:
            device_queue() = default;

            device_queue(const VkDevice& p_device, const queue_params& p_config) {
                vkGetDeviceQueue(p_device, p_config.family, p_config.index, &m_queue_handler);
            }

            [[nodiscard]] bool alive() const { return m_queue_handler; }

            operator VkQueue() const { return m_queue_handler; }

            operator VkQueue() { return m_queue_handler; }

        private:
            VkQueue m_queue_handler = nullptr;
        };
    };
};