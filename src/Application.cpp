#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include "objloader.hpp"

static const float PI = 3.1415926;

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

static std::string ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);
	std::string line;
	std::stringstream ss;
	while (getline(stream, line))
	{
		ss << line << '\n';
	}
	return ss.str(); 
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cerr << "Failed to compile " << source << std::endl;
		std::cerr << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}


static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(480, 360, "Hello World", NULL, NULL);
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

	// Load the foot mesh data
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	loadOBJ("../res/foot_mesh.obj", vertices, uvs, normals);

	// Create buffer for foot mesh vertex data
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

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

	// Get and set up the shaders
	std::string vs = ParseShader("../res/shaders/VertexShader.glsl");
	std::string fs = ParseShader("../res/shaders/FragmentShader.glsl");

	unsigned int shader = CreateShader(vs, fs);
	glUseProgram(shader);
	
	int modelloc = glGetUniformLocation(shader, "u_M");
	glUniformMatrix4fv(modelloc, 1, GL_FALSE, &model[0][0]);

	int viewloc = glGetUniformLocation(shader, "u_V");
	glUniformMatrix4fv(viewloc, 1, GL_FALSE, &view[0][0]);

	int projloc = glGetUniformLocation(shader, "u_P");
	glUniformMatrix4fv(projloc, 1, GL_FALSE, &proj[0][0]);

	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer
	GLuint FramebufferName = 0;
	glGenFramebuffers(1, &FramebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

	// The texture we're going to render to
	//GLuint renderedTexture;
	//glGenTextures(1, &renderedTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	//glBindTexture(GL_TEXTURE_2D, renderedTexture);
	// Give an empty image to OpenGL ( the last "0" )
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 480, 360, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glBindTexture(GL_TEXTURE_2D, 0);
	
	// The depth buffer 
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 480, 360);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Alternative: Depth texture. Slower, but you can sample it later in your shader
	GLuint depthTexture;
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 480, 360, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	// Some copied texture 2D parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);	
	// Configuring the framebuffer
	// Set renderedTexture as our color attachment #0
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderedTexture, 0);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
	
	// For the depth map
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

	// Get the list of draw buffers
	//GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	GLenum DrawBuffers[1] = {GL_DEPTH_ATTACHMENT};
	glDrawBuffers(1, DrawBuffers); // 1 is the size of DrawBuffers
	
	// check if the framebuffer is OK
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) 
	{
		std::cerr << "framebuffer setup was not successful" << std::endl;
		return -1;
	}

	// Save image only once in the loop
	bool didSaveImage = false;
	float* depthImage = new float[480*360];

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// Render to our framebuffer
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glBindFramebuffer(GL_FRAMEBUFFER, depthTexture);
		glViewport(0, 0, 480, 360); // Render on the whole framebuffer, complete from the lower left corner to the upper right
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Drawing the 3D model
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());	

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	
		// Save image
		if (!didSaveImage)
		{
			glGetTexImage(GL_TEXTURE_2D, 0,	GL_DEPTH_COMPONENT, GL_FLOAT, depthImage);
			for (int i = 0; i < 480*360; i++)
			{
				std::cout << depthImage[i] << " ";
			}
			didSaveImage = true;
		}
	}

	glfwTerminate();
	return 0;
}
