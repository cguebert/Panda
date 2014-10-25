in vec2 f_texCoord;
in vec2 f_blurTexCoords[14];

uniform sampler2D tex0;

out vec4 fragColor;

void main()
{
	fragColor = vec4(0.0);
	fragColor += texture2D(tex0, f_blurTexCoords[ 0]) * 0.0044299121055113265;
	fragColor += texture2D(tex0, f_blurTexCoords[ 1]) * 0.00895781211794;
	fragColor += texture2D(tex0, f_blurTexCoords[ 2]) * 0.0215963866053;
	fragColor += texture2D(tex0, f_blurTexCoords[ 3]) * 0.0443683338718;
	fragColor += texture2D(tex0, f_blurTexCoords[ 4]) * 0.0776744219933;
	fragColor += texture2D(tex0, f_blurTexCoords[ 5]) * 0.115876621105;
	fragColor += texture2D(tex0, f_blurTexCoords[ 6]) * 0.147308056121;
	fragColor += texture2D(tex0, f_texCoord         ) * 0.159576912161;
	fragColor += texture2D(tex0, f_blurTexCoords[ 7]) * 0.147308056121;
	fragColor += texture2D(tex0, f_blurTexCoords[ 8]) * 0.115876621105;
	fragColor += texture2D(tex0, f_blurTexCoords[ 9]) * 0.0776744219933;
	fragColor += texture2D(tex0, f_blurTexCoords[10]) * 0.0443683338718;
	fragColor += texture2D(tex0, f_blurTexCoords[11]) * 0.0215963866053;
	fragColor += texture2D(tex0, f_blurTexCoords[12]) * 0.00895781211794;
	fragColor += texture2D(tex0, f_blurTexCoords[13]) * 0.0044299121055113265;
}
