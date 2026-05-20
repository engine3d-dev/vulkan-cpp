module;

#include <concepts>
#include <type_traits>
#include <tuple>
#include <vulkan/vulkan.h>

export module vk:feature_extensions;

export namespace vk {

    /**
     * @brief ExtensionConcept being a concept
     *
     * Used for mainly performing compile-time checks if the features specified
     * are actual valid features
     *
     * Because Vulkan is a C API and .pNext being a void*. This can be bug-prone
     * for accidentally pointing to a type that is not a feature struct like a
     * std::string, for example.
     *
     * VulkanChainable concept is used to verify the conditions of the type that
     * is being passed to chain_features.
     *
     * Which is important to ensure that we only accept types that do contain
     * .sType and .pNext
     *
     * TODO: Probably might have this be a template specialization due to being
     * able to verify at compile-time if the vulkan features, we need to check
     * for are valid. This way even if the struct has `.sType` and `.pNext`, we
     * can at least verify the struct is a feature that is valid
     *
     *
     * Example:
     *
     * ```C++
     *
     * // This trait should be false by default
     * // Meaning if you reach this, then it should error out
     * template<typename T>
     * struct device_feature : public std::false_type {
     *      // Having this be a compiler-error by default if this struct is
     * reached.
     *      // Users should not be allowed to supply a non vulkan feature struct
     * ONLY the vulkan feature struct static_assert(*this, "Invalid vulkan
     * feature specified. Only accepting valid vulkan features.");
     * };
     */
    template<typename T>
    concept ExtensionConcept = requires(T t) {
        { t.sType } -> std::convertible_to<VkStructureType>;
        { t.pNext } -> std::convertible_to<void*>;
    };

    /**
     * @brief a trait that can be aliased to an
     */
    template<typename T, VkStructureType SType>
    struct feature_trait : public T {

        feature_trait(T p_initialize_feature)
          : T(p_initialize_feature) {
            this->sType = SType;
        }
    };

    using physical_device_features2 =
      feature_trait<VkPhysicalDeviceFeatures2,
                    VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2>;
    using buffer_device_address = feature_trait<
      VkPhysicalDeviceBufferDeviceAddressFeatures,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES>;

    // Descriptor indexing to access unbounded array of a given uniform
    using descriptor_indexing_feature = feature_trait<
      VkPhysicalDeviceDescriptorIndexingFeatures,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES>;

    //! @brief Modernized Vulkan feature that reduces state managing for
    //! renderpasses + framebuffers
    using dynamic_rendering_feature = feature_trait<
      VkPhysicalDeviceDynamicRenderingFeatures,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES>;

    //! @brief Represent buffers as direct pointer in shaders
    // using buffer_device_address_feature = feature_trait<
    //   VkPhysicalDeviceBufferAddressFeatures,
    //   VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES>;

    //! @brief Modernize approach for bindless techniques
    using descriptor_buffer_feature = feature_trait<
      VkPhysicalDeviceDescriptorBufferFeaturesEXT,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT>;

    //! @brief Updating descriptors similar fashion as push constants
    using push_descriptors_feature = feature_trait<
      VkPhysicalDevicePushDescriptorPropertiesKHR,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR>;

    //! @brief Feature that can skip monolith pipeline objects
    using shader_object_feature = feature_trait<
      VkPhysicalDeviceShaderObjectFeaturesEXT,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_OBJECT_FEATURES_EXT>;

    //! @brief Handled dynamic pipeline states
    using pipeline_library = feature_trait<
      VkPhysicalDeviceGraphicsPipelineLibraryFeaturesEXT,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_GRAPHICS_PIPELINE_LIBRARY_FEATURES_EXT>;

    //! @brief Clean barriers and semaphores
    using sync2_feature = feature_trait<
      VkPhysicalDeviceSynchronization2Features,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES>;

    template<ExtensionConcept... Features>
    class device_features {
    public:
        device_features(Features&&... p_args)
          : m_data(std::forward<Features>(p_args)...) {

            // Check unpacked structures of vulkan features
            // Then we chain the pNext pointer altogether for each feature we
            // unpack if there are any
            if constexpr (sizeof...(Features) > 0) {
                std::apply([this](auto&... spec) { chain_features(spec...); },
                           m_data);
            }
        }

        /**
         * @brief We provide the chained pNext ptr
         *
         * This will let us directly have the chained features the user can
         * specify.
         *
         * @return nullptr if no features are specified, returns the pNext
         * feature chain otherwise.
         *
         * @returns the internal data of the vulkan device extensions that are
         * chained altogether.
         *
         * Will return nullptr if no extensions are enabled.
         */
        [[nodiscard]] void* data() noexcept {

            // At compile-time we retrieve the head pointer of the entire
            // feature list chain altogether.
            if constexpr (sizeof...(Features) > 0) {
                return &std::get<0>(m_data);
            }

            return nullptr;
        }

    private:
        /**
         * @brief Vulkan uses a linked-list specifying lists of features to
         * enable.
         *
         * This linked-list feature is used to enable modern GPU features. This
         * internal API allows to take in a collection of structs that are
         * inter-connected them into a single chain the internal API's can read
         * from.
         *
         * The pNext used for chaining
         *
         * [ Feature A ]      [ Feature B ]     [ Feature N ]
         * +------------+     +------------+    +-----------+
         * | .sType     |     | .sType     |    | .sType    |
         * | .pNext     | --> | .pNext     | -> | .pNext    |
         * +------------+     +------------+    +-----------+
         * ( head )             ( internal )     ( tail )
         *
         * Example Usage:
         *
         * ```C++
         *
         * vk::device_features device_features{
         *    vk::descriptor_indexing_feature{{
         *        .descriptorBindingPartiallyBound = true,
         *        .descriptorBindingVariableDescriptorCount = true,
         *        .descriptorBindingSampledImageUpdateAfterBind = true,
         *    } },
         *    vk::dynamic_rendering{ {
         *    .dynamicRendering = true,
         *    } },
         * };
         *
         * vk::device_params config_logical_device = {
         *      .features = device_features,
         * };
         *
         * ```
         *
         *
         */
        template<ExtensionConcept T, ExtensionConcept... Feature>
        void chain_features(T& head, Feature&... tail) {
            /**
             * @brief Passing an arbitrary sequences of
             * VkPhysicalDevice*Features
             *
             *
             * @brief Handled using Variadic Templates
             * Compiler does not generate a loop, rather it'll generate a flat
             * sequence of instructions.
             *
             * Essentially doing something like:
             * chain_features(descriptor_indexing, dynamic_rendering).
             *
             * The compiler will automatically assign the chaining .pNext
             * pointer per sequence: (where it will automatically assign the
             * .pNext pointer per feature specified in it's sequence)
             *
             * 1.) current.pNext = &descriptor_indexing
             * 2.) descriptor_indexing.pNext = &dynamic_rendering
             *
             */
            auto* current = reinterpret_cast<VkBaseOutStructure*>(&head);
            ((current->pNext = reinterpret_cast<VkBaseOutStructure*>(&tail),
              current = reinterpret_cast<VkBaseOutStructure*>(&tail)),
             ...);
            current->pNext = nullptr;
        }

    private:
        std::tuple<Features...> m_data;
    };

};