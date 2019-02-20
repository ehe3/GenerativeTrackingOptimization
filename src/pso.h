#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <chrono>

#include "model.h"

static void GLClearError()
{
	while(glGetError() != GL_NO_ERROR);
}

static void GLCheckError()
{
	while(GLenum error = glGetError()) 
	{
		std::cerr << "[OpenGL Error]:" << error << std::endl;
	}
}

float* CropImage(float* originalImage, const int& originalWidth, const int& originalHeight, const int& newWidth, const int& newHeight)
{
	int xStart = (originalWidth - newWidth) / 2;
	int xEnd = (originalWidth + newWidth) / 2;
	int yStart = (originalHeight - newHeight) / 2;
	int yEnd = (originalHeight + newHeight) / 2;

	float* croppedImage = new float[newWidth*newHeight];
	int index = 0;

	for (int h = 0; h < originalHeight; h++)
	{
		for (int w = 0; w < originalWidth; w++)
		{
			// Check to see if in cropping range
			if (w >= xStart && w < xEnd && h >= yStart && h < yEnd)
			{
				croppedImage[index] = originalImage[h*originalWidth + w];
				index = index + 1;
			}
		}
	}
	
	return croppedImage;
}


float CalculateEnergy(float* depthImage1, float* depthImage2, int imageSize)
{
	float energy = 0.0f;
	for (int i = 0; i < imageSize; i++)
	{
		energy = energy + std::abs(depthImage1[i] - depthImage2[i]);
	}
	return energy;
}

struct PoseParameters
{
public:
	float XTranslation;
	float YTranslation;
	float ZTranslation;
	float XRotation;
	float YRotation;
	float ZRotation;
	
	PoseParameters() : XTranslation{0.0f}, YTranslation{0.0f}, ZTranslation{0.0f}, XRotation{0.0f}, YRotation{0.0f}, ZRotation{0.0f} {}

	PoseParameters(float xtrans, float ytrans, float ztrans, float xrot, float yrot, float zrot) : XTranslation{xtrans}, YTranslation{ytrans}, ZTranslation{ztrans}, XRotation{xrot}, YRotation{yrot}, ZRotation{zrot} {}

	PoseParameters(const PoseParameters &params)
	{
		XTranslation = params.XTranslation;
		YTranslation = params.YTranslation;
		ZTranslation = params.ZTranslation;
		XRotation = params.XRotation;
		YRotation = params.YRotation;
		ZRotation = params.ZRotation;
	}

	PoseParameters operator+(PoseParameters const &obj) const
	{
		return PoseParameters(XTranslation + obj.XTranslation, YTranslation + obj.YTranslation, ZTranslation + obj.ZTranslation, XRotation + obj.XRotation, YRotation + obj.YRotation, ZRotation + obj.ZRotation);	
	}
	
	PoseParameters operator-(PoseParameters const &obj) const
	{
		return PoseParameters(XTranslation - obj.XTranslation, YTranslation - obj.YTranslation, ZTranslation - obj.ZTranslation, XRotation - obj.XRotation, YRotation - obj.YRotation, ZRotation - obj.ZRotation);	
	}

	PoseParameters operator*(float c)
	{
		return PoseParameters(c * XTranslation, c * YTranslation, c * ZTranslation, c * XRotation, c * YRotation, c * ZRotation);
	}


	// For debugging only
	void Print()
	{
		std::cout << "XTranslation: " << XTranslation << " YTranslation: " << YTranslation << " ZTranslation: " << ZTranslation << " XRotation: " << XRotation << " YRotation: " << YRotation << " ZRotation: " << ZRotation << std::endl;
	}
};

class Particle {

public:
	PoseParameters Position;
	float BestEnergyScore;
	PoseParameters BestPosition;
	PoseParameters Velocity;

	Particle(PoseParameters initialPosition, float energy): Position{initialPosition}, BestEnergyScore{energy}, BestPosition{initialPosition}, Velocity{PoseParameters()} {}

	Particle(): Position{PoseParameters()}, BestEnergyScore{std::numeric_limits<float>::infinity()}, BestPosition{PoseParameters()}, Velocity{PoseParameters()} {}

};

float** GenerateMapsFromParticles(int numParams, Particle* particles);
float** GenerateMapsFromPoseParameters(int numParams, PoseParameters* poseparams);
float CalculateEnergy(float* depthImage1, float* depthImage2, int size);

