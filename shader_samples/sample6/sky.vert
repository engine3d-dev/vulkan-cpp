#version 450

layout(set = 0, binding = 0) uniform CameraVectors{
    vec4 forward;
    vec4 right;
    vec4 up;
} CameraData;

layout(location = 0) out vec3 forward; // forward direction

const vec2 screen_corners[6] = vec2[](
    vec2(-1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, -1.0),
    vec2(-1.0, -1.0)
);

void main() {
    vec2 pos = screen_corners[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    forward = normalize(CameraData.forward + pos.x * CameraData.right - pos.y * CameraData.up).xyz;
}