#pragma once
#include <vulkan-cpp/types.hpp>
#include <span>
#include <vector>

namespace vk {
    class instance {
    public:
        instance(const application_configuration& p_config, const debug_message_utility& p_debug_message_utils);

        //! @return if instance is valid
        [[nodiscard]] bool alive() const { return !m_instance; }

        //! @return validation layers available
        std::span<const layer_properties> validation() { return m_layer_properties; }

        operator VkInstance() { return m_instance; }

        operator VkInstance() const { return m_instance; }

        void destroy();
    private:
        VkInstance m_instance = nullptr;
        std::vector<layer_properties> m_layer_properties{};
    };
};