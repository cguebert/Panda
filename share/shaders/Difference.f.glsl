#version 330

in vec2 f_texCoord;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 color; // layout(location = 0)

void main(void)
{
	vec4 t0 = texture(tex0, f_texCoord);
	vec4 t1 = texture(tex1, f_texCoord);
	color.rgb = t0.rgb - t1.rgb;
//	color.rgb = (t0.rgb - t1.rgb) / 2 + 0.5;
	color.a = 1;
}
