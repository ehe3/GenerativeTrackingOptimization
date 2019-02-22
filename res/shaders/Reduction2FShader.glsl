#version 310 es

precision mediump float;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex;
uniform float width;
uniform float height;

void main()
{
	int x = int(TexCoords.x * (width - 1.0));
	int y = int((TexCoords.y - 0.5) * (height - 1.0));
	float c00 = texelFetch(tex, ivec2(2*x, 2*y), 0).z;
	float c01 = texelFetch(tex, ivec2(2*x, 2*y+1), 0).z;
	float c10 = texelFetch(tex, ivec2(2*x+1, 2*y), 0).z;
	float c11 = texelFetch(tex, ivec2(2*x+1, 2*y+1), 0).z;
	gl_FragDepth = (c00 + c01 + c10 + c11) / 4.0;
}
