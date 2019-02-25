#version 310 es

precision mediump float;

in vec2 TexCoords;

uniform sampler2D tex;

out vec4 FragColor;

void main()
{
	gl_FragDepth = texture(tex, TexCoords).z;
}
