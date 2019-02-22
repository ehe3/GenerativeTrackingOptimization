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
	int y = int((TexCoords.y - 0.8) * (height - 1.0));
	float c00 = texelFetch(tex, ivec2(5*x, 5*y), 0).z; 
	float c01 = texelFetch(tex, ivec2(5*x, 5*y+1), 0).z; 
	float c10 = texelFetch(tex, ivec2(5*x+1, 5*y), 0).z;
	float c11 = texelFetch(tex, ivec2(5*x+1, 5*y+1), 0).z;
	float c02 = texelFetch(tex, ivec2(5*x, 5*y+2), 0).z;
	float c12 = texelFetch(tex, ivec2(5*x+1, 5*y+2), 0).z;
	float c22 = texelFetch(tex, ivec2(5*x+2, 5*y+2), 0).z;
	float c21 = texelFetch(tex, ivec2(5*x+2, 5*y+1), 0).z;
	float c20 = texelFetch(tex, ivec2(5*x+2, 5*y), 0).z;
	float c03 = texelFetch(tex, ivec2(5*x, 5*y+3), 0).z;
	float c13 = texelFetch(tex, ivec2(5*x+1, 5*y+3), 0).z;
	float c23 = texelFetch(tex, ivec2(5*x+2, 5*y+3), 0).z;
	float c33 = texelFetch(tex, ivec2(5*x+3, 5*y+3), 0).z;
	float c32 = texelFetch(tex, ivec2(5*x+3, 5*y+2), 0).z;
	float c31 = texelFetch(tex, ivec2(5*x+3, 5*y+1), 0).z;
	float c30 = texelFetch(tex, ivec2(5*x+3, 5*y), 0).z;
	float c04 = texelFetch(tex, ivec2(5*x, 5*y+4), 0).z;
	float c14 = texelFetch(tex, ivec2(5*x+1, 5*y+4), 0).z;
	float c24 = texelFetch(tex, ivec2(5*x+2, 5*y+4), 0).z;
	float c34 = texelFetch(tex, ivec2(5*x+3, 5*y+4), 0).z;
	float c44 = texelFetch(tex, ivec2(5*x+4, 5*y+4), 0).z;
	float c43 = texelFetch(tex, ivec2(5*x+4, 5*y+3), 0).z;
	float c42 = texelFetch(tex, ivec2(5*x+4, 5*y+2), 0).z;
	float c41 = texelFetch(tex, ivec2(5*x+4, 5*y+1), 0).z;
	float c40 = texelFetch(tex, ivec2(5*x+4, 5*y), 0).z;

	gl_FragDepth = (c00 + c01 + c02 + c03 + c04 + c10 + c11 + c12 + c13 + c14 + c20 + c21 + c22 + c23 + c24 + c30 + c31 + c32 + c33 + c34 + c40 + c41 + c42 + c43 + c44) / 25.0;




}
