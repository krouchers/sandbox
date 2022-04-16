#version 450

layout(binding = 0) uniform pvm_matrix {
    mat4 model;
    mat4 view;
    mat4 proj;
} pvm;

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 in_tex_coord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 out_tex_coord;

void main() {
    gl_Position = pvm.proj * pvm.view * pvm.model * vec4(positions, 1);
    fragColor = color;
    out_tex_coord = in_tex_coord;
}