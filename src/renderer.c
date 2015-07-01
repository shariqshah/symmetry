#include "renderer.h"
#include "GLFW/glfw3.h"

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
