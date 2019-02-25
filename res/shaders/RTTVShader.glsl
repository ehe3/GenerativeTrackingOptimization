#version 310 es

layout(location = 0) in vec4 aPos;
layout(location = 5) in vec2 aOffset;
layout(location = 6) in mat4 instanceMatrix;

uniform int instances;
uniform mat4 u_M;
uniform mat4 u_P;

flat out int instanceid;

void main()
{
	vec4 pos = u_P * instanceMatrix *aPos;
	float xPos = pos.x/float(instances) + aOffset.x - 1.0 + (1.0/float(instances));
	gl_Position = vec4(xPos, pos.y + aOffset.y, pos.z, 1.0);
	instanceid = gl_InstanceID;
}
