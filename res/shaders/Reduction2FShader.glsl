#version 310 es

precision mediump float;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex;

void main()
{
	vec4 c00 = texture(tex, TexCoords);
	vec4 c01 = texture(tex, TexCoords + vec2(0, 1));
	vec4 c10 = texture(tex, TexCoords + vec2(1, 0));
	vec4 c11 = texture(tex, TexCoords + vec2(1, 1));

	FragColor = (c00 + c01 + c10 + c11) / 4.0f;

}
