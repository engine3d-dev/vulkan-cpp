#include <vulkan-cpp/shader_resource.hpp>
#include <vulkan-cpp/utilities.hpp>
#include <stdexcept>
#include <filesystem>
#include <fstream>

namespace vk {
    // Reading the raw .spv binaries
    static std::vector<char> read(const std::string& p_file) {
        std::vector<char> out_buffer;
        std::ifstream ins(p_file, std::ios::ate | std::ios::binary);

        if (!ins) {
            return {};
        }

        uint32_t file_size = (uint32_t)ins.tellg();
        out_buffer.resize(file_size);
        ins.seekg(0);
        ins.read(out_buffer.data(), file_size);
        return out_buffer;
    }

    //! @brief Ensure file reads are valid before reading raw .spv binaries
    static std::vector<char> compile_binary_shader_source(
      const shader_source& p_shader_source) {
        std::vector<char> binary_blob{};

        if (std::filesystem::is_regular_file(p_shader_source.filename)) {
            binary_blob = read(p_shader_source.filename);
        }

        return binary_blob;
    }

    shader_resource::shader_resource() {
        throw std::runtime_error("Cannot construct an empty shader_resource -- default constructor called");
    }
    
    shader_resource::shader_resource(const VkDevice& p_device, const shader_resource_info& p_info) : m_device(p_device) {
        m_shader_module_handlers.resize(p_info.sources.size());

        for(size_t i = 0; i < p_info.sources.size(); i++) {
            const shader_source shader_src = p_info.sources[i];
            std::vector<char> blob = compile_binary_shader_source(shader_src);

            if(blob.empty()) {
                m_is_resource_valid = false;
                return;
            }

            std::span<char> binary(blob.begin(), blob.end());
            VkShaderModuleCreateInfo shader_module_ci = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .pNext = nullptr,
                .codeSize = static_cast<uint32_t>(binary.size_bytes()),
                .pCode = (const uint32_t*)binary.data()
            };
            
            // Setting m_shader_module_handlers[i]'s stage and the VkShaderModule handle altogether
            vk_check(vkCreateShaderModule(m_device, &shader_module_ci, nullptr, &m_shader_module_handlers[i].module),"vkCreateShaderModule");
            m_shader_module_handlers[i].stage = shader_src.stage;
        }

        m_is_resource_valid = true;
    }

    void shader_resource::vertex_attributes(const std::span<const vertex_attribute>& p_attributes) {
        /*
            -- These comments are a reminder to myself --
            - this function simplifies the need to separately define vertex attributes and the vertex binding attributes as shown below:
            
            - vertex attributes specify the types of data within the vertex
            
            - vertex binding attribute specifies the rate of reading that data layout specified by the vertex attributes

            - Interpret the following vertex attributes below with this shader code with `layout(location = n)` specified
            where by default these are set to binding zero by the shader

            layout(location = 0) in vec3 inPosition;
            layout(location = 1) in vec3 inColor;
            layout(location = 2) in vec3 inNormals;
            layout(location = 3) in vec2 inTexCoords;

            m_shader_group.set_vertex_attributes(VkVertexInputAttributeDescription{
                { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, position), },
                { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, color), },
                { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(vk::vertex, normals), },
                { .location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT,    .offset = offsetof(vk::vertex, uv), },
            });

            m_shader_group.set_vertex_bind_attributes(VkVertexInputBindingDescription{
                {.binding = 0, .stride = sizeof(vk::vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,},
            });

            Which gets handled in specifying the following below
        */

        m_vertex_binding_attributes.resize(p_attributes.size());

        for(size_t i = 0; i < m_vertex_binding_attributes.size(); i++) {
            // setting up vertex binding
            const vertex_attribute attribute = p_attributes[i];
            m_vertex_attributes.resize(attribute.entries.size());
            m_vertex_binding_attributes[i] = {
                .binding = attribute.binding,
                .stride = attribute.stride,
                .inputRate = to_input_rate(attribute.input_rate)
            };

            // then setting up the vertex attributes for the vertex data layouts
            for(size_t j = 0; j < attribute.entries.size(); j++) {
                const vertex_attribute_entry entry = attribute.entries[j];
                m_vertex_attributes[j] = {
                    .location = entry.location,
                    .binding = attribute.binding,
                    .format = to_format(entry.format),
                    .offset = entry.stride
                };
            }
        }
    }

    void shader_resource::destroy() {
        for(auto& handle : m_shader_module_handlers) {
            if(handle.module != nullptr) {
                vkDestroyShaderModule(m_device, handle.module, nullptr);
            }
        }
    }
};