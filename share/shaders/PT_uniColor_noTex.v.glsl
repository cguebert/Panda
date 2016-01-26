#version 330

layout (location = 0) in vec2 position;

uniform mat4 MVP;

void main(void)
{
	gl_Position = MVP * vec4(position, 0, 1);
}
