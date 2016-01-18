#version 330

in vec2 vertex;

uniform mat4 MVP;

void main(void)
{
	gl_Position = MVP * vec4(vertex, 0, 1);
}
