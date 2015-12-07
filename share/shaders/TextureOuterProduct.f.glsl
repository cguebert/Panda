#version 400

in vec2 f_texCoord;

uniform sampler2D tex0;
uniform sampler2D tex1;

out vec4 color; // layout(location = 0)

void main(void)
{
	color = texture(tex0, vec2(f_texCoord.s, 0)) * texture(tex1, vec2(f_texCoord.t, 0));
}
