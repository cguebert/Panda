#version 400

uniform sampler2D texS;
uniform sampler2D texD;

uniform float opacity;

void main(void) 
{
	vec4 Sp = texture(texS, gl_TexCoord[0].st);
	vec3 Sc = Sp.rgb;
	float Sa = Sp.a * opacity;
	vec4 Dp = texture(texD, gl_TexCoord[0].st);
	vec3 Dc = Dp.rgb;
	float Da = Dp.a;
	
	float Ra = Sa + Da * (1-Sa);
	vec3 Rc = Sc*Sa + Dc*Da*(1-Sa);
	
	if(Ra > 0)
		gl_FragColor = vec4(Rc / Ra, Ra);
	else
		gl_FragColor = vec4(0);
}