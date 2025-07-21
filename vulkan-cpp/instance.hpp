#pragma once
#include <vulkan-cpp/types.hpp>
#include <span>
#include <vector>

namespace vk {
    /**
     * @brief vk::instance represents VkInstance to initialize the vulkan API.
     *
     * Provides configuration settings that can be applied to the application
     * and the vulkan debug utility for the validation layers
     */
    class instance {
    public:
        /**
         * @param p_config sets the application information that vulkan has
         * optionally.
         * @param p_debug_message_utils is for setting up vulkan's utility
         * tooling for debugging and enabling validation layers
         */
        instance(const application_configuration& p_config,
                 const debug_message_utility& p_debug_message_utils);

        //! @return true if a valid VkInstance
        [[nodiscard]] bool alive() const { return !m_instance; }

        //! @return available validation layers
        std::span<const layer_properties> validation() {
            return m_layer_properties;
        }

        operator VkInstance() { return m_instance; }

        operator VkInstance() const { return m_instance; }

        //! @brief Invokes the destruction of the VkInstance.
        void destroy();

    private:
        VkInstance m_instance = nullptr;
        std::vector<layer_properties> m_layer_properties{};
    };
};