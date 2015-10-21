#include "renderer.h"
#include "GLFW/glfw3.h"

#include "log.h"
#include "camera.h"
#include "model.h"

static int default_fbo = -1;

void on_framebuffer_size_change(GLFWwindow* window, int width, int height);

void renderer_init(GLFWwindow* window)
{
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glfwSetFramebufferSizeCallback(window, on_framebuffer_size_change);
}

void renderer_draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/* Hard coded version */
	struct Camera* camera = camera_get(0);
	model_render_all(camera);
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

int renderer_check_glerror(const char* context)
{
	int error = 1;
	GLenum error_code = glGetError();
	const char* errorString = "No Error";
	switch(error_code)
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

	if(error_code != GL_NO_ERROR)
		log_error(context, errorString);
	else
		error = 0;

	return error;
}
