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

		Particle(): Position{PoseParameters()}, BestEnergyScore{std::numeric_limits<float>::infinity()}, BestPosition{PoseParameters()}, Velocity{PoseParameters()} {}

};

class PSO {

	private:	
		float CognitiveConst;
		float SocialConst;
		float ConstrictionConst;
		// OpenGL vars
		GLFWwindow* window;
		Shader SubtractionShader, RTTShader, R2Shader, R4X3YShader, R5Shader, PTShader;
		Model footModel;
		// quads, textures, and buffers
		GLuint quadVAO, quadVBO, refdepthtex, ping, depthtexture, pong, difftex, pang, tex320x240, pung, tex160x120, pling, tex80x60, plang, tex40x30, plong, tex20x15, plung, tex5x5, pleng, tex1x1;

	public:
		PSO(float CogConst=2.8, float SocConst=1.3) : 
			CognitiveConst{CogConst}, 
			SocialConst{SocConst}, 
			ConstrictionConst{0.0f}, 
			window{nullptr}
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

			window = glfwCreateWindow(640, 480, "PSO", NULL, NULL);

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
			R4X3YShader = Shader("../res/shaders/PassThroughQuadVertexShader.glsl", "../res/shaders/Reduction4X3YFShader.glsl");
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

			// set up ping
			glGenFramebuffers(1, &ping);
			glBindFramebuffer(GL_FRAMEBUFFER, ping);

