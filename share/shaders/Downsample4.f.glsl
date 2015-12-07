#version 400

in vec2 f_texCoord;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
	vec3 col0 = textureOffset(tex0, f_texCoord, ivec2( -2,  0 ) ).xyz;
	vec3 col1 = textureOffset(tex0, f_texCoord, ivec2(  2,  0 ) ).xyz;
	vec3 col2 = textureOffset(tex0, f_texCoord, ivec2(  0, -2 ) ).xyz;
	vec3 col3 = textureOffset(tex0, f_texCoord, ivec2(  0,  2 ) ).xyz;

	vec3 col = (col0+col1+col2+col3) * 0.25;
	fragColor = vec4(col.xyz, 1.0);
}
