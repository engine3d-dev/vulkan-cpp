module;

#include <span>
#include <vector>
#include <vulkan/vulkan.h>
#include <expected>

export module vk:instance;

import :types;
import :utilities;
import :physical_device;

export namespace vk {
    inline namespace v1 {
        /**
         * @brief vk::instance represents VkInstance to initialize the vulkan
         * API.
         *
         * Provides configuration settings that can be applied to the
         * application and the vulkan debug utility for the validation layers
         */
        class instance {
        public:
            instance() = delete;
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
                    m_layer_properties.emplace_back(
                      property.layerName,
                      property.specVersion,
                      property.implementationVersion,
                      property.description);
                }

                // Setting up instance extensions
                instance_ci.enabledExtensionCount =
                  static_cast<uint32_t>(p_config.extensions.size());
                instance_ci.ppEnabledExtensionNames =
                  p_config.extensions.data();

#if defined(__APPLE__)
                instance_ci.flags |=
                  VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

                // Only execute this if we are in the debug build
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
                // Setting up validation layers
                instance_ci.enabledLayerCount =
                  static_cast<uint32_t>(p_config.validations.size());
                instance_ci.ppEnabledLayerNames = p_config.validations.data();

                VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
                    .sType =
                      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .messageSeverity =
                      static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(
                        p_debug_message_utils.severity),
                    .messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(
                      p_debug_message_utils.message_type),
                    .pfnUserCallback = p_debug_message_utils.callback,
                };

                // This is to invoke the vulkan debug utils if it is a valid
                // callback To ensure that we are not using an invalid debug
                // callback
                if (p_debug_message_utils.callback != nullptr) {
                    // instance_ci.pNext =
                    // (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
                    instance_ci.pNext =
                      reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(
                        &debug_create_info);
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

// Set the debug utility function pointer if we are in the debug build.
#if !defined(NDEBUG) || defined(_DEBUG) || defined(DEBUG)
                // This needs to be created after the VkInstance is or else it
                // wont be applied the debug information during validation layer
                // error message execution
                m_vk_set_debug_utils_object_name_ext =
                  reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
                    vkGetInstanceProcAddr(m_instance,
                                          "vkSetDebugUtilsObjectNameEXT"));
#endif
            }

            ~instance() = default;

            //! @return true if a valid VkInstance
            [[nodiscard]] bool alive() const { return !m_instance; }

            //! @return available validation layers
            std::span<const layer_properties> validation() {
                return m_layer_properties;
            }

            /**
             * @brief Enumerate physical devices and select specific physical
             * device to report properties from.
             *
             * @return vk::physical_device if successful
             * @return VkResult if an unexpected error occurs
             *
             *
             * ```C++
             *
             * vk::instance api_instance = ...;
             * std::expected<vk::physical_device, VkResult> expected =
             * api_instance.enumerate_physical_device(vk::physical_device_type::integrated);
             *
             * vk::physical_device physical_device = expected.value();
             *
             * ```
             */
            std::expected<physical_device, VkResult> enumerate_physical_device(
              physical_gpu p_device_type) {
                uint32_t device_count = 0;
                VkResult res = vkEnumeratePhysicalDevices(
                  m_instance, &device_count, nullptr);

                if (res != VK_SUCCESS) {
                    return std::unexpected(res);
                }

                std::vector<VkPhysicalDevice> physical_devices(device_count);
                res = vkEnumeratePhysicalDevices(
                  m_instance, &device_count, physical_devices.data());

                if (res != VK_SUCCESS) {
                    return std::unexpected(res);
                }

                for (const auto& device : physical_devices) {
                    VkPhysicalDeviceProperties device_properties;
                    vkGetPhysicalDeviceProperties(device, &device_properties);

                    if (device_properties.deviceType ==
                        static_cast<VkPhysicalDeviceType>(p_device_type)) {
                        return physical_device(device);
                    }
                }

                return std::unexpected(res);
            }

            /**
             * @brief returns function pointer to allow for setting debug object
             * name
             *
             *
             * This allows for utilizing vkSetDebugUtilsObjectNameEXT during
             * debug builds
             *
             * This allows for setting up object names that is useful to the
             * programmer when a validation layer error message occurs
             * unexpectedly
             *
             */
            [[nodiscard]] PFN_vkSetDebugUtilsObjectNameEXT
            get_debug_object_name() const {
                return m_vk_set_debug_utils_object_name_ext;
            }

            operator VkInstance() { return m_instance; }

            operator VkInstance() const { return m_instance; }

            //! @brief Invokes the destruction of the VkInstance.
            void destruct() {
                if (m_instance != nullptr) {
                    vkDestroyInstance(m_instance, nullptr);
                }
            }

        private:
            VkInstance m_instance = nullptr;
            std::vector<layer_properties> m_layer_properties{};
            // This needs to be set or else it becomes nullptr
            PFN_vkSetDebugUtilsObjectNameEXT
              m_vk_set_debug_utils_object_name_ext;
        };
    };
};