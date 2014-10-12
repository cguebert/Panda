#version 440

in vec2 vertex;
in vec2 texCoord;

out vec2 f_texCoord;

uniform mat4 MVP;

void main(void) 
{
	f_texCoord = texCoord;
	gl_Position = MVP * vec4(vertex, 0, 1);
}
