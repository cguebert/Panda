#version 440

in vec4 f_color;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(tex0, vec2(gl_PointCoord.x, 1-gl_PointCoord.y)) * f_color;
}
