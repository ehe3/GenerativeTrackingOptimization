#version 310 es

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 u_M;
uniform mat4 u_V;
uniform mat4 u_P;

out vec2 TexCoords;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = u_P * u_V *	u_M * aPos;
}
