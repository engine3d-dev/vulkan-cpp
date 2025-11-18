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

        /**
         * @brief returns function pointer to allow for setting debug object name
         * 
         * 
         * This allows for utilizing vkSetDebugUtilsObjectNameEXT during debug builds
         * 
         * This allows for setting up object names that is useful to the programmer when a validation layer error message occurs unexpectedly
         * 
        */
        [[nodiscard]] PFN_vkSetDebugUtilsObjectNameEXT get_debug_object_name() const {
            return m_vk_set_debug_utils_object_name_ext;
        }

        operator VkInstance() { return m_instance; }

        operator VkInstance() const { return m_instance; }

        //! @brief Invokes the destruction of the VkInstance.
        void destroy();

    private:
        VkInstance m_instance = nullptr;
        std::vector<layer_properties> m_layer_properties{};
        // This needs to be set or else it becomes nullptr
        PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name_ext;
    };
};