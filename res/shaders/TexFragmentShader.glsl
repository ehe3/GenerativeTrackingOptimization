#version 310 es

precision mediump float;

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D gendepTexture;

void main()
{
	vec4 ref = texture(screenTexture, TexCoords);
	vec4 rend = texture(gendepTexture, TexCoords);
	if (ref.x > rend.x)
	{
		FragColor = ref - rend;
	}
	else 
	{
		FragColor = rend - ref;
	}
}
