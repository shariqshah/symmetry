#include "renderer.h"
#include "GLFW/glfw3.h"

#include "log.h"

void on_framebuffer_size_change(GLFWwindow* window, int width, int height);

void renderer_init(GLFWwindow* window)
{
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glfwSetFramebufferSizeCallback(window, on_framebuffer_size_change);
}

void renderer_draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_cleanup(void)
{
	
}

void on_framebuffer_size_change(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void renderer_set_clearcolor(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

void renderer_check_glerror(const char* context)
{
	GLenum error = glGetError();
	const char* errorString = "No Error";
	switch(error)
	{
	case GL_INVALID_OPERATION:
		errorString = "Invalid Operation";
		break;
	case GL_NO_ERROR:
		errorString = "No Error";
		break;
	case GL_INVALID_ENUM:
		errorString = "Invalid ENUM";
		break;
	case GL_INVALID_VALUE:
		errorString = "Invalid Value";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		errorString = "Invalid FrameBuffer Operation";
		break;
	case GL_OUT_OF_MEMORY:
		errorString = "Out of Memory";
		break;
	case GL_STACK_UNDERFLOW:
		errorString = "Stack Underflow";
		break;
	case GL_STACK_OVERFLOW:
		errorString = "Stack Overflow";
		break;
	}

	if(error != GL_NO_ERROR)
	{
		log_error(context, errorString);
	}
}
