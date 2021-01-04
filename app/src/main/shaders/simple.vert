#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec4 i_position;
layout(location = 1) in vec4 i_color;
layout(location = 2) in vec2 i_tex_coords;

layout(location = 0) out vec3 o_color;
layout(location = 1) out vec2 o_tex_coords;

void main(){
    gl_Position = ubo.proj * ubo.view * ubo.model * i_position;
    o_color = i_color.rgb;
    o_tex_coords = i_tex_coords;
}