#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "shader.h"
#include "model.h"

static const float PI = 3.1415926;
static const int windowWidth = 480;
static const int windowHeight = 360;

//static void GLClearError()
//{
//	while(glGetError() != GL_NO_ERROR);
//}
//
//static void GLCheckError()
//{
//	while(GLenum error = glGetError()) 
//	{
//		std::cerr << "[OpenGL Error]:" << error << std::endl;
//	}
//}

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

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Testing", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return -1;

	// Get and set up the shaders
	const char* RTTVertexShaderPath = "../res/shaders/RTTVShader.glsl";
	const char* RTTFragmentShaderPath = "../res/shaders/RTTFShader.glsl";

	Shader RTTShader(RTTVertexShaderPath, RTTFragmentShaderPath);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The depth buffer 
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowWidth, windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// No color buffer is drawn to
	glDrawBuffer(GL_NONE);
	
	// check if the framebuffer is OK
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		std::cerr << "framebuffer setup was not successful" << std::endl;
		return -1;
	}

	// Save image only once in the loop
	// TODO: CHANGE TO OFFLINE
	bool didSaveImage = false;
	float* depthImageFromRenderbuffer = new float[windowWidth*windowHeight];

	// Load model
	const char* footModelPath = "../res/foot.obj";
	Model footModel(footModelPath);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Drawing the 3D modelfrom
		float zNear = 0.1f;
		float zFar = 1.0f;
		// Rotations are in radians
		float rotateX = PI/2 + PI/8;
		float rotateY = PI/2;
		float rotateZ = 0;
		float translateX = -0.1f; // left right
		float translateY = 0.0f; // up down
		float translateZ = -0.15f; // near far
		// Set up MVP matricies
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(translateX, translateY, translateZ));
		model = glm::rotate(glm::rotate(glm::rotate(model, rotateX, glm::vec3(1, 0, 0)), rotateY, glm::vec3(0, 1, 0)), rotateZ, glm::vec3(0, 0, 1));
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.05f));
		glm::mat4 proj = glm::perspective(58.59f, 1.778f, zNear, zFar);

		// Set up shader
		RTTShader.use();
		RTTShader.setMat4("u_M", model);
		RTTShader.setMat4("u_V", view);
		RTTShader.setMat4("u_P", proj);
		RTTShader.setMat4("u_P_F", proj);
		RTTShader.setFloat("zNear", zNear);
		RTTShader.setFloat("zFar", zFar);
		footModel.Draw(RTTShader);

		// Render to our framebuffer
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, depthrenderbuffer);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	
		// Save image
		if (!didSaveImage)
		{
			glReadPixels(0, 0, windowWidth, windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depthImageFromRenderbuffer);

			const char* outputPath = "/home/eric/Dev/dm.txt";
			WriteToFile(depthImageFromRenderbuffer, windowWidth, windowHeight, outputPath);
			didSaveImage = true;
		}
	}
}
