#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoords;
layout(location = 2) in vec3 fragNormals;
layout(location = 3) in vec3 FragPos;

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 1) uniform sampler2D diffuse_texture;
layout(set = 1, binding = 2) uniform sampler2D specular_texture;

void main() {
    float shininess = 64.0;
    vec3 ambient_value = vec3(0.2, 0.2, 0.2);
    vec3 diffuse_value = vec3(0.5, 0.5, 0.5);
    vec3 specular_value = vec3(1.0, 1.0, 1.0);

    // NOT PART OF THIS CODE
    // vec4 diffuse_test = texture(diffuse_texture, fragTexCoords);
    // vec4 specular_test = texture(specular_texture, fragTexCoords);
    vec3 ambient = ambient_value * texture(diffuse_texture, fragTexCoords).rgb;

    // // apply diffuse
    vec3 norm = normalize(fragNormals);
    vec3 lightPos = vec3(0, 0, 0); // TODO: Replace this with a uniform that passes in the light position
    vec3 light_direction = normalize(lightPos - FragPos);
    float diff = max(dot(norm, light_direction), 0.0);
    vec3 diffuse = diffuse_value * diff * texture(diffuse_texture, fragTexCoords).rgb;

    // apply specular
    vec3 viewPos = vec3(0.0, 0.0, 0.0); // TODO: Replace this with a dynamic uniform
    vec3 view_direction = normalize(viewPos - FragPos);
    vec3 reflect_direction = reflect(-light_direction, norm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), shininess);
    vec3 specular = specular_value * spec * texture(specular_texture, fragTexCoords).rgb;
    
    // vec4 lighting_result = vec4(ambient + diffuse + specular, 1.0);
    vec3 result = ambient + diffuse + specular;

    vec4 color = vec4(result, 1.0);
    outColor = color;


}