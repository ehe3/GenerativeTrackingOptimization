#include <iostream>
#include <string>
#include <fstream>

#include "pso.h"

static const float PI = 3.1415926;
static const double DEPTH_SCALE = 0.0010000000474974513;

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

static float* ReadFile(const char* location, const int& width, const int& height, const double& depthscale)
{
	std::ifstream textfile(location);

	float* image = new float[width*height];
	std::string line;
	std::string delimiter = " ";
	size_t pos = 0;
	int index = 0;
	while(std::getline(textfile, line))
	{
		pos = 0;
		std::string token;
		while ((pos = line.find(delimiter)) != std::string::npos)
		{
			token = line.substr(0, pos);
			line.erase(0, pos + delimiter.length());
			float scaledVal = std::stoi(token) * DEPTH_SCALE;
			image[index] = (scaledVal > 1.0f) ? 1.0f : scaledVal;
			index = index + 1;
		}
	}
	std::cout << "Total items parsed: " << index << std::endl;
	return image;
}

int main()
{
	const char* RefLocation = "../../Depth-Resources/000050.txt";
	int width = 200;
	int height = 200;
	float* readImage = ReadFile(RefLocation, width, height, DEPTH_SCALE);
	PoseParameters params1(-0.18f, 0.05f, -0.62f, 5* PI/8, 1.5 * PI, -PI/4);
	PoseParameters params2(-0.17f, 0.0f, -0.43f, 5* PI/8 + 0.2, 1.5 * PI + 0.13, -PI/4 + 0.03);
	PoseParameters params3(-0.05f, 0.2f, -0.72f, 5* PI/8 - 0.1, 1.5 * PI - 0.12, -PI/4 + 0.11);
	PoseParameters params4(-0.13f, 0.08f, -0.81f, 5* PI/8 - 0.5, 1.5 * PI + 0.43, -PI/4 - 0.02);
	PoseParameters params5(0.0f, 0.25f, -0.63f, 5* PI/8 + 0.4, 1.5 * PI - 0.02, -PI/4 - 0.24);
	PoseParameters params6(-0.04f, 0.13f, -0.62f, PI/2, 1.5 * PI - 0.12, -PI/4 - 0.12);
	PoseParameters params[6] = {params1, params2, params3, params4, params5, params6};
	float** images = GenerateMapsFromPoseParameters(6, params);
	std::cout << std::endl << std::endl << "HEREERERER" << std::endl << std::endl;
	for (int i = 0; i < 6; i++)
	{
		std::cout << "Image " << i << ": " << CalculateEnergy(readImage, images[i], width*height) << std::endl;
	}
	PSO pso(params, readImage, 6, 20, 200, 200);
	PoseParameters optimizedParams = pso.Run();
	std::cout << optimizedParams.XTranslation << " " << optimizedParams.YTranslation << " " << optimizedParams.ZTranslation << " " << optimizedParams.XRotation << " " << optimizedParams.YRotation << " " << optimizedParams.ZRotation << std::endl;
	
	for (int i = 0; i < 6; i++)
	{
		char outputPath[50];
		sprintf(outputPath, "%s%d%s", "/home/eric/Dev/Depth-Resources/dm", i, ".txt");
		WriteToFile(images[i], 200, 200, outputPath);
	}
	PoseParameters oppa[1] = {optimizedParams};
	float** image = GenerateMapsFromPoseParameters(1, oppa);
	const char* optimgoutput = "/home/eric/Dev/Depth-Resources/opt.txt";
	WriteToFile(image[0], 200, 200, optimgoutput);
	std::cout << "Opt: " << CalculateEnergy(readImage, image[0], width*height);
}
