#version 400

in vec2 f_texCoord;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
	fragColor = vec4(texture(tex0, f_texCoord).xyz, 1.0);
}
