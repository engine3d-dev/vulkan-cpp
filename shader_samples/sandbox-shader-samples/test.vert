#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inNormals;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoords;
layout(location = 2) out vec3 fragNormals;
layout(location = 3) out vec3 FragPos;

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 proj_view;
} ubo;

layout(set = 1, binding = 0) uniform geometry_ubo {
    mat4 model;
    vec4 color;
} geometry_src;



void main() {
    FragPos = vec3(geometry_src.model * vec4(inPosition, 1.0));
    gl_Position = (ubo.proj_view * geometry_src.model) * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoords = inTexCoords;
    fragNormals = inNormals;
}