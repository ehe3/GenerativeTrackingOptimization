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
	const char* vertexPath = "../res/shaders/VertexShader.glsl";
	const char* fragmentPath = "../res/shaders/FragmentShader.glsl"; 

	Shader shader(vertexPath, fragmentPath);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	//// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer
	//GLuint FramebufferName = 0;
	//glGenFramebuffers(1, &FramebufferName);
	//glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
	//
	//// The depth buffer 
	//GLuint depthrenderbuffer;
	//glGenRenderbuffers(1, &depthrenderbuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, windowHeight, windowHeight);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	//// Depth texture. Slower, but you can sample it later in your shader
	//GLuint depthTexture;
	//glGenTextures(1, &depthTexture);
	//glBindTexture(GL_TEXTURE_2D, depthTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	//
	//// Some copied texture 2D parameters
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glBindTexture(GL_TEXTURE_2D, 0);	
	//
	//// For the depth map
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	//// Get the list of draw buffers
	//GLenum DrawBuffers[1] = {GL_DEPTH_ATTACHMENT};
	//glDrawBuffers(1, DrawBuffers); // 1 is the size of DrawBuffers
	//
	//// check if the framebuffer is OK
	//if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	//{
	//	std::cerr << "framebuffer setup was not successful" << std::endl;
	//	return -1;
	//}

	//// Save image only once in the loop
	//// TODO: CHANGE TO OFFLINE
	//bool didSaveImage = false;
	//float* depthImage = new float[windowWidth*windowHeight];

	// Load model
	const char* footModelPath = "../res/foot.obj";
	Model footModel(footModelPath);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// Render to our framebuffer
		//glBindTexture(GL_TEXTURE_2D, depthTexture);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glViewport(0, 0, windowWidth, windowHeight); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		/* Render here */
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Drawing the 3D model
		// Rotations are in radians
		float rotateX = PI/2 + PI/8;
		float rotateY = 0;
		float rotateZ = 0;
		float translateX = -0.2f; // left right
		float translateY = 0.0f; // up down
		float translateZ = -0.1f; // near far
		// Set up MVP matricies
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(translateX, translateY, translateZ));
		model = glm::rotate(glm::rotate(glm::rotate(model, rotateX, glm::vec3(1, 0, 0)), rotateY, glm::vec3(0, 1, 0)), rotateZ, glm::vec3(0, 0, 1));
		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.25f));
		glm::mat4 proj = glm::perspective(58.59f, 1.778f, 0.001f, 10.0f);

		shader.use();

		shader.setMat4("u_M", model);
		shader.setMat4("u_V", view);
		shader.setMat4("u_P", proj);

		footModel.Draw(shader);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	
		// Save image
		//if (!didSaveImage)
		//{
		//	glGetTexImage(GL_TEXTURE_2D, 0,	GL_DEPTH_COMPONENT, GL_FLOAT, depthImage);
		//	for (int i = 0; i < windowWidth*windowHeight; i++)
		//	{
		//		//std::cout << depthImage[i] << " ";
		//	}
		//	didSaveImage = true;
		//}
	}

	glfwTerminate();
	return 0;
}
