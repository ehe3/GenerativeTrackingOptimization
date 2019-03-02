#version 460 core

layout(location = 0) in vec4 aPos;

out float depth;

uniform mat4 u_M;
uniform mat4 u_P;

void main()
{
	gl_Position = u_P * u_M * aPos;
}
