#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inNormals;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoords;
layout(location = 2) out vec3 fragNormals;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

struct mesh_uniforms {
    vec4 color;
    mat4 model;
};

layout(set = 1, binding = 0) readonly buffer geometry_data {
    mesh_uniforms uniforms[];
};

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoords = inTexCoords;
    fragNormals = inNormals;
}