#version 400

in vec2 vertex;
in vec2 texCoord;

uniform mat4 MVP;
uniform float radiusScale;

out vec2 f_texCoord;
out vec2 f_blurTexCoords[14];

void main()
{
	gl_Position = MVP * vec4(vertex, 0, 1);
	f_texCoord = texCoord;
	vec2 dir = vec2(radiusScale, 0);
	f_blurTexCoords[ 0] = f_texCoord + dir * -7;
	f_blurTexCoords[ 1] = f_texCoord + dir * -6;
	f_blurTexCoords[ 2] = f_texCoord + dir * -5;
	f_blurTexCoords[ 3] = f_texCoord + dir * -4;
	f_blurTexCoords[ 4] = f_texCoord + dir * -3;
	f_blurTexCoords[ 5] = f_texCoord + dir * -2;
	f_blurTexCoords[ 6] = f_texCoord + dir * -1;
	f_blurTexCoords[ 7] = f_texCoord + dir *  1;
	f_blurTexCoords[ 8] = f_texCoord + dir *  2;
	f_blurTexCoords[ 9] = f_texCoord + dir *  3;
	f_blurTexCoords[10] = f_texCoord + dir *  4;
	f_blurTexCoords[11] = f_texCoord + dir *  5;
	f_blurTexCoords[12] = f_texCoord + dir *  6;
	f_blurTexCoords[13] = f_texCoord + dir *  7;
}
