#version 450

layout(binding = 0) uniform pvm_matrix {
    mat4 model;
    mat4 view;
    mat4 proj;
} pvm;

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 in_tex_coord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 out_tex_coord;

const vec3 direction_to_light = normalize(vec3(2.0, 2.0, 1.5));

void main() {

    vec3 world_space_normal = normalize(mat3(pvm.model) * normal);

    float light_intensity = max(dot(world_space_normal, direction_to_light), 0.4);

    gl_Position = pvm.proj * pvm.view * pvm.model * vec4(positions, 1.0f);
    fragColor = light_intensity * color;
    out_tex_coord = in_tex_coord;
}