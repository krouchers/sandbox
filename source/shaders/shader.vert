#version 450

layout(binding = 0) uniform pvm_matrix {
    mat4 model;
    mat4 view;
    mat4 proj;
} pvm;

layout(location = 0) in vec2 positions;
layout(location = 1) in vec3 color;
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = pvm.proj * pvm.view * pvm.model * vec4(positions, 0, 1);
    fragColor = color;
}