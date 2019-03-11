#version 460 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec4 boneweights;
layout(location = 2) in float aOffset;
layout(location = 3) in mat4 instanceMatrix;
layout(location = 7) in mat4 toerotMatrix;
layout(location = 11) in mat4 legrotMatrix;

uniform int instances;
uniform mat4 u_M;
uniform mat4 u_P;

uniform mat4 m2btoe;
uniform mat4 m2bleg;
uniform mat4 b2mtoe;
uniform mat4 b2mleg;

flat out int instanceid;

void main()
{
	mat4 bonetransform = boneweights.z*b2mtoe*toerotMatrix*m2btoe;
	bonetransform = bonetransform + boneweights.x*b2mleg*legrotMatrix*m2bleg;
	bonetransform = bonetransform + (boneweights.y + boneweights.w)*mat4(1.0);
	vec4 pos = u_P * instanceMatrix *bonetransform*aPos;
	float xPos = pos.x/float(instances) + pos.w*(aOffset - 1.0 + (1.0/float(instances)));
	gl_Position = vec4(xPos, pos.y, pos.z, pos.w);
	instanceid = gl_InstanceID;
}
