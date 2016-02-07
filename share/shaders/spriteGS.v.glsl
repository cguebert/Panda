#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;
layout (location = 2) in float size;

uniform mat4 MVP;

out VS_OUT {
    float size;
    vec4 color;
} vs_out;

void main(void)
{
    vs_out.size = size;
    vs_out.color = color;
    gl_Position = MVP * vec4(position, 0, 1);
}