class PSO {

private:	
	Particle* particles;
	float* referenceImage;
	int populationSize;
	int iterations;
	int ImageWidth;
	int ImageHeight;
	float CognitiveConst;
	float SocialConst;
	float ConstrictionConst;
	PoseParameters GlobalBestPosition;
	float GlobalBestEnergy;

public:
	PSO(PoseParameters* parameterList, float* refImg, int numParticles, int iters, int ImgWidth, int ImgHeight, float CogConst=2.8, float SocConst=1.3) : referenceImage{refImg}, populationSize{numParticles}, iterations{iters}, ImageWidth{ImgWidth}, ImageHeight{ImgHeight}, CognitiveConst{CogConst}, SocialConst{SocConst}, ConstrictionConst{0.0f}, GlobalBestPosition{PoseParameters()}, GlobalBestEnergy{std::numeric_limits<float>::infinity()} {
		float Phi = CognitiveConst + SocialConst;
		if (Phi <= 4) 
		{
			std::cerr << "WARNING: Optimization constants too small" << std::endl;
		}
		ConstrictionConst = 2.0f / std::abs(2.0f - Phi - sqrt(Phi * Phi - 4 * Phi)); 
		
		// Intialize particles
		particles = new Particle[numParticles];
		for (int i = 0; i < numParticles; i++)
		{
			particles[i].Position = parameterList[i];
		}
	}

	PoseParameters Run()
	{
		GLFWwindow* window;

		// Initialize GLFW
		if (!glfwInit())
		{
			std::cerr << "WARNING: GLFW not initialized properly" << std::endl;
		}

		// Create a windowed mode winodw and its OpenGL context
		window = glfwCreateWindow(200, 200, "Depth", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			std::cerr << "WARNING: GLFW window was not created properly" << std::endl;
		}

		// Make the window's context current and then hide it
		glfwMakeContextCurrent(window);
		//glfwHideWindow(window);

		// Initialize GLEW
		glewExperimental = true;
		if (glewInit() != GLEW_OK)
		{
			std::cerr << "WARNING: GLEW not initialized properly" << std::endl;
		}

		float quadVertices[] = {
			// positions   // texCoords
			-1.0f,  1.0f,  0.0f, 0.0f,
			-1.0f, -1.0f,  0.0f, 1.0f,
			 1.0f, -1.0f,  1.0f, 1.0f,

			-1.0f,  1.0f,  0.0f, 0.0f,
			 1.0f, -1.0f,  1.0f, 1.0f,
			 1.0f,  1.0f,  1.0f, 0.0f
		};

		// Get and set up shaders
		Shader screenShader("../res/shaders/TexVertexShader.glsl", "../res/shaders/TexFragmentShader.glsl");
		Shader RTTShader("../res/shaders/RTTVShader.glsl", "../res/shaders/RTTFShader.glsl");
		Shader R2Shader("../res/shaders/ReductionVShader.glsl", "../res/shaders/Reduction2FShader.glsl");
		Shader R5Shader("../res/shaders/ReductionVShader.glsl", "../res/shaders/Reduction5FShader.glsl");
		
		// Declare quad
		GLuint quadVAO, quadVBO;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

		// Load reference image into a texture
		GLuint refdepthtex;
		glGenTextures(1, &refdepthtex);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, refdepthtex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 200, 200, 0, GL_DEPTH_COMPONENT, GL_FLOAT, referenceImage);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Set up ping
		GLuint ping;
		glGenFramebuffers(1, &ping);
		glBindFramebuffer(GL_FRAMEBUFFER, ping);
		
