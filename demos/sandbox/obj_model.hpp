#define GLM_FORCE_RADIANS
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// loading tinyobjloader library here
#include <tiny_obj_loader.h>
#include "uniforms.hpp"
#include <vulkan-cpp/vertex_buffer.hpp>
#include <print>
#include <filesystem>

template<typename T, typename... Rest>
void hash_combine(size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed << 2);
    (hash_combine(seed, rest), ...);
}


namespace std {

    template<>
    struct hash<vk::vertex_input> {
        size_t operator()(const vk::vertex_input& vertex) const {
            size_t seed = 0;
            hash_combine(
              seed, vertex.position, vertex.color, vertex.normals, vertex.uv);
            return seed;
        }
    };
}

// This is how we are going to load a .obj model for this demo
// Example of how you might want to have your own classes for loading geometry-meshes
class obj_model {
public:
    obj_model() = default;
    obj_model(const std::filesystem::path& p_filename, const VkDevice& p_device, const vk::physical_device& p_physical, bool p_flip=false) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        //! @note If we return the constructor then we can check if the mesh
        //! loaded successfully
        //! @note We also receive hints if the loading is successful!
        //! @note Return default constructor automatically returns false means
        //! that mesh will return the boolean as false because it wasnt
        //! successful
        if (!tinyobj::LoadObj(&attrib,
                              &shapes,
                              &materials,
                              &warn,
                              &err,
                              p_filename.string().c_str())) {
            std::println("Could not load model from path {}", p_filename.string());
            m_is_loaded = false;
            return;
        }

        std::vector<vk::vertex_input> vertices;
        std::vector<uint32_t> indices;
        std::unordered_map<vk::vertex_input, uint32_t> unique_vertices{};

        std::println("p_flip = {}", p_flip);

        for(size_t i = 0; i < shapes.size(); i++) {
            auto shape = shapes[i];
            // for (const auto& index : shape.mesh.indices) {
            for(size_t j = 0; j < shape.mesh.indices.size(); j++) {
                auto index = shape.mesh.indices[j];
                ::vk::vertex_input vertex{};

                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                if (index.vertex_index >= 0) {
                    vertex.position = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    vertex.color = {
                        attrib.colors[3 * index.vertex_index + 0],
                        attrib.colors[3 * index.vertex_index + 1],
                        attrib.colors[3 * index.vertex_index + 2]
                    };
                }

                if (!attrib.normals.empty()) {
                    vertex.normals = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                if(!attrib.texcoords.empty()) {
                    glm::vec2 flipped_uv = {
                        attrib.texcoords[static_cast<long long>(index.texcoord_index) * 2],
                        1.0f - attrib.texcoords[static_cast<long long>(index.texcoord_index) * 2 + 1],
                    };

                    glm::vec2 original_uv = {
                        attrib.texcoords[static_cast<long long>(index.texcoord_index) * 2],
                        attrib.texcoords[static_cast<long long>(index.texcoord_index) * 2 + 1],
                    };

                    // vertex.uv = m_flip ? flipped_uv : original_uv;
                    if(p_flip) {
                        vertex.uv = flipped_uv;
                    }
                    else {
                        vertex.uv = original_uv;
                    }
                }
                else {
                    vertex.uv = glm::vec2(0.f, 0.f);
                }

                if (!unique_vertices.contains(vertex)) {
                    unique_vertices[vertex] =
                      static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(unique_vertices[vertex]);
            }
        }
        vk::vertex_buffer_settings vertex_info = {
            .phsyical_memory_properties = p_physical.memory_properties(),
            .vertices = vertices
        };

        vk::index_buffer_settings index_info = {
            .phsyical_memory_properties = p_physical.memory_properties(),
            .indices = indices
        };
        m_vertex_buffer = vk::vertex_buffer(p_device, vertex_info);
        m_index_buffer = vk::index_buffer(p_device, index_info);
        m_is_loaded = true;
    }

    [[nodiscard]] bool loaded() const { return m_is_loaded; }

    void bind(const VkCommandBuffer& p_command) {
        m_vertex_buffer.bind(p_command);
        if(m_index_buffer.size() > 0) {
            m_index_buffer.bind(p_command);
        }
    }

    void draw(const VkCommandBuffer& p_command) {
        if(m_index_buffer.size() > 0) {
            vkCmdDrawIndexed(p_command, static_cast<uint32_t>(m_index_buffer.size()), 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(p_command, m_vertex_buffer.size(), 1, 0, 0);
        }
    }

    void destroy() {
        m_vertex_buffer.destroy();
        m_index_buffer.destroy();
    }

private:
    bool m_is_loaded=false;
    vk::vertex_buffer m_vertex_buffer{};
    vk::index_buffer m_index_buffer{};
};