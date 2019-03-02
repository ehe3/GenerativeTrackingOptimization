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

				void Assuage(float xT=0.05, float yT=0.05, float zT=0.05, float xR=0.1, float yR=0.1, float zR=0.1)
				{
					XTranslation = XTranslation > xT ? xT : XTranslation < -xT ? -xT : XTranslation;
					YTranslation = YTranslation > yT ? yT : YTranslation < -yT ? -yT : YTranslation;
					ZTranslation = ZTranslation > zT ? zT : ZTranslation < -zT ? -zT : ZTranslation;
					XRotation = XRotation > xR ? xR : XRotation < -xR ? -xR : XRotation;
					YRotation = YRotation > yR ? yR : YRotation < -yR ? -yR : YRotation;
					ZRotation = ZRotation > zR ? zR : ZRotation < -zR ? -zR : ZRotation;
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
        int NumParticles;
        float CognitiveConst;
        float SocialConst;
        float ConstrictionConst;
        // OpenGL vars
        GLFWwindow* window;
        glm::mat4 ProjMat;
        Shader RepeatShader, SubtractionShader, RTTShader, R2Shader, PTShader;
        Model footHigh, footMid, footLow;
        // quads, textures, and buffers
        GLuint quadVAO, quadVBO, repeatQuadVAO, repeatQuadVBO, refdepthtex, peng, repeattex, ping, depthtexture, pong, difftex, pang, tex64, pung, tex32, pling, tex16, plang, tex8, plong, tex4, plung, tex2, pleng, tex1;
        // instance buffers
        GLuint instanceVBO, transformationInstanceBuffer;

    public:
        PSO(int numParticles, float CogConst=2.8, float SocConst=1.3) : 
            NumParticles{numParticles},	
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

            window = glfwCreateWindow(128*NumParticles, 128, "PSO", NULL, NULL);

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

            ProjMat = glm::perspective(glm::radians(42.0f), 1.0f, 0.05f, 1.0f);

            // Get and set up shaders
            RepeatShader = Shader("../res/shaders/PTVS.glsl", "../res/shaders/PTFSRepeat.glsl");
            SubtractionShader = Shader("../res/shaders/SubtractionVertexShader.glsl", "../res/shaders/SubtractionFragmentShader.glsl");
            RTTShader = Shader("../res/shaders/RTTVShader.glsl", "../res/shaders/RTTFShader.glsl");
            R2Shader = Shader("../res/shaders/PassThroughQuadVertexShader.glsl", "../res/shaders/Reduction2FShader.glsl");
            PTShader = Shader("../res/shaders/PTVS.glsl", "../res/shaders/PTFS.glsl");

            // Load the foot model 
						footHigh = Model("../res/models/foot_1.obj");
						footMid = Model("../res/models/foot_3.obj");
						footLow = Model("../res/models/foot_5.obj");

            float translations[numParticles];
            for (int i = 0; i < NumParticles; i++)
            {
                translations[i] = i*2.0/NumParticles;
            }

            // set up the instance VBO for offsets
            glGenBuffers(1, &instanceVBO);
            glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)*NumParticles, &translations[0], GL_STATIC_DRAW);

            glBindVertexArray(footModel.meshes[0].VAO);
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribDivisor(5, 1);

            // set up the instance VBO for model matricies
            glGenBuffers(1, &transformationInstanceBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, transformationInstanceBuffer);
            glBufferData(GL_ARRAY_BUFFER, NumParticles*sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);

            glBindVertexArray(footModel.meshes[0].VAO);
            GLsizei vec4Size = sizeof(glm::vec4);

            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4*vec4Size, (void*)0);
            glEnableVertexAttribArray(7);
            glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4*vec4Size, (void*)(vec4Size));
            glEnableVertexAttribArray(8);
            glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4*vec4Size, (void*)(2*vec4Size));
            glEnableVertexAttribArray(9);
            glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4*vec4Size, (void*)(3*vec4Size));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribDivisor(6, 1);
            glVertexAttribDivisor(7, 1);
            glVertexAttribDivisor(8, 1);
            glVertexAttribDivisor(9, 1);

            glBindVertexArray(0);

            float quadVertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 0.0f,
                -1.0f, -1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 1.0f,

                -1.0f,  1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 1.0f,
                1.0f,  1.0f,  1.0f, 0.0f
            };

            float repeatQuadVertices[] = {
                // positions   // texCoords
                -1.0f,  1.0f,  0.0f, 0.0f,
                -1.0f, -1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f*NumParticles, 1.0f,

                -1.0f,  1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f*NumParticles, 1.0f,
                1.0f,  1.0f,  1.0f*NumParticles, 0.0f
            };

            // declare quad
            glGenVertexArrays(1, &quadVAO);
            glGenBuffers(1, &quadVBO);
            glBindVertexArray(quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glBindVertexArray(0);

            // declare repeat quad
            glGenVertexArrays(1, &repeatQuadVAO);
            glGenBuffers(1, &repeatQuadVBO);
            glBindVertexArray(repeatQuadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, repeatQuadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(repeatQuadVertices), &repeatQuadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            glBindVertexArray(0);

            // set up peng 
            glGenFramebuffers(1, &peng);
            glBindFramebuffer(GL_FRAMEBUFFER, peng);

            // reference depth map on repeat
            glGenTextures(1, &repeattex);
            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, repeattex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*128, 128, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, repeattex, 0);

            // set up ping
            glGenFramebuffers(1, &ping);
            glBindFramebuffer(GL_FRAMEBUFFER, ping);

            // rendered models in line
            glGenTextures(1, &depthtexture);
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, depthtexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*128, 128, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthtexture, 0);

            // set up pong 
            glGenFramebuffers(1, &pong);
            glBindFramebuffer(GL_FRAMEBUFFER, pong);

            // texture that is the difference of reference and rendered
            glGenTextures(1, &difftex);
            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_2D, difftex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*128, 128, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, difftex, 0);

            // set up pang 
            glGenFramebuffers(1, &pang);
            glBindFramebuffer(GL_FRAMEBUFFER, pang);

            // Nx64x64 texture
            glGenTextures(1, &tex64);
            glActiveTexture(GL_TEXTURE0 + 4);
            glBindTexture(GL_TEXTURE_2D, tex64);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*64, 64, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex64, 0);

            // set up pung
            glGenFramebuffers(1, &pung);
            glBindFramebuffer(GL_FRAMEBUFFER, pung);

            // Nx32x32 texture
            glGenTextures(1, &tex32);
            glActiveTexture(GL_TEXTURE0 + 5);
            glBindTexture(GL_TEXTURE_2D, tex32);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*32, 32, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex32, 0);

            // set up pling	
            glGenFramebuffers(1, &pling);
            glBindFramebuffer(GL_FRAMEBUFFER, pling);

            // Nx16x16 texture
            glGenTextures(1, &tex16);
            glActiveTexture(GL_TEXTURE0 + 6);
            glBindTexture(GL_TEXTURE_2D, tex16);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*16, 16, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex16, 0);

            // set up plang
            glGenFramebuffers(1, &plang);
            glBindFramebuffer(GL_FRAMEBUFFER, plang);

            // Nx8x8 texture
            glGenTextures(1, &tex8);
            glActiveTexture(GL_TEXTURE0 + 7);
            glBindTexture(GL_TEXTURE_2D, tex8);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*8, 8, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex8, 0);

            // set up plong
            glGenFramebuffers(1, &plong);
            glBindFramebuffer(GL_FRAMEBUFFER, plong);

            // Nx4x4 texture
            glGenTextures(1, &tex4);
            glActiveTexture(GL_TEXTURE0 + 8);
            glBindTexture(GL_TEXTURE_2D, tex4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*4, 4, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex4, 0);

            // set up plung
            glGenFramebuffers(1, &plung);
            glBindFramebuffer(GL_FRAMEBUFFER, plung);

            // Nx2x2 texture
            glGenTextures(1, &tex2);
            glActiveTexture(GL_TEXTURE0 + 9);
            glBindTexture(GL_TEXTURE_2D, tex2);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*2, 2, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex2, 0);

            // set up pleng
            glGenFramebuffers(1, &pleng);
            glBindFramebuffer(GL_FRAMEBUFFER, pleng);

            // Nx1x1 texture
            glGenTextures(1, &tex1);
            glActiveTexture(GL_TEXTURE0 + 10);
            glBindTexture(GL_TEXTURE_2D, tex1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, NumParticles*1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex1, 0);
        }

        PoseParameters Run(PoseParameters* parameterList, float* refImg, int iters)
        {	
            // Load reference image into texture 0
            glGenTextures(1, &refdepthtex);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, refdepthtex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 128, 128, 0, GL_DEPTH_COMPONENT, GL_FLOAT, refImg);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);


            glEnable(GL_DEPTH_TEST);

            // Intialize particles
            Particle* particles = new Particle[NumParticles];
            for (int i = 0; i < NumParticles; i++)
            {
                particles[i].Position = parameterList[i];
            }

						// BEGIN TESTING CODE
						
            //glm::mat4* Movements = new glm::mat4[NumParticles];
            //for (int i = 0; i < NumParticles; i++)
            //{
            //    PoseParameters currparam = particles[i].Position;
            //    // Set up MVP matricies
            //    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(currparam.XTranslation, currparam.YTranslation, currparam.ZTranslation));
            //    model = glm::rotate(glm::rotate(glm::rotate(model, currparam.XRotation, glm::vec3(1, 0, 0)), currparam.YRotation, glm::vec3(0, 1, 0)), currparam.ZRotation, glm::vec3(0, 0, 1));
            //    Movements[i] = model;	
            //}

            //glNamedBufferSubData(transformationInstanceBuffer, 0, NumParticles*sizeof(glm::mat4), &Movements[0]);


						//while (!glfwWindowShouldClose(window))
						//{
            //  RepeatShader.use();
            //  RepeatShader.setInt("tex", 0);
            //  glBindFramebuffer(GL_FRAMEBUFFER, peng);
            //  glClear(GL_DEPTH_BUFFER_BIT);
            //  glBindVertexArray(repeatQuadVAO);
            //  glDrawArrays(GL_TRIANGLES, 0, 6);

            //  glEnable(GL_DEPTH_TEST);
            //  //Send matricies to shader
            //  RTTShader.use(); 
            //  RTTShader.setMat4("u_P", ProjMat);
            //  RTTShader.setInt("instances", NumParticles);
            //  RTTShader.setFloat("zNear", 0.05f);
            //  RTTShader.setFloat("zFar", 1.0f);

            //  glBindFramebuffer(GL_FRAMEBUFFER, ping);
            //  glClear(GL_DEPTH_BUFFER_BIT);

            //  RTTShader.use();
            //  Mesh footMesh = footModel.meshes[0];
            //  glBindVertexArray(footMesh.VAO);
            //  glDrawElementsInstanced(GL_TRIANGLES, footMesh.indices.size(), GL_UNSIGNED_INT, 0, NumParticles);

            //  glBindFramebuffer(GL_FRAMEBUFFER, pong);
            //  glClear(GL_DEPTH_BUFFER_BIT);
            //  SubtractionShader.use();
            //  SubtractionShader.setInt("screenTexture", 1);
            //  SubtractionShader.setInt("gendepTexture", 2);
            //  glBindVertexArray(quadVAO);
            //  glDrawArrays(GL_TRIANGLES, 0, 6);

						//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
						//	glClear(GL_DEPTH_BUFFER_BIT);
						//	PTShader.use();
						//	PTShader.setInt("tex", 3);
						//	glBindVertexArray(quadVAO);
						//	glDrawArrays(GL_TRIANGLES, 0, 6);
						//
						//	glfwSwapBuffers(window);
						//	glfwPollEvents();
						//}








						// END TESTING CODE


            // Time the PSO without setup
            auto start = std::chrono::high_resolution_clock::now();
            // Setup finished, start the particle swarm!
            PoseParameters GlobalBestPosition;
            float GlobalBestEnergy = std::numeric_limits<float>::infinity();

            for (int generation = 0; generation < iters; generation++)
            {
                glm::mat4* Movements = new glm::mat4[NumParticles];
                for (int i = 0; i < NumParticles; i++)
                {
                    PoseParameters currparam = particles[i].Position;
                    // Set up MVP matricies
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(currparam.XTranslation, currparam.YTranslation, currparam.ZTranslation));
                    model = glm::rotate(glm::rotate(glm::rotate(model, currparam.XRotation, glm::vec3(1, 0, 0)), currparam.YRotation, glm::vec3(0, 1, 0)), currparam.ZRotation, glm::vec3(0, 0, 1));
                    Movements[i] = model;	
                }

                glNamedBufferSubData(transformationInstanceBuffer, 0, NumParticles*sizeof(glm::mat4), &Movements[0]);

                RepeatShader.use();
                RepeatShader.setInt("tex", 0);
                glBindFramebuffer(GL_FRAMEBUFFER, peng);
                glClear(GL_DEPTH_BUFFER_BIT);
                glBindVertexArray(repeatQuadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glEnable(GL_DEPTH_TEST);
                //Send matricies to shader
                RTTShader.use(); 
                RTTShader.setMat4("u_P", ProjMat);
                RTTShader.setInt("instances", NumParticles);
                RTTShader.setFloat("zNear", 0.05f);
                RTTShader.setFloat("zFar", 1.0f);

                glBindFramebuffer(GL_FRAMEBUFFER, ping);
                glClear(GL_DEPTH_BUFFER_BIT);

                RTTShader.use();
                Mesh footMesh = footModel.meshes[0];
                glBindVertexArray(footMesh.VAO);
                glDrawElementsInstanced(GL_TRIANGLES, footMesh.indices.size(), GL_UNSIGNED_INT, 0, NumParticles);

                glBindFramebuffer(GL_FRAMEBUFFER, pong);
                glClear(GL_DEPTH_BUFFER_BIT);
                SubtractionShader.use();
                SubtractionShader.setInt("screenTexture", 1);
                SubtractionShader.setInt("gendepTexture", 2);
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                glBindFramebuffer(GL_FRAMEBUFFER, pang);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 3);
                R2Shader.setFloat("width", NumParticles*128.0f);
                R2Shader.setFloat("height", 128.0f);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, pung);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 4);
                R2Shader.setFloat("width", NumParticles*64.0f);
                R2Shader.setFloat("height", 64.0f);
                glViewport(0, 0, NumParticles*64, 64);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, pling);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 5);
                R2Shader.setFloat("width", NumParticles*32.0f);
                R2Shader.setFloat("height", 32.0f);
                glViewport(0, 0, NumParticles*32, 32);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, plang);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 6);
                R2Shader.setFloat("width", NumParticles*16.0f);
                R2Shader.setFloat("height", 16.0f);
                glViewport(0, 0, NumParticles*16, 16);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, plong);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 7);
                R2Shader.setFloat("width", NumParticles*8.0f);
                R2Shader.setFloat("height", 8.0f);
                glViewport(0, 0, NumParticles*8, 8);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, plung);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 8);
                R2Shader.setFloat("width", NumParticles*4.0f);
                R2Shader.setFloat("height", 4.0f);
                glViewport(0, 0, NumParticles*4, 4);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, pleng);
                glClear(GL_DEPTH_BUFFER_BIT);
                R2Shader.use();
                R2Shader.setInt("tex", 9);
                R2Shader.setFloat("width", NumParticles*2.0f);
                R2Shader.setFloat("height", 2.0f);
                glViewport(0, 0, NumParticles*2, 2);
                glDrawArrays(GL_TRIANGLES, 0 , 6);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glClear(GL_DEPTH_BUFFER_BIT);
                PTShader.use();
                PTShader.setInt("tex", 9);
                glBindVertexArray(quadVAO);
                glViewport(0, 0, NumParticles*128, 128);
                glDrawArrays(GL_TRIANGLES, 0, 6);

                float* currentdt = new float[NumParticles];
                glGetTextureImage(tex1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, sizeof(float)*NumParticles, currentdt);

                // first loop to update local bests and global best
                for (int p = 0; p < NumParticles; p++)
                {
                    if (currentdt[p] < particles[p].BestEnergyScore)
                    {
                        particles[p].BestEnergyScore = currentdt[p];
                        particles[p].BestPosition = particles[p].Position;
                        if (currentdt[p] < GlobalBestEnergy)
                        {
                            GlobalBestEnergy = currentdt[p];
                            GlobalBestPosition = particles[p].Position;
                        }
                        std::cout << "cie: " << currentdt[p]*128*128 << std::endl;
                        std::cout << "cbes for particle " << p << ": " << particles[p].BestEnergyScore*128*128 << std::endl;
                    }
										//std::cout << "cie: " << currentdt[p]*128*128 << std::endl;
                    //std::cout << "cbes for particle " << p << ": " << particles[p].BestEnergyScore*128*128 << std::endl;
                }
                std::cout << "gbe: " << GlobalBestEnergy*128*128 << std::endl;

                // second loop to update position and velocities
                for (int p = 0; p < NumParticles; p++)
                {
                    float r1 = ((float) std::rand() / RAND_MAX);
                    float r2 = ((float) std::rand() / RAND_MAX);
                    PoseParameters personalVelocity = (particles[p].BestPosition - particles[p].Position)*CognitiveConst*r1;
										PoseParameters socialVelocity = (GlobalBestPosition - particles[p].Position)*SocialConst*r2;
										particles[p].Velocity = particles[p].Velocity + (personalVelocity + socialVelocity)*ConstrictionConst;
										//particles[p].Velocity.Assuage(0.05, 0.05, 0.05, 0.1, 0.1, 0.1);
										//std::cout << "velocity for particle: " << p << ": " << std::endl; particles[p].Velocity.Print();
                    particles[p].Position = particles[p].Position + particles[p].Velocity; 
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            //std::cout << "Time it took for PSO to execute without OpenGL setup is: " << std::chrono::duration_cast<std::chrono::milliseconds> (end-start).count() << std::endl;
            return GlobalBestPosition;
        }
};
