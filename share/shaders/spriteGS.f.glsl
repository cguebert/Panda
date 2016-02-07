#version 330 core

in GS_OUT {
    vec4 color;
    vec2 texCoords;
} fs_in;

uniform sampler2D tex0;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(tex0, fs_in.texCoords) * fs_in.color;
}
