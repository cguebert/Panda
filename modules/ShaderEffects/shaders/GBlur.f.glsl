#version 400

in vec2 f_texCoord;

uniform sampler2D tex0;
uniform vec2 pixelOffset;

out vec4 fragColor;

void main()
{
	// Kernel width 35 x 35
	const int stepCount = 9;
	const float gWeights[stepCount] ={
	   0.10855,
	   0.13135,
	   0.10406,
	   0.07216,
	   0.04380,
	   0.02328,
	   0.01083,
	   0.00441,
	   0.00157
	};
	const float gOffsets[stepCount] ={
	   0.66293,
	   2.47904,
	   4.46232,
	   6.44568,
	   8.42917,
	   10.41281,
	   12.39664,
	   14.38070,
	   16.36501
	};

	vec3 colOut = vec3( 0, 0, 0 );
	for( int i = 0; i < stepCount; i++ )
	{
		vec2 texCoordOffset = gOffsets[i] * pixelOffset;
		vec3 col = texture(tex0, f_texCoord + texCoordOffset).xyz + texture(tex0, f_texCoord - texCoordOffset).xyz;
		colOut += gWeights[i] * col;
	}

	fragColor = vec4(colOut, 1);
}