			// loaded reference depth texture
			glGenTextures(1, &depthtexture);
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, depthtexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 640, 480, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture, 0);

			// set up pong 
			glGenFramebuffers(1, &pong);
			glBindFramebuffer(GL_FRAMEBUFFER, pong);

			// texture that is the difference of reference and rendered
			glGenTextures(1, &difftex);
			glActiveTexture(GL_TEXTURE0 + 2);
			glBindTexture(GL_TEXTURE_2D, difftex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 640, 480, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, difftex, 0);

			// set up pang 
			glGenFramebuffers(1, &pang);
			glBindFramebuffer(GL_FRAMEBUFFER, pang);

			// 320x240 texture
			glGenTextures(1, &tex320x240);
			glActiveTexture(GL_TEXTURE0 + 3);
			glBindTexture(GL_TEXTURE_2D, tex320x240);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 320, 240, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex320x240, 0);

			// set up pung
			glGenFramebuffers(1, &pung);
			glBindFramebuffer(GL_FRAMEBUFFER, pung);

			// 160x120 texture
			glGenTextures(1, &tex160x120);
			glActiveTexture(GL_TEXTURE0 + 4);
			glBindTexture(GL_TEXTURE_2D, tex160x120);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 160, 120, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex160x120, 0);

			// set up pling	
			glGenFramebuffers(1, &pling);
			glBindFramebuffer(GL_FRAMEBUFFER, pling);

			// 80x60 texture
			glGenTextures(1, &tex80x60);
			glActiveTexture(GL_TEXTURE0 + 5);
			glBindTexture(GL_TEXTURE_2D, tex80x60);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 80, 60, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex80x60, 0);

			// set up plang
			glGenFramebuffers(1, &plang);
			glBindFramebuffer(GL_FRAMEBUFFER, plang);

			// 40x30 texture
			glGenTextures(1, &tex40x30);
			glActiveTexture(GL_TEXTURE0 + 6);
			glBindTexture(GL_TEXTURE_2D, tex40x30);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 40, 30, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex40x30, 0);

			// set up plong
			glGenFramebuffers(1, &plong);
			glBindFramebuffer(GL_FRAMEBUFFER, plong);

			// 20x15 texture
			glGenTextures(1, &tex20x15);
			glActiveTexture(GL_TEXTURE0 + 7);
			glBindTexture(GL_TEXTURE_2D, tex20x15);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 20, 15, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex20x15, 0);

			// set up plung
			glGenFramebuffers(1, &plung);
			glBindFramebuffer(GL_FRAMEBUFFER, plung);

			// 5x5 texture
			glGenTextures(1, &tex5x5);
			glActiveTexture(GL_TEXTURE0 + 8);
			glBindTexture(GL_TEXTURE_2D, tex5x5);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 5, 5, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex5x5, 0);
			
			// set up pleng
			glGenFramebuffers(1, &pleng);
			glBindFramebuffer(GL_FRAMEBUFFER, pleng);

			// 1x1 texture
			glGenTextures(1, &tex1x1);
			glActiveTexture(GL_TEXTURE0 + 9);
			glBindTexture(GL_TEXTURE_2D, tex1x1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex1x1, 0);
		}

		PoseParameters Run(PoseParameters* parameterList, float* refImg, int numParticles, int iters)
		{	
			// Load reference image into texture 0
			glGenTextures(1, &refdepthtex);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, refdepthtex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 640, 480, 0, GL_DEPTH_COMPONENT, GL_FLOAT, refImg);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	

			// Intialize particles
			Particle* particles = new Particle[numParticles];
			for (int i = 0; i < numParticles; i++)
			{
				particles[i].Position = parameterList[i];
			}
			
			// Time the PSO without setup
			auto start = std::chrono::high_resolution_clock::now();
			// Setup finished, start the particle swarm!
			PoseParameters GlobalBestPosition;
			float GlobalBestEnergy = std::numeric_limits<float>::infinity();

			glm::mat4 proj = glm::perspective(glm::radians(42.0f), 1.333f, 0.05f, 1.0f);

			for (int generation = 0; generation < iters; generation++)
			{
				for (int p = 0; p < numParticles; p++)
				{
					// Set up MVP matricies
					glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(particles[p].Position.XTranslation, particles[p].Position.YTranslation, particles[p].Position.ZTranslation));
					model = glm::rotate(glm::rotate(glm::rotate(model, particles[p].Position.XRotation, glm::vec3(1, 0, 0)), particles[p].Position.YRotation, glm::vec3(0, 1, 0)), particles[p].Position.ZRotation, glm::vec3(0, 0, 1));

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
					R2Shader.setFloat("width", 640.0f);
					R2Shader.setFloat("height", 480.0f);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, pung);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 3);
					R2Shader.setFloat("width", 320.0f);
					R2Shader.setFloat("height", 240.0f);
					glViewport(0, 0, 320, 240);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, pling);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 4);
					R2Shader.setFloat("width", 160.0f);
					R2Shader.setFloat("height", 120.0f);
					glViewport(0, 0, 160, 120);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, plang);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 5);
					R2Shader.setFloat("width", 80.0f);
					R2Shader.setFloat("height", 60.0f);
					glViewport(0, 0, 80, 60);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, plong);
					glClear(GL_DEPTH_BUFFER_BIT);
					R2Shader.use();
					R2Shader.setInt("tex", 6);
					R2Shader.setFloat("width", 40.0f);
					R2Shader.setFloat("height", 30.0f);
					glViewport(0, 0, 40, 30);
					glDrawArrays(GL_TRIANGLES, 0 , 6);

					glBindFramebuffer(GL_FRAMEBUFFER, plung);
					glClear(GL_DEPTH_BUFFER_BIT);
					R4X3YShader.use();
					R4X3YShader.setInt("tex", 7);
					R4X3YShader.setFloat("width", 20.0f);
					R4X3YShader.setFloat("height", 15.0f);
					glViewport(0, 0, 20, 15);
					glDrawArrays(GL_TRIANGLES, 0 , 6);
						
					glBindFramebuffer(GL_FRAMEBUFFER, pleng);
					glClear(GL_DEPTH_BUFFER_BIT);
					R5Shader.use();
					R5Shader.setInt("tex", 8);
					R5Shader.setFloat("width", 5.0f);
					R5Shader.setFloat("height", 5.0f);
					glViewport(0, 0, 5, 5);
					glDrawArrays(GL_TRIANGLES, 0 , 6);
					
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
					glClear(GL_DEPTH_BUFFER_BIT);
					PTShader.use();
					PTShader.setInt("tex", 9);
					glViewport(0, 0, 640, 480);
					glBindVertexArray(quadVAO);
					glDrawArrays(GL_TRIANGLES, 0, 6);

					float* currentdt = new float[1];
					glGetTextureImage(tex1x1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, sizeof(float), currentdt);
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
