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
		int ImageWidth;
		int ImageHeight;
		float CognitiveConst;
		float SocialConst;
		float ConstrictionConst;
		PoseParameters GlobalBestPosition;
		float GlobalBestEnergy;
		// OpenGL vars
		GLFWwindow* window;
		Shader SubtractionShader, RTTShader, R2Shader, R5Shader, PTShader;
		Model footModel;
		// quads, textures, and buffers
		GLuint quadVAO, quadVBO, refdepthtex, ping, depthtexture, pong, difftex, pang, tex100, pung, tex50, pling, tex10, plang, tex2, finalboss, tex1;

	public:
		PSO(float CogConst=2.8, float SocConst=1.3) : 
			CognitiveConst{CogConst}, 
			SocialConst{SocConst}, 
			ConstrictionConst{0.0f}, 
			GlobalBestPosition{PoseParameters()}, 
			GlobalBestEnergy{std::numeric_limits<float>::infinity()},
			window{glfwCreateWindow(200, 200, "PSO", NULL, NULL)}
		{
			float Phi = CognitiveConst + SocialConst;
			if (Phi <= 4) 
			{
				std::cerr << "WARNING: Optimization constants too small" << std::endl;
			}
			ConstrictionConst = 2.0f / std::abs(2.0f - Phi - sqrt(Phi * Phi - 4 * Phi)); 


			// Initialize GLFW
			if (!glfwInit())
			{
				std::cerr << "WARNING: GLFW not initialized properly" << std::endl;
			}

			// Check to see if the window is valid
			if (!window)
			{
				glfwTerminate();
				std::cerr << "WARNING: GLFW window was not created properly" << std::endl;
			}

			// Make the window's context current and then hide it
			glfwMakeContextCurrent(window);
			glfwHideWindow(window);

			// Initialize GLEW
			glewExperimental = true;
			if (glewInit() != GLEW_OK)
			{
				std::cerr << "WARNING: GLEW not initialized properly" << std::endl;
			}

			// Get and set up shaders
			SubtractionShader = Shader("../res/shaders/PassThroughQuadVertexShader.glsl", "../res/shaders/SubtractionFragmentShader.glsl");
			RTTShader = Shader("../res/shaders/RTTVShader.glsl", "../res/shaders/RTTFShader.glsl");
			R2Shader = Shader("../res/shaders/PassThroughQuadVertexShader.glsl", "../res/shaders/Reduction2FShader.glsl");
			R5Shader = Shader("../res/shaders/PassThroughQuadVertexShader.glsl", "../res/shaders/Reduction5FShader.glsl");
			PTShader = Shader("../res/shaders/PTVS.glsl", "../res/shaders/PTFS.glsl");

			// Load the foot model 
			footModel = Model("../res/foot.obj");

			float quadVertices[] = {
				// positions   // texCoords
				-1.0f,  1.0f,  0.0f, 0.0f,
				-1.0f, -1.0f,  0.0f, 1.0f,
				1.0f, -1.0f,  1.0f, 1.0f,

				-1.0f,  1.0f,  0.0f, 0.0f,
				1.0f, -1.0f,  1.0f, 1.0f,
				1.0f,  1.0f,  1.0f, 0.0f
			};

			// Declare quad
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


			// Set up ping
			glGenFramebuffers(1, &ping);
			glBindFramebuffer(GL_FRAMEBUFFER, ping);

			// Render to texture
			glGenTextures(1, &depthtexture);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, depthtexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 200, 200, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Set up pong 
			glGenFramebuffers(1, &pong);
			glBindFramebuffer(GL_FRAMEBUFFER, pong);

			glGenTextures(1, &difftex);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, difftex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 200, 200, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, difftex, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// Set up pang 
			glGenFramebuffers(1, &pang);
			glBindFramebuffer(GL_FRAMEBUFFER, pang);

			glGenTextures(1, &tex100);
			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, tex100);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 100, 100, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex100, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glGenFramebuffers(1, &pung);
			glBindFramebuffer(GL_FRAMEBUFFER, pung);

			glGenTextures(1, &tex50);
			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, tex50);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 50, 50, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex50, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glGenFramebuffers(1, &pling);
			glBindFramebuffer(GL_FRAMEBUFFER, pling);

			glGenTextures(1, &tex10);
			glActiveTexture(GL_TEXTURE0 + 5);
			glBindTexture(GL_TEXTURE_2D, tex10);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 10, 10, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex10, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glGenFramebuffers(1, &plang);
			glBindFramebuffer(GL_FRAMEBUFFER, plang);

			glGenTextures(1, &tex2);
			glActiveTexture(GL_TEXTURE0 + 6);
			glBindTexture(GL_TEXTURE_2D, tex2);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 2, 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex2, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glGenFramebuffers(1, &finalboss);
			glBindFramebuffer(GL_FRAMEBUFFER, finalboss);

			glGenTextures(1, &tex1);
			glActiveTexture(GL_TEXTURE0 + 7);
			glBindTexture(GL_TEXTURE_2D, tex1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex1, 0);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		PoseParameters Run(PoseParameters* parameterList, float* refImg, int numParticles, int iters, int choice)
		{	
			// Load reference image into texture 0
			glGenTextures(1, &refdepthtex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, refdepthtex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, 200, 200, 0, GL_DEPTH_COMPONENT, GL_FLOAT, refImg);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	

			// Intialize particles
			Particle* particles = new Particle[numParticles];
			for (int i = 0; i < numParticles; i++)
			{
				particles[i].Position = parameterList[i];
			}

			//static const float PIE = 3.1415926;
			//PoseParameters params;
			//if (choice == 0) {
			//	params = PoseParameters(-0.18f, 0.05f, -0.62f, 5* PIE/8, 1.5 * PIE, -PIE/4); // 1
			//} else if (choice == 1)
			//{
			//	params = PoseParameters(-0.17f, 0.0f, -0.43f, 5* PIE/8 + 0.2, 1.5 * PIE + 0.13, -PIE/4 + 0.03); // 2
			//} else if (choice == 2)
			//{
			//	params = PoseParameters(-0.05f, 0.2f, -0.72f, 5* PIE/8 - 0.1, 1.5 * PIE - 0.12, -PIE/4 + 0.11); // 3
			//} else if (choice == 3)
			//{
			//	params = PoseParameters(-0.13f, 0.08f, -0.81f, 5* PIE/8 - 0.5, 1.5 * PIE + 0.43, -PIE/4 - 0.02);// 4
			//} else if (choice == 4)
			//{
			//	params = PoseParameters(0.0f, 0.25f, -0.63f, 5* PIE/8 + 0.4, 1.5 * PIE - 0.02, -PIE/4 - 0.24); // 5
			//} else if (choice == 5)
			//{
			//	params = PoseParameters(-0.04f, 0.13f, -0.62f, PIE/2, 1.5 * PIE - 0.12, -PIE/4 - 0.12); // 6
			//}
			// Time the PSO without setup
			auto start = std::chrono::high_resolution_clock::now();
			// Setup finished, start the particle swarm!
			for (int generation = 0; generation < iters; generation++)
			{
				for (int p = 0; p < numParticles; p++)
				{

					// Set up MVP matricies
					glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(particles[p].Position.XTranslation, particles[p].Position.YTranslation, particles[p].Position.ZTranslation));
					model = glm::rotate(glm::rotate(glm::rotate(model, particles[p].Position.XRotation, glm::vec3(1, 0, 0)), particles[p].Position.YRotation, glm::vec3(0, 1, 0)), particles[p].Position.ZRotation, glm::vec3(0, 0, 1));
					glm::mat4 proj = glm::perspective(glm::radians(58.59f), 1.0f, 0.05f, 1.0f);

					//Send matricies to shader
					RTTShader.use(); // Check to see if this needs to be called every time
					RTTShader.setMat4("u_M", model);
					RTTShader.setMat4("u_P", proj);
					RTTShader.setFloat("zNear", 0.05f);
					RTTShader.setFloat("zFar", 1.0f);

					glEnable(GL_DEPTH_TEST);
					glBindFramebuffer(GL_FRAMEBUFFER, ping);
					glClear(GL_DEPTH_BUFFER_BIT);

					RTTShader.use();
					footModel.Draw(RTTShader);

					glBindFramebuffer(GL_FRAMEBUFFER, pong);
					glClear(GL_DEPTH_BUFFER_BIT);

					SubtractionShader.use();
					SubtractionShader.setInt("screenTexture", 0);
					SubtractionShader.setInt("gendepTexture", 1);

					glBindVertexArray(quadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					glBindFramebuffer(GL_FRAMEBUFFER, pang);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 2);
					R2Shader.setFloat("width", 200.0f);
					R2Shader.setFloat("height", 200.0f);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, pung);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 3);
					R2Shader.setFloat("width", 100.0f);
					R2Shader.setFloat("height", 100.0f);
					glViewport(0, 0, 100, 100);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, pling);
					glClear(GL_DEPTH_BUFFER_BIT);
					R5Shader.use();
					R5Shader.setInt("tex", 4);
					R5Shader.setFloat("width", 50.0f);
					R5Shader.setFloat("height", 50.0f);
					glViewport(0, 0, 50, 50);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, plang);
					glClear(GL_DEPTH_BUFFER_BIT);
					R5Shader.use();
					R5Shader.setInt("tex", 5);
					R5Shader.setFloat("width", 10.0f);
					R5Shader.setFloat("height", 10.0f);
					glViewport(0, 0, 10, 10);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, finalboss);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 6);
					R2Shader.setFloat("width", 2.0f);
					R2Shader.setFloat("height", 2.0f);
					glViewport(0, 0, 2, 2);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glClear(GL_DEPTH_BUFFER_BIT);
					PTShader.use();
					PTShader.setInt("tex", 7);
					glViewport(0, 0, 200, 200);
					glBindVertexArray(quadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);
					float* currentdt = new float[1];
					glGetTextureImage(tex1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, sizeof(float), currentdt);
					//std::cout << "Final depth value: " << currentdt[0] << std::endl;

					float currentIndividualEnergy = currentdt[0];	

					std::cout << "cie: " << currentIndividualEnergy << std::endl;
					std::cout << "cbes for particle " << p << ": " << particles[p].BestEnergyScore << std::endl;
					if (currentIndividualEnergy < particles[p].BestEnergyScore)
					{
						particles[p].BestEnergyScore = currentIndividualEnergy;
						particles[p].BestPosition = particles[p].Position;
						if (currentIndividualEnergy < GlobalBestEnergy)
						{
							GlobalBestEnergy = currentIndividualEnergy;
							GlobalBestPosition = particles[p].Position;
						}
					}
					std::cout << "gbe: " << GlobalBestEnergy << std::endl;
					float r1 = ((float) std::rand() / RAND_MAX);
					float r2 = ((float) std::rand() / RAND_MAX);
					particles[p].Velocity = particles[p].Velocity + ((particles[p].BestPosition - particles[p].Position)*CognitiveConst*r1 + (GlobalBestPosition - particles[p].Position)*SocialConst*r2)*ConstrictionConst;
					particles[p].Position = particles[p].Position + particles[p].Velocity; 
				}
			}
			auto end = std::chrono::high_resolution_clock::now();
			std::cout << "Time it took for PSO to execute without OpenGL setup is: " << std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count() << std::endl;
			return GlobalBestPosition;
		}
};
