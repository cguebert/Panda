#version 400

in vec2 vertex;
in vec4 color;

uniform mat4 MVP;

out vec4 f_color;

void main(void)
{
	f_color = color;
	gl_Position = MVP * vec4(vertex, 0, 1);
}
