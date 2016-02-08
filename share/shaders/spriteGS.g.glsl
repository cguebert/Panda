#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    float size;
    vec4 color;
} gs_in[];

uniform mat4 MVP;

out GS_OUT {
    vec4 color;
    vec2 texCoords;
} gs_out;

void main(void)
{
    vec4 pos = gl_in[0].gl_Position;
    float s = gs_in[0].size / 2.0f;
    gs_out.color = gs_in[0].color;
    
    gs_out.texCoords = vec2(1.0f, 1.0f);
    gl_Position = pos + MVP * vec4(s, -s, 0.0f, 0.0f);  // top right   
    EmitVertex();   
    
    gs_out.texCoords = vec2(0.0f, 1.0f);
    gl_Position = pos + MVP * vec4(-s, -s, 0.0f, 0.0f); // top left
    EmitVertex();
    
    gs_out.texCoords = vec2(1.0f, 0.0f);
    gl_Position = pos + MVP * vec4(s, s, 0.0f, 0.0f);   // bottom right
    EmitVertex();
    
    gs_out.texCoords = vec2(0.0f, 0.0f);
    gl_Position = pos + MVP * vec4(-s, s, 0.0f, 0.0f);  // bottom left
    EmitVertex();
    EndPrimitive();
}
