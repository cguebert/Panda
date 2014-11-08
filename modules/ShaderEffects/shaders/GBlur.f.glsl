#version 400

in vec2 f_texCoord;

uniform sampler2D tex0;
uniform vec2 pixelOffset;

out vec4 fragColor;

void main()
{
	const int stepCount = ~~1~~;
	const float gWeights[stepCount] = { ~~2~~ };
	const float gOffsets[stepCount] = { ~~3~~ };

	vec3 colOut = vec3( 0, 0, 0 );
	for( int i = 0; i < stepCount; i++ )
	{
		vec2 texCoordOffset = gOffsets[i] * pixelOffset;
		vec3 col = texture(tex0, f_texCoord + texCoordOffset).xyz + texture(tex0, f_texCoord - texCoordOffset).xyz;
		colOut += gWeights[i] * col;
	}

	fragColor = vec4(colOut, 1);
}
