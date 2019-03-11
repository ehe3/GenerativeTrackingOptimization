#version 460 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 boneweights;

out float depth;

uniform mat4 u_M;
uniform mat4 u_P;

uniform mat4 toe_rot;
uniform mat4 leg_rot;

void main()
{
	mat4 bonetransform = boneweights.z*toe_rot;
	bonetransform = bonetransform + boneweights.x*leg_rot;
	//bonetransform = bonetransform + (boneweights.y+boneweights.w)*mat4(1.0);
	bonetransform = bonetransform + (boneweights.y+boneweights.w)*mat4(1.0);
	gl_Position = u_P * u_M * bonetransform*aPos;
}
