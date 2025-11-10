#version 430

layout(location = 0) in vec3 inPosition; // vertex of skybox

layout(location = 0) out vec3 localPos;

layout(set = 0, binding = 0) uniform uniform_data {
    mat4 view;
    mat4 projection;
} ubo;

void main() {
    localPos = inPosition;
    gl_Position = ubo.projection * ubo.view * vec4(inPosition, 1.0);
}