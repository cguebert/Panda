#version 400

uniform sampler2D texS; // Source
uniform sampler2D texD; // Destination

uniform float opacity;
uniform int mode;

void main(void) 
{
	vec4 Sp = texture(texS, gl_TexCoord[0].st);
	float Sa = Sp.a * opacity;	// Source alpha
	vec3 Sca = Sp.rgb * Sa;		// Premultiplied source color
	vec4 Dp = texture(texD, gl_TexCoord[0].st);
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
	/*	if(2 * Dca <= Da)
			Rca = 2 * Sca * Dca + Sca * (1 - Da) + Dca * (1 - Sa);
		else
			Rca = Sca * (1 + Da) + Dca * (1 + Sa) - 2 * Dca * Sca - Da * Sa;
		Ra = Sa + Da - Sa * Da;*/
	}

	else if(mode == 16) // Darken
	{
	/*	if(Sca * Da < Dca * Sa)
		{
			Rca = Sca + Dca * (1 - Sa);
			Ra = Sa + Da * (1 - Sa);
		}
		else
		{
			Rca = Dca + Sca * (1 - Da);
			Ra = Sa + Da * (1 - Sa);
		}*/
	}

	else if(mode == 17) // Lighten
	{
	/*	if(Sca * Da > Dca * Sa)
		{
			Rca = Sca + Dca * (1 - Sa);
			Ra = Sa + Da * (1 - Sa);
		}
		else
		{
			Rca = Dca + Sca * (1 - Da);
			Ra = Sa + Da * (1 - Sa);
		}*/
	}

	else if(mode == 18) // ColorDodge
	{
	/*	if(Sca == Sa && Dca == 0)
			Rca = Sca * (1 - Da);
		else if(Sca == Sa)
			Rca = Sa * Da + Sca * (1 - Da) + Dca * (1 - Sa);
		else if(Sca)
			Rca = Sa * Da * min(1, Dca / Da * Sa / (Sa - Sca)) + Sca * (1 - Da) + Dca * (1 - Sa);

		Ra = Sa + Da * (1 - Sa);*/
	}

	else if(mode == 19) // ColorBurn
	{
	/*	if(Sca == 0 && Dca == Da)
			Rca = Sa * Da + Dca * (1 - Sa);
		else if(Sca == 0)
			Rca = Dca * (1 - Sa);
		else if(Sca)
			Rca = Sa * Da * (1 - min(1, (1 - Dca / Da) * Sa / Sca)) + Sca * (1 - Da) + Dca * (1 - Sa);*/
	}

	else if(mode == 20) // HardLight
	{
	/*	if(2 * Sca <= Sa)
			Rca = 2 * Sca * Dca + Sca * (1 - Da) + Dca * (1 - Sa);
		else
			Rca = Sca * (1 + Da) + Dca * (1 + Sa) - Sa * Da - 2 * Sca * Dca;
		Ra = Sa * Da * (1 - Sa);*/
	}

	else if(mode == 21) // SoftLight
	{
	/*	vec3 m = Dca / Da;
		if(2 * Sca <= Sa)
			Rca = Dca * (Sa + (2 * Sca - Sa) * (1 - m))  + Sca * (1 - Da) + Dca * (1 - Sa);
		else if(2 * Sca > Sa && 4 * Dca <= Da)
			Rca = Da * (2 * Sca - Sa) * (16 * m*m*m - 12 * m*m - 3 * m) + Sca * (1 - Da) + Dca;
		else
			Rca = Da * (2 * Sca - Sa) * (sqrt(m) - m) + Sca - Sca * Da + Dca;
		Ra = Sa + Da * (1 - Sa);*/
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
	
	if(Ra > 0)
		gl_FragColor = vec4(Rca / Ra, Ra);
	else
		gl_FragColor = vec4(0, 0, 0, 0);
}
