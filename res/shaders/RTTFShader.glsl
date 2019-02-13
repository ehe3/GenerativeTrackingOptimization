#version 310 es

precision mediump float;

// Output data
uniform mat4 u_P_F;
uniform float zNear;
uniform float zFar;

void main()
{
	float zTrans = 2.0 * gl_FragCoord.z - 1.0;
	gl_FragDepth = 2.0 * zNear * zFar / (zFar + zNear - zTrans * (zFar - zNear));
}
