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
	vec4 c02 = texture(tex, TexCoords + vec2(0, 2));
	vec4 c12 = texture(tex, TexCoords + vec2(1, 2));
	vec4 c22 = texture(tex, TexCoords + vec2(2, 2));
	vec4 c21 = texture(tex, TexCoords + vec2(2, 1));
	vec4 c20 = texture(tex, TexCoords + vec2(2, 0));
	vec4 c03 = texture(tex, TexCoords + vec2(0, 3));
	vec4 c13 = texture(tex, TexCoords + vec2(1, 3));
	vec4 c23 = texture(tex, TexCoords + vec2(2, 3));
	vec4 c33 = texture(tex, TexCoords + vec2(3, 3));
	vec4 c32 = texture(tex, TexCoords + vec2(3, 2));
	vec4 c31 = texture(tex, TexCoords + vec2(3, 1));
	vec4 c30 = texture(tex, TexCoords + vec2(3, 0));
	vec4 c04 = texture(tex, TexCoords + vec2(0, 4));
	vec4 c14 = texture(tex, TexCoords + vec2(1, 4));
	vec4 c24 = texture(tex, TexCoords + vec2(2, 4));
	vec4 c34 = texture(tex, TexCoords + vec2(3, 4));
	vec4 c44 = texture(tex, TexCoords + vec2(4, 4));
	vec4 c43 = texture(tex, TexCoords + vec2(4, 3));
	vec4 c42 = texture(tex, TexCoords + vec2(4, 2));
	vec4 c41 = texture(tex, TexCoords + vec2(4, 1));
	vec4 c40 = texture(tex, TexCoords + vec2(4, 0));

	FragColor = (c00 + c01 + c02 + c03 + c04 + c10 + c11 + c12 + c13 + c14 + c20 + c21 + c22 + c23 + c24 + c30 + c31 + c32 + c33 + c34 + c40 + c41 + c42 + c43 + c44) / 25.0;




}
