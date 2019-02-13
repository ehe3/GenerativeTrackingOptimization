#version 310 es

layout(location = 0) in vec4 aPos;

out float depth;

uniform mat4 u_M;
uniform mat4 u_V;
uniform mat4 u_P;

void main()
{
	gl_Position = u_P * u_V * u_M * aPos;
}
