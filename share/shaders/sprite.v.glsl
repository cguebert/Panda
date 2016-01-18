#version 330

in vec2 position;
in float size;
in vec4 color;

uniform mat4 MVP;

out vec4 f_color;

void main(void)
{
	f_color = color;
	gl_Position = MVP * vec4(position, 0, 1);
	gl_PointSize = max(1.0, size);
}
