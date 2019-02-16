#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model.h"
#include "pso.h"

static const float PI = 3.1415926535;
static const int windowWidth = 640;
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

static float* CropImage(float* originalImage, const int& originalWidth, const int& originalHeight, const int& newWidth, const int& newHeight)
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

float** GenerateMapsFromParticles(int numParams, Particle* particles)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return NULL;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Testing", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return NULL;
	}

	// Make the window's context current and then hide it 
	glfwMakeContextCurrent(window);
	glfwHideWindow(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return NULL;

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
		return NULL;
	}

	// Save image into array
	float** depthImages = new float*[numParams];

	// Load model
	const char* footModelPath = "../res/foot.obj";
	Model footModel(footModelPath);

	float zNear = 0.05f;
	float zFar = 1.0f;
	
	for (int i = 0; i < numParams; i++) {
		// allocate space for depth image
		float* depthImageFromRenderbuffer = new float[windowWidth*windowHeight];
		
		/* Render here */
		glClear(GL_DEPTH_BUFFER_BIT);
		
		// Set up MVP matricies
		PoseParameters params = particles[i].Position;
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(params.XTranslation, params.YTranslation, params.ZTranslation));
		model = glm::rotate(glm::rotate(glm::rotate(model, params.XRotation, glm::vec3(1, 0, 0)), params.YRotation, glm::vec3(0, 1, 0)), params.ZRotation, glm::vec3(0, 0, 1));
		model = glm::scale(model, glm::vec3(params.Scale));
		glm::mat4 proj = glm::perspective(58.59f, 1.778f, zNear, zFar);
		// Set up shader
		RTTShader.use();
		RTTShader.setMat4("u_M", model);
		RTTShader.setMat4("u_P", proj);
		RTTShader.setMat4("u_P_F", proj);
		RTTShader.setFloat("zNear", zNear);
		RTTShader.setFloat("zFar", zFar);
		footModel.Draw(RTTShader);
	
		// Render to our framebuffer
		glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, depthrenderbuffer);

		// Save image
		glReadPixels(0, 0, windowWidth, windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depthImageFromRenderbuffer);		
		float* croppedDM = CropImage(depthImageFromRenderbuffer, 640, 360, 200, 200);
		depthImages[i] = croppedDM;
	}

	return depthImages;
}

float** GenerateMapsFromPoseParameters(int numParams, PoseParameters* poseparams)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return NULL;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL Testing", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return NULL;
	}

	// Make the window's context current and then hide it
	glfwMakeContextCurrent(window);
	glfwHideWindow(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return NULL;

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
		return NULL;
	}

	// Save image into array
	float** depthImages = new float*[numParams];

	// Load model
	const char* footModelPath = "../res/foot.obj";
	Model footModel(footModelPath);

	float zNear = 0.1f;
	float zFar = 1.0f;

	for (int i = 0; i < numParams; i++) {
		// allocate space for depth image
		float* depthImageFromRenderbuffer = new float[windowWidth*windowHeight];

		/* Render here */
		glClear(GL_DEPTH_BUFFER_BIT);

		// Set up MVP matricies
		PoseParameters params = poseparams[i];
		glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(params.XTranslation, params.YTranslation, params.ZTranslation));
		model = glm::rotate(glm::rotate(glm::rotate(model, params.XRotation, glm::vec3(1, 0, 0)), params.YRotation, glm::vec3(0, 1, 0)), params.ZRotation, glm::vec3(0, 0, 1));
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

		// Save image
		glReadPixels(0, 0, windowWidth, windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT, depthImageFromRenderbuffer);
		float* croppedDM = CropImage(depthImageFromRenderbuffer, 640, 360, 200, 200);
		depthImages[i] = croppedDM;
	}

	return depthImages;
}
