#include <string>
#include <fstream>
#include <chrono>
#include <random>
#define STB_IMAGE_IMPLEMENTATION

#include "pso.h"

static const float PI = 3.1415926;
static const int windowWidth = 128;
static const int windowHeight = 128;

//float CalculateEnergy(float* depthImage1, float* depthImage2, int imageSize)
//{
//	float energy = 0.0f;
//	for (int i = 0; i < imageSize; i++)
//	{
//		energy = energy + std::abs(depthImage1[i] - depthImage2[i]);
//	}
//	return energy;
//}
//
//float** GenerateMapsFromPoseParameters(int numParams, PoseParameters* poseparams)
//{
//	GLFWwindow* window;
//
//	/* Initialize the library */
//	if (!glfwInit())
//		return NULL;
//
//	/* Create a windowed mode window and its OpenGL context */
//	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Testing", NULL, NULL);
//	if (!window)
//	{
//		glfwTerminate();
//		return NULL;
//	}
//
//	// Make the window's context current and then hide it
//	glfwMakeContextCurrent(window);
//	glfwHideWindow(window);
//
//	// Initialize GLEW
//	glewExperimental = true;
//	if (glewInit() != GLEW_OK)
//		return NULL;
//
//	// Get and set up the shaders
//	const char* RTTVertexShaderPath = "../res/shaders/MainTestVertexShader.glsl";
//	const char* RTTFragmentShaderPath = "../res/shaders/MainTestFragmentShader.glsl";
//
//	Shader RTTShader(RTTVertexShaderPath, RTTFragmentShaderPath);
//
//	// Enable depth testing
//	glEnable(GL_DEPTH_TEST);
//
//	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer
//	GLuint FramebufferName = 0;
//	glGenFramebuffers(1, &FramebufferName);
//	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
//
//	// The depth buffer
//	GLuint depthrenderbuffer;
//	glGenRenderbuffers(1, &depthrenderbuffer);
//	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
//	glBindRenderbuffer(GL_RENDERBUFFER, 0);
//
//	// No color buffer is drawn to
//	glDrawBuffer(GL_NONE);
//
//	// check if the framebuffer is OK
//	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
//	{
//		std::cerr << "framebuffer setup was not successful" << std::endl;
//		return NULL;
//	}
//
//	// Save image into array
//	float** depthImages = new float*[numParams];
//
//	// Load model
//	const char* footModelPath = "../res/foot.obj";
//	Model footModel(footModelPath);
//
//	float zNear = 0.1f;
//	float zFar = 1.0f;
//
//	for (int i = 0; i < numParams; i++) {
//		// allocate space for depth image
//		float* depthImageFromRenderbuffer = new float[windowWidth*windowHeight];
//
//		/* Render here */
//		glClear(GL_DEPTH_BUFFER_BIT);
//
//		// Set up MVP matricies
//		PoseParameters params = poseparams[i];
//		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(params.XTranslation, params.YTranslation, params.ZTranslation));
//		model = glm::rotate(glm::rotate(glm::rotate(model, params.XRotation, glm::vec3(1, 0, 0)), params.YRotation, glm::vec3(0, 1, 0)), params.ZRotation, glm::vec3(0, 0, 1));
//		model = glm::scale(model, glm::vec3(1.0f));
//		glm::mat4 proj = glm::perspective(glm::radians(42.0f), 1.0f, zNear, zFar);
//		// Set up shader
//		RTTShader.use();
//		RTTShader.setMat4("u_M", model);
//		RTTShader.setMat4("u_P", proj);
//		RTTShader.setMat4("u_P_F", proj);
//		RTTShader.setFloat("zNear", zNear);
//		RTTShader.setFloat("zFar", zFar);
//		footModel.Draw(RTTShader);
//
//		// Render to our framebuffer
//		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
//		glBindFramebuffer(GL_FRAMEBUFFER, depthrenderbuffer);
//
//		// Save image
//		glReadPixels(0, 0, windowWidth, windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depthImageFromRenderbuffer);
//		depthImages[i] = depthImageFromRenderbuffer;
//	}
//
//	return depthImages;
//}
//
//static void WriteToFile(const float* depthBuffer, const int& width, const int& height, const char* destination) 
//{
//	std::ofstream outfile(destination);
//	for (int i=0; i < height; i++) 
//	{
//		for (int j = 0; j < width; j++)
//		{
//			outfile << depthBuffer[i*width+j] << " ";
//		}
//		outfile << std::endl;
//	}
//	outfile.close();
//}

static float* ReadFile(const char* location, const int& width, const int& height)
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
			image[index] = std::stof(token);
			index = index + 1;
		}
	}
	std::cout << "Total items parsed: " << index << std::endl;
	return image;
}

int main()
{
	int totalParticles = 10;
	float* refImage = ReadFile("../../Depth-Resources/ref128.txt", windowWidth, windowHeight);
	float* flippedRefImage = ReadFile("../../Depth-Resources/ref128f.txt", windowWidth, windowHeight);
	
	std::random_device rd;
	std::mt19937 gen(rd());

	float tx = -0.17f; float ty = 0.0f; float tz = -0.43f; 
	float rx = 5*PI/8+0.2; float ry=1.5*PI+0.13; float rz=-PI/4+0.03;
	float st = 0.05; float sr = 0.3;

	std::uniform_real_distribution<float> transx(tx-st,tx+st);
	std::uniform_real_distribution<float> transy(ty-st,ty+st);
	std::uniform_real_distribution<float> transz(tz-st,tz+st);
	std::uniform_real_distribution<float> rotx(rx-sr,rx+sr);
	std::uniform_real_distribution<float> roty(ry-sr,ry+sr);
	std::uniform_real_distribution<float> rotz(rz-sr,rz+sr);

	PoseParameters params[totalParticles];
	for (int i = 0; i < totalParticles; i++)
	{
		params[i] = PoseParameters(transx(gen), transy(gen), transz(gen), rotx(gen), roty(gen), rotz(gen));	
		params[i].Print();
	}
	//float** images = GenerateMapsFromPoseParameters(totalParticles, params);
	//for (int i = 0; i < totalParticles; i++)
	//{
	//	std::cout << "Image " << i << ": " << CalculateEnergy(refImage, images[i], windowWidth*windowHeight) << std::endl;
	//}

	PSO pso(totalParticles);
	PoseParameters optimizedParams = pso.Run(params, flippedRefImage, 30);
	std::cout << optimizedParams.XTranslation << " " << optimizedParams.YTranslation << " " << optimizedParams.ZTranslation << " " << optimizedParams.XRotation << " " << optimizedParams.YRotation << " " << optimizedParams.ZRotation << std::endl;
	
	//for (int i = 0; i < totalParticles; i++)
	//{
	//	char outputPath[50];
	//	sprintf(outputPath, "%s%d%s", "/home/eric/Dev/Depth-Resources/dm", i, ".txt");
	//	WriteToFile(images[i], 128, 128, outputPath);
	//}
	//PoseParameters oppa[1] = {optimizedParams};
	//float** image = GenerateMapsFromPoseParameters(1, oppa);
	//const char* optimgoutput = "/home/eric/Dev/Depth-Resources/opt.txt";
	//WriteToFile(image[0], 128, 128, optimgoutput);
	//std::cout << "Opt: " << CalculateEnergy(refImage, image[0], windowWidth*windowHeight) << std::endl;
}
