#include "window_system.h"
#include "GLFW/glfw3.h"

#include "log.h"

static GLFWwindow*      active_window = NULL;
static on_window_close  window_close_custom  = NULL;
static on_window_resize window_resize_custom = NULL;

void window_error_callback(int error, const char* description);
void window_resize(GLFWwindow* window, int width, int height);
void window_close_callback(GLFWwindow* window);

bool window_init(const char* title, int width, int height)
{
	bool success = true;
	glfwSetErrorCallback(window_error_callback);
	if(!glfwInit())
	{
		log_error("window_create", "Initializing glfw failed");
		success = false;
	}
	else
	{
		log_message("Initialized with GLFW version %d.%d.%d",
					GLFW_VERSION_MAJOR,
					GLFW_VERSION_MINOR,
					GLFW_VERSION_REVISION);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		active_window = glfwCreateWindow(width, height, title, NULL, NULL);
		if(!active_window)
		{
			log_error("window_create", "Failed to create window");
			success = false;
		}
		else
		{
			glfwMakeContextCurrent(active_window);
			glfwSwapInterval(1);
			glfwSetWindowSizeCallback(active_window, window_resize);
			glfwSetWindowCloseCallback(active_window, window_close_callback);
		}
	}
	return success;
}

void window_error_callback(int error, const char* description)
{
	log_error("GLFW", "(%d) %s", error, description);
}

void window_close_callback(GLFWwindow* window)
{
	if(!window_close_custom)
		glfwSetWindowShouldClose(window, GL_TRUE);
	else
		window_close_custom();
}

void window_cleanup(void)
{
	if(active_window) glfwDestroyWindow(active_window);
	glfwTerminate();
}

void window_poll_events(void)
{
	glfwPollEvents();
}

void window_swap_buffers(void)
{
	glfwSwapBuffers(active_window);
}

void window_set_size(int width, int height)
{
	glfwSetWindowSize(active_window, width, height);
}

void window_resize(GLFWwindow* window, int width, int height)
{
	/* Maybe resize main frame buffer here? */
	if(window_resize_custom) window_resize_custom(width, height);
}

GLFWwindow* window_get_active(void)
{
	return active_window;
}

bool window_should_close(void)
{
	return glfwWindowShouldClose(active_window);
}
