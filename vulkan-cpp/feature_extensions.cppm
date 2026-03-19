module;

#include <concepts>
#include <type_traits>
#include <tuple>

export module vk::feature_extensions;

namespace vk {
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
     *
     *
     * template<>
     * struct device_feature<VkPhysicalDeviceDescriptorIndexingFeatures> :
     * public std::true_type {};
     */
    template<typename T>
    concept ExtensionConcept = requires(T t) {
        { t.sType } -> std::convertible_to<VkStructureType>;
        { t.pNext } -> std::convertible_to<void*>;
    };

    // template<ExtensionConcept T, ExtensionConcept... Args>
    // void chain_features(T& head, Args&... tail) {
    //     /**
    //      * @brief Passing an arbitrary sequences of VkPhysicalDevice*Features
    //      *
    //      *
    //      * @brief Handled using Variadic Templates
    //      * Compiler does not generate a loop, rather it'll generate a flat
    //      * sequence of instructions.
    //      *
    //      * Essentially doing something like:
    //      chain_features(descriptor_indexing,
    //      * dynamic_rendering).
    //      *
    //      * The compiler will automatically assign the chaining .pNext pointer
    //      * per sequence: (where it will automatically assign the .pNext
    //      pointer
    //      * per feature specified in it's sequence) 1.) current.pNext =
    //      * &descriptor_indexing 2.) descriptor_indexing.pNext =
    //      * &dynamic_rendering
    //      *
    //      */
    //     auto* current = reinterpret_cast<VkBaseOutStructure*>(&head);
    //     ((current->pNext = reinterpret_cast<VkBaseOutStructure*>(&tail),
    //       current = reinterpret_cast<VkBaseOutStructure*>(&tail)),
    //      ...);
    //     current->pNext = nullptr;
    // }

    template<typename T, VkStructureType STYPE>
    struct feature_type : public T {
        explicit feature_type(T p_intiializer)
          : T(p_intiializer) {
            this->sType = STYPE;
        }
    };

    // TEMP: Use for testing
    using descriptor_indexing_feature = feature_type<
      VkPhysicalDeviceDescriptorIndexingFeatures,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES>;
    using dynamic_rendering = feature_type<
      VkPhysicalDeviceDynamicRenderingFeatures,
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES>;

    template<ExtensionConcept... Features>
    struct features {
    public:
        features(Features&&... p_args)
          : m_data(std::forward<Features>(p_args)...) {

            // We take our unpacked structure of vulkan features
            // Then we chain the pNext pointer altogether for each feature we
            // unpack
            if constexpr (sizeof...(Features) > 0) {
                std::apply([](auto&... spec) { chain_features(spec...); },
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
         */
        [[nodiscard]] void* get_head() noexcept {
            if constexpr (sizeof...(Features) > 0) {
                return &std::get<0>(m_data);
            }

            return nullptr;
        }

    private:
        template<typename T>
        void chain_pointers(T& head, Features&... tail) {
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
             * .pNext pointer per feature specified in it's sequence) 1.)
             * current.pNext = &descriptor_indexing 2.)
             * descriptor_indexing.pNext = &dynamic_rendering
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