#include <iostream>
#include <fstream>

#include "depth_generation.h"

static const float PI = 3.1415926;

static void WriteToFile(const float* depthBuffer, const int& width, const int& height, const char* destination) 
{
	std::ofstream outfile(destination);
	for (int i=0; i < height; i++) 
	{
		for (int j = 0; j < width; j++)
		{
			outfile << depthBuffer[i*width+j] << " ";
		}
		outfile << std::endl;
	}
	outfile.close();
}

int main()
{
	PoseParameters params1(0.0f, 0.0f, -0.15f, PI/2 - PI/8, PI/2, 0);
	PoseParameters params2(-0.1f, 0.0f, -0.15f, PI/2 + PI/8, PI/2, 0);
	PoseParameters params[2] = {params1, params2};
	float** images = GenerateMapsFromPoseParameters(2, params);
	for (int i = 0; i < 2; i++)
	{
		char outputPath[50];
		sprintf(outputPath, "%s%d%s", "/home/eric/Dev/Depth-Resources/dm", i, ".txt");
		WriteToFile(images[i], 480, 360, outputPath);
	}
}
