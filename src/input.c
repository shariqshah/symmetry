#include <assert.h>
#include "input.h"
#include "GLFW/glfw3.h"

#include "window_system.h"
#include "log.h"

void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods);
void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos);

void input_init(GLFWwindow* window)
{
	glfwSetMouseButtonCallback(window, input_on_mousebutton);
	glfwSetKeyCallback(window, input_on_key);
	glfwSetCursorPosCallback(window, input_on_cursor_move);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
}

void input_cleanup(void)
{
	
}

void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos)
{
	
}

void input_get_cursor_pos(double* xpos, double* ypos)
{
	assert(xpos && ypos);
	GLFWwindow* window = window_get_active();
	glfwGetCursorPos(window, xpos, ypos);
}

void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	
}

void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
	
}

void input_set_cursor_mode(Cursor_Mode mode)
{
	GLFWwindow* window = window_get_active();
	int cursor_mode = GLFW_CURSOR_NORMAL;
	if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_HIDDEN;
	else if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_DISABLED;
	
	glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
}

bool input_get_key_state(int key, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetKey(window, key);
	return current_state == state_type ? true : false;
}

bool input_get_mousebutton_state(int button, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetMouseButton(window, button);
	return current_state == state_type ? true : false;
}

