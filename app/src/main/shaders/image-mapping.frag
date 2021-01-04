#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D tex_sampler;

layout(location = 0) in vec3 i_color;
layout(location = 1) in vec2 i_tex_coords;

layout(location = 0) out vec4 o_color;

void main() {
    o_color = texture(tex_sampler, i_tex_coords) + 0.1f;
}
