#version 460 core

precision mediump float;

// Output data
uniform float zNear;
uniform float zFar;

flat in int instanceid;

out vec4 FragColor;

void main()
{
	float lowerBound = 128.0*float(instanceid);
	float upperBound = lowerBound+128.0;
	if ((gl_FragCoord.x <	lowerBound) || (gl_FragCoord.x > upperBound)) {
		discard;
	}
	float zTrans = 2.0 * gl_FragCoord.z - 1.0;
	gl_FragDepth = 2.0 * zNear * zFar / (zFar + zNear - zTrans * (zFar - zNear));
}