		// Render to texture
		GLuint depthtexture;
		glGenTextures(1, &depthtexture);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, depthtexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 200, 200, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture, 0);

		//glBindTexture(GL_TEXTURE_2D, 0);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// No color buffer is drawn to (check if this is even necessary)
		glDrawBuffer(GL_NONE);

		// Check if the ping was set up correctly
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "WARNING: ping setup was not successful" << std::endl;
		}
		
		// Load the foot model 
		const char* footmodelpath = "../res/foot.obj";
		Model footModel(footmodelpath);
		
		// Set up MVP matricies
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.18f, 0.05f, -0.62f));
		model = glm::rotate(glm::rotate(glm::rotate(model, (float)(5*3.14159/8), glm::vec3(1, 0, 0)), (float)(1.5*3.14159), glm::vec3(0, 1, 0)), (float)(-3.14159/4), glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(58.59f), 1.0f, 0.05f, 1.0f);
				
		 //Send matricies to shader
		RTTShader.use(); // Check to see if this needs to be called every time
		RTTShader.setMat4("u_M", model);
		RTTShader.setMat4("u_P", proj);
		RTTShader.setFloat("zNear", 0.05f);
		RTTShader.setFloat("zFar", 1.0f);
		footModel.Draw(RTTShader);
		
		while (!glfwWindowShouldClose(window))
		{
			//glDisable(GL_DEPTH_TEST);
			glEnable(GL_DEPTH_TEST);
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glActiveTexture(GL_TEXTURE0 + 1);
			RTTShader.use();
			footModel.Draw(RTTShader);

			glBindTexture(GL_TEXTURE_2D, depthtexture);
			glBindFramebuffer(GL_FRAMEBUFFER, ping);
			
			glDisable(GL_DEPTH_TEST);

			// difference
			//glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, depthtexture);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			screenShader.use();
			screenShader.setInt("screenTexture", 0);
			screenShader.setInt("gendepTexture", 1);
		
			glBindVertexArray(quadVAO);
			glBindTexture(GL_TEXTURE_2D, refdepthtex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		
			// scale to 100 
			//glActiveTexture(GL_TEXTURE0 + 2);
			//glBindTexture(GL_TEXTURE_2D, dt2);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dt2, 0);

			//R2Shader.use();
			//R2Shader.setInt("tex", 0);

			//glBindVertexArray(quadVAO);
			//glBindTexture(GL_TEXTURE_2D, dt2);
			//glDrawArrays(GL_TRIANGLES, 0, 6);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		// Time the PSO without setup
		//auto start = std::chrono::high_resolution_clock::now();
		//// Setup finished, start the particle swarm!
		//for (int generation = 0; generation < iterations; generation++)
		//{
		//	for (int p = 0; p < populationSize; p++)
		//	{
		//		// See if this array actually needs to be allocated on the heap + memory management
		//		float* depthImageFromRenderbuffer = new float[640*360];

		//		// First, generate the depth maps from OpenGL context
		//		PoseParameters params = particles[p].Position;
		//		particles[p].Position.Print();			
		//		// Clear Depth Buffer
		//		glClear(GL_DEPTH_BUFFER_BIT);

		//		// Set up MVP matricies
		//		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(params.XTranslation, params.YTranslation, params.ZTranslation));
		//		model = glm::rotate(glm::rotate(glm::rotate(model, params.XRotation, glm::vec3(1, 0, 0)), params.YRotation, glm::vec3(0, 1, 0)), params.ZRotation, glm::vec3(0, 0, 1));
		//		model = glm::scale(model, glm::vec3(1.0f));
		//		glm::mat4 proj = glm::perspective(glm::radians(58.59f), 1.778f, 0.05f, 1.0f);
		//		
		//		// Send matricies to shader
		//		Shader.use(); // Check to see if this needs to be called every time
		//		Shader.setMat4("u_M", model);
		//		Shader.setMat4("u_P", proj);
		//		Shader.setMat4("u_P_F", proj);
		//		Shader.setFloat("zNear", 0.05f);
		//		Shader.setFloat("zFar", 1.0f);
		//		footModel.Draw(Shader);

		//		// Save image
		//		glReadPixels(0, 0, 640, 360, GL_DEPTH_COMPONENT, GL_FLOAT, depthImageFromRenderbuffer);
		//	
		//		// Start swarm calculations
		//		float* croppedDM = CropImage(depthImageFromRenderbuffer, 640, 360, 200, 200);
		//		float currentIndividualEnergy = CalculateEnergy(referenceImage, croppedDM, 200*200);	
		//		delete[] croppedDM;
		//		delete[] depthImageFromRenderbuffer;

		//		std::cout << "cie: " << currentIndividualEnergy << std::endl;
		//		std::cout << "cbes for particle " << p << ": " << particles[p].BestEnergyScore << std::endl;
		//		if (currentIndividualEnergy < particles[p].BestEnergyScore)
		//		{
		//			particles[p].BestEnergyScore = currentIndividualEnergy;
		//			particles[p].BestPosition = particles[p].Position;
		//			if (currentIndividualEnergy < GlobalBestEnergy)
		//			{
		//				GlobalBestEnergy = currentIndividualEnergy;
		//				GlobalBestPosition = particles[p].Position;
		//			}
		//		}
		//		std::cout << "gbe: " << GlobalBestEnergy << std::endl;
		//		float r1 = ((float) std::rand() / RAND_MAX);
		//		float r2 = ((float) std::rand() / RAND_MAX);
		//		particles[p].Velocity = particles[p].Velocity + ((particles[p].BestPosition - particles[p].Position)*CognitiveConst*r1 + (GlobalBestPosition - particles[p].Position)*SocialConst*r2)*ConstrictionConst;
		//		particles[p].Position = particles[p].Position + particles[p].Velocity; 
		//	}
		//}
		//auto end = std::chrono::high_resolution_clock::now();
		//std::cout << "Time it took for PSO to execute without OpenGL setup is: " << std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count() << std::endl;
		//return GlobalBestPosition;
		return PoseParameters();
	}
};
