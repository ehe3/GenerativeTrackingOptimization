#pragma once

struct PoseParameters
{
public:
	float XTranslation;
	float YTranslation;
	float ZTranslation;
	float XRotation;
	float YRotation;
	float ZRotation;
	PoseParameters(float xtrans, float ytrans, float ztrans, float xrot, float yrot, float zrot) : XTranslation{xtrans}, YTranslation{ytrans}, ZTranslation{ztrans}, XRotation{xrot}, YRotation{yrot}, ZRotation{zrot} {}
};

float** GenerateMapsFromPoseParameters(int numParams, PoseParameters* poseparams);

