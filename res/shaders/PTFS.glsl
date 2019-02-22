#version 310 es

precision mediump float;

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex;

void main()
{
	FragColor = texture(tex, TexCoords);
	//int x = int(TexCoords.x * 199.0);
	//	//FragColor = vec4(TexCoords.y, TexCoords.y, TexCoords.y, 1.0);
	//	}}
}
