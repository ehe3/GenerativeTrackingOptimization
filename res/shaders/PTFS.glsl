#version 460 core

precision mediump float;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex;

void main()
{
	FragColor = texture(tex, TexCoords);
}
