#version 330

in vec2 f_texCoord;

uniform sampler2D texS; // Source
uniform sampler2D texD; // Destination

uniform float opacity;
uniform int mode;

out vec4 color; // layout(location = 0)

void main(void)
{
	vec4 Sp = texture(texS, f_texCoord);
	float Sa = Sp.a * opacity;	// Source alpha
	vec3 Sca = Sp.rgb * Sa;		// Premultiplied source color
	vec4 Dp = texture(texD, f_texCoord);
	float Da = Dp.a;			// Destination alpha
	vec3 Dca = Dp.rgb * Da;		// Premultiplied destination color

	float Ra = 0;				// Result alpha
	vec3 Rca = vec3(0);			// Premultiplied result color
	if(mode == 0) // SourceOver
	{
		Rca = Sca + Dca * (1 - Sa);
		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 1) // DestinationOver
	{
		Rca = Dca + Sca * (1 - Da);
		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 2) // Clear
	{ // All zero
	}

	else if(mode == 3) // Source
	{
		Rca = Sca;
		Ra = Sa;
	}

	else if(mode == 4) // Destination
	{
		Rca = Dca;
		Ra = Da;
	}

	else if(mode == 5) // SourceIn
	{
		Rca = Sca * Da;
		Ra = Sa * Da;
	}

	else if(mode == 6) // DestinationIn
	{
		Rca = Dca * Sa;
		Ra = Sa * Da;
	}

	else if(mode == 7) // SourceOut
	{
		Rca = Sca * (1 - Da);
		Ra = Sa * (1 - Da);
	}

	else if(mode == 8) // DestinationOut
	{
		Rca = Dca * (1 - Sa);
		Ra = Da * (1 - Sa);
	}

	else if(mode == 9) // SourceAtop
	{
		Rca = Sca * Da + Dca * (1 - Sa);
		Ra = Da;
	}

	else if(mode == 10) // DestinationAtop
	{
		Rca = Dca * Sa + Sca * (1 - Da);
		Ra = Sa;
	}

	else if(mode == 11) // Xor
	{
		Rca = Sca * (1 - Da) + Dca * (1 - Sa);
		Ra = Sa + Da - 2 * Sa * Da;
	}

	else if(mode == 12) // Plus
	{
		Rca = Sca + Dca;
		Ra = Sa + Da;
	}

	else if(mode == 13) // Multiply
	{
		Rca = Sca * Dca + Sca * (1 - Da) + Dca * (1 - Sa);
		Ra = Sa + Da - Sa * Da;
	}

	else if(mode == 14) // Screen
	{
		Rca = Sca + Dca - Sca * Dca;
		Ra = Sa + Da - Sa * Da;
	}

	else if(mode == 15) // Overlay
	{
		for(int i=0; i<3; ++i)
		{
			if(2 * Dca[i] <= Da)
				Rca[i] = 2 * Sca[i] * Dca[i] + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
			else
				Rca[i] = Sca[i] * (1 + Da) + Dca[i] * (1 + Sa) - 2 * Dca[i] * Sca[i] - Da * Sa;
		}

		Ra = Sa + Da - Sa * Da;
	}

	else if(mode == 16) // Darken
	{
		for(int i=0; i<3; ++i)
		{
			if(Sca[i] * Da < Dca[i] * Sa)
				Rca[i] = Sca[i] + Dca[i] * (1 - Sa);
			else
				Rca[i] = Dca[i] + Sca[i] * (1 - Da);
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 17) // Lighten
	{
		for(int i=0; i<3; ++i)
		{
			if(Sca[i] * Da > Dca[i] * Sa)
				Rca[i] = Sca[i]+ Dca[i] * (1 - Sa);
			else
				Rca[i] = Dca[i] + Sca[i] * (1 - Da);
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 18) // ColorDodge
	{
		for(int i=0; i<3; ++i)
		{
			if(Sca[i] == Sa && Dca[i] == 0)
				Rca[i] = Sca[i] * (1 - Da);
			else if(Sca[i] == Sa)
				Rca[i] = Sa * Da + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
			else
				Rca[i] = Sa * Da * min(1, Dca[i] / Da * Sa / (Sa - Sca[i])) + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 19) // ColorBurn
	{
		for(int i=0; i<3; ++i)
		{
			if(Sca[i] == 0 && Dca[i] == Da)
				Rca[i] = Sa * Da + Dca[i] * (1 - Sa);
			else if(Sca[i] == 0)
				Rca[i] = Dca[i] * (1 - Sa);
			else
				Rca[i] = Sa * Da * (1 - min(1, (1 - Dca[i] / Da) * Sa / Sca[i])) + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 20) // HardLight
	{
		for(int i=0; i<3; ++i)
		{
			if(2 * Sca[i] <= Sa)
				Rca[i] = 2 * Sca[i] * Dca[i] + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
			else
				Rca[i] = Sca[i] * (1 + Da) + Dca[i] * (1 + Sa) - Sa * Da - 2 * Sca[i] * Dca[i];
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 21) // SoftLight
	{
		for(int i=0; i<3; ++i)
		{
			float m = Dca[i] / Da;
			if(2 * Sca[i] <= Sa)
				Rca[i] = Dca[i] * (Sa + (2 * Sca[i] - Sa) * (1 - m))  + Sca[i] * (1 - Da) + Dca[i] * (1 - Sa);
			else if(2 * Sca[i] > Sa && 4 * Dca[i] <= Da)
				Rca[i] = Da * (2 * Sca[i] - Sa) * (16 * m*m*m - 12 * m*m - 3 * m) + Sca[i] * (1 - Da) + Dca[i];
			else
				Rca[i] = Da * (2 * Sca[i] - Sa) * (sqrt(m) - m) + Sca[i] * (1 - Da) + Dca[i];
		}

		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 22) // Difference
	{
		Rca = Sca + Dca - 2 * min(Sca * Da, Dca * Sa);
		Ra = Sa + Da * (1 - Sa);
	}

	else if(mode == 23) // Exclusion
	{
		Rca = (Sca * Da + Dca * Sa - 2 * Sca * Dca) + Sca * (1 - Da) + Dca * (1 - Sa);
		Ra = Sa + Da - Sa * Da;
	}

	for(int i=0; i<3; ++i)
		Rca[i] = clamp(Rca[i], 0, 1);
	Ra = clamp(Ra, 0, 1);

	if(Ra > 0)
		color = vec4(Rca / Ra, Ra);
	else
		color = vec4(0, 0, 0, 0);
}
