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
	int y = int((TexCoords.y - 2.0/3.0) * (height - 1.0));
	float c00 = texelFetch(tex, ivec2(4*x, 3*y), 0).z;
	float c01 = texelFetch(tex, ivec2(4*x, 3*y), 0).z;
	float c02 = texelFetch(tex, ivec2(4*x, 3*y), 0).z;
	float c10 = texelFetch(tex, ivec2(4*x+1, 3*y), 0).z;
	float c11 = texelFetch(tex, ivec2(4*x+1, 3*y+1), 0).z;
	float c12 = texelFetch(tex, ivec2(4*x+1, 3*y+2), 0).z;
	float c20 = texelFetch(tex, ivec2(4*x+2, 3*y), 0).z;
	float c21 = texelFetch(tex, ivec2(4*x+2, 3*y+1), 0).z;
	float c22 = texelFetch(tex, ivec2(4*x+2, 3*y+2), 0).z;
	float c30 = texelFetch(tex, ivec2(4*x+3, 3*y), 0).z;
	float c31 = texelFetch(tex, ivec2(4*x+3, 3*y+1), 0).z;
	float c32 = texelFetch(tex, ivec2(4*x+3, 3*y+2), 0).z;
	gl_FragDepth = (c00 + c01 + c02 + c10 + c11 + c12 + c20 + c21 + c22 + c30 + c31 + c32) / 12.0;	
}
