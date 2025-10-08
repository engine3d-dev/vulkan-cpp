#version 450

layout(location = 1) in vec3 forward; // forward vector

layout(set = 1, binding = 0) uniform samplerCube skybox_cube;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(skybox_cube, forward);
}