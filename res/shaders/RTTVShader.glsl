#version 310 es

layout(location = 0) in vec4 aPos;

uniform mat4 u_M;
uniform mat4 u_P;

void main()
{
	gl_Position = u_P * u_M * aPos;
}
