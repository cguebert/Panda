#version 330

in vec2 f_texCoord;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(tex0, f_texCoord);
}
