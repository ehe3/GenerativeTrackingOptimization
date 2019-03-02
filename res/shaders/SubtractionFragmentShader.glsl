#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D gendepTexture;

void main()
{
	//FragColor = texture(screenTexture, TexCoords);
	vec4 ref = texture(screenTexture, TexCoords);
	vec4 rend = texture(gendepTexture, TexCoords);
	if (ref.z > rend.z)
	{
		gl_FragDepth = ref.z - rend.z;
	}
	else 
	{
		gl_FragDepth = rend.z - ref.z;
	}
}
