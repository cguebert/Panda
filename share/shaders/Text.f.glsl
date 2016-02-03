#version 330

in vec2 f_texCoord;
in vec4 f_color;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
	float a = texture(tex0, f_texCoord).r;
    fragColor = vec4(f_color.rgb, f_color.a * a);
}
