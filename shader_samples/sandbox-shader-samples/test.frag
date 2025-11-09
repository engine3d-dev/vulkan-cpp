#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoords;
layout(location = 2) in vec3 fragNormals;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D diffuse_texture;
layout(set = 1, binding = 2) uniform sampler2D specular_texture;

void main() {
    vec4 diffuse = texture(diffuse_texture, fragTexCoords);
    // vec4 specular = texture(specular_texture, fragTexCoords);

    outColor = texture(diffuse_texture, fragTexCoords);


}