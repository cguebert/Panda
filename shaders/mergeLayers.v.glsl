#version 400

uniform mat4 MVP;

void main(void) 
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = MVP * vec4(gl_Vertex.xy, 0, 1);
}
