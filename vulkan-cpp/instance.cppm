module;

#include <span>
#include <vector>
#include <vulkan/vulkan.h>

export module vk:instance;

export import :types;
export import :utilities;

export namespace vk {
    inline namespace v1 {
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
            instance(const application_params& p_config,
                    const debug_message_utility& p_debug_message_utils) {
                VkApplicationInfo app_info = {
                    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                    .pNext = nullptr,
                    .applicationVersion = 1,
                    .pEngineName = p_config.name.c_str(),
                    .engineVersion = 1,
                    .apiVersion = static_cast<uint32_t>(p_config.version),
                };

                VkInstanceCreateInfo instance_ci = {
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .pApplicationInfo = &app_info
                };

                // Setting up validation layers properties
                uint32_t layer_count = 0;
                std::vector<VkLayerProperties> layer_properties;
                vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
                // std::vector<VkLayerProperties> layer_properties(layer_count);
                layer_properties.resize(layer_count);
                vkEnumerateInstanceLayerProperties(&layer_count,
                                                layer_properties.data());

                for (const VkLayerProperties property : layer_properties) {
                    m_layer_properties.emplace_back(property.layerName,
                                                    property.specVersion,
                                                    property.implementationVersion,
                                                    property.description);
                }

                // Setting up instance extensions
                instance_ci.enabledExtensionCount =
                static_cast<uint32_t>(p_config.extensions.size());
                instance_ci.ppEnabledExtensionNames = p_config.extensions.data();

#if defined(__APPLE__)
                instance_ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

                // Only run validation layers if we are running vulkan-cpp in debug mode
        #if _DEBUG
                // Setting up validation layers
                instance_ci.enabledLayerCount =
                static_cast<uint32_t>(p_config.validations.size());
                instance_ci.ppEnabledLayerNames = p_config.validations.data();

                VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(p_debug_message_utils.severity),
                    .messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(p_debug_message_utils.message_type),
                    .pfnUserCallback = p_debug_message_utils.callback,
                };

                // This is to invoke the vulkan debug utils if it is a valid callback
                // To ensure that we are not using an invalid debug callback
                if (p_debug_message_utils.callback != nullptr) {
                    instance_ci.pNext =
                    (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
                }
                else {
                    instance_ci.pNext = nullptr;
                }
        #else
                instance_ci.enabledLayerCount = 0;
                instance_ci.ppEnabledLayerNames = nullptr;
                instance_ci.pNext = nullptr;
        #endif
                vk_check(vkCreateInstance(&instance_ci, nullptr, &m_instance),
                        "vkCreateInstance");
                
        #if _DEBUG
                // This needs to be created after the VkInstance is or else it wont be applied the debug information during validation layer error message execution
                m_vk_set_debug_utils_object_name_ext = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(m_instance, "vkSetDebugUtilsObjectNameEXT"));
        #endif
            }

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
            void destroy() {}

        private:
            VkInstance m_instance = nullptr;
            std::vector<layer_properties> m_layer_properties{};
            // This needs to be set or else it becomes nullptr
            PFN_vkSetDebugUtilsObjectNameEXT m_vk_set_debug_utils_object_name_ext;
        };
    };
};