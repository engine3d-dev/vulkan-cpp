#pragma once
#include <vulkan-cpp/types.hpp>
#include <vulkan/vulkan.h>
#include <vector>

namespace vk {
    /**
     * @param sources holds data the shader source and stage the source it corresponds to.
     * @param vertex_attributes are the vertex attributes that are used to setup vulkan vertex attributes and the binding attributes.
    */
    struct shader_resource_info {
        std::span<const shader_source> sources{};
        std::span<const vertex_attribute> vertex_attributes{};
    };

    /**
     * @brief shader_resource acts as a unit of shader sources that a vulkan graphics pipeline will associate with
     * 
     * Every graphics pipeline as an associated group of shader sources with their own specific shader stages they are apart of.
     * 
     * @param p_device is the logical device for creating the vulkan handlers
     * @param p_info is the shader_resource properties for providing shader sources to load and what vertex attributes to return from this shader resource
     * 
    */
    class shader_resource {
    public:
        shader_resource();
        shader_resource(const VkDevice& p_device, const shader_resource_info& p_info);

        [[nodiscard]] bool is_valid() const { return m_is_resource_valid; }

        void vertex_attributes(const std::span<const vertex_attribute>& p_attributes);

        //! @return the handlers of vulkan shader modules for each individual shader source loaded altogether
        [[nodiscard]] std::span<const shader_handle> handles() const { return m_shader_module_handlers; }

        //! @return vulkan data spec for vertex attributes
        [[nodiscard]] std::span<const VkVertexInputAttributeDescription> vertex_attributes() const { return m_vertex_attributes; }

        //! @return vulkan data spec for vertex binding attributes
        [[nodiscard]] std::span<const VkVertexInputBindingDescription> vertex_bind_attributes() const { return m_vertex_binding_attributes; }

        //! @brief used for explicit cleanup for this resource
        void destroy();

    private:
        VkDevice m_device=nullptr;
        bool m_is_resource_valid=false;
        std::vector<VkVertexInputAttributeDescription> m_vertex_attributes;
        std::vector<VkVertexInputBindingDescription> m_vertex_binding_attributes;
        std::vector<shader_handle> m_shader_module_handlers;
    };
};