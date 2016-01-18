#version 330

in vec2 vertex;
in vec2 texCoord;

uniform mat4 MVP;

out vec2 f_texCoord;

void main(void)
{
	f_texCoord = texCoord;
	gl_Position = MVP * vec4(vertex, 0, 1);
}
