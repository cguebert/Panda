#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec4 color;

uniform mat4 MVP;

out vec4 f_color;

void main(void)
{
	f_color = color;
	gl_Position = MVP * vec4(position, 0, 1);
}
