#version 460 core

layout(location = 0) in vec4 aPos;
layout(location = 5) in float aOffset;
layout(location = 6) in mat4 instanceMatrix;

uniform int instances;
uniform mat4 u_M;
uniform mat4 u_P;

flat out int instanceid;

void main()
{
	vec4 pos = u_P * instanceMatrix *aPos;
	float xPos = pos.x/float(instances) + pos.w*(aOffset - 1.0 + (1.0/float(instances)));
	gl_Position = vec4(xPos, pos.y, pos.z, pos.w);
	instanceid = gl_InstanceID;
}
