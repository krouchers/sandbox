#version 450

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texture_coord;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(tex_sampler, texture_coord).rgb, 1.0);
}