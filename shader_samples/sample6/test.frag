#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoords;
layout(location = 2) in vec3 fragNormals;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D texture_image;
// layout(set = 0, binding 2) uniform samplerCube skybox_texture;

void main() {
    // outColor = vec4(fragColor, 1.0);

    // Adding texture
    outColor = texture(texture_image, fragTexCoords);

}