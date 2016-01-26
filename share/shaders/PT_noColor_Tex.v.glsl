#version 330

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoord;

uniform mat4 MVP;

out vec2 f_texCoord;

void main(void)
{
	f_texCoord = texCoord;
	gl_Position = MVP * vec4(position, 0, 1);
}
