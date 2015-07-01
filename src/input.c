#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "GLFW/glfw3.h"

#include "window_system.h"
#include "log.h"

#define KS_INACTIVE -1; 			/* state for input map is set to KS_INACTIVE(KeyState_Inactive) when
									   the key is neither pressed nor released */

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods);
static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos);
static int  map_find(const char* name);

static Array* input_map_list;

void input_init(GLFWwindow* window)
{
	glfwSetMouseButtonCallback(window, input_on_mousebutton);
	glfwSetKeyCallback(window, input_on_key);
	glfwSetCursorPosCallback(window, input_on_cursor_move);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	input_map_list = array_new(Input_Map);
}

void input_cleanup(void)
{
	for(unsigned int i = 0; i < input_map_list->length; i++)
	{
		Input_Map* map = array_get(input_map_list, i);
		array_free(map->keys);
	}
	array_free(input_map_list);
}

static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos)
{
	
}

void input_get_cursor_pos(double* xpos, double* ypos)
{
	assert(xpos && ypos);
	GLFWwindow* window = window_get_active();
	glfwGetCursorPos(window, xpos, ypos);
}

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	for(unsigned int i = 0; i < input_map_list->length; i++)
	{
		Input_Map* map = array_get(input_map_list, i);
		for(unsigned int j = 0; j < map->keys->length; j++)
		{
			int map_key = array_get_val(map->keys, int, j);
			if(map_key == key)
			{
				map->state = action;
				break;
			}
		}
	}
}

static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
	
}

void input_cursor_mode_set(Cursor_Mode mode)
{
	GLFWwindow* window = window_get_active();
	int cursor_mode = GLFW_CURSOR_NORMAL;
	if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_HIDDEN;
	else if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_DISABLED;
	
	glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
}

bool input_map_state_get(const char* map_name, int state)
{
	int current_state = KS_INACTIVE;
	for(unsigned int i = 0; i < input_map_list->length; i++)
	{
		Input_Map* map = array_get(input_map_list, i);
		if(strcmp(map->name, map_name) == 0)
		{
			current_state = map->state;
			break;
		}
	}
	return state == current_state ? true : false;
}

bool inputkey_state_get(int key, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetKey(window, key);
	return current_state == state_type ? true : false;
}

bool input_mousebutton_state_get(int button, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetMouseButton(window, button);
	return current_state == state_type ? true : false;
}

void input_map_create(const char* name, int* keys, size_t num_keys)
{
	assert(name && keys && num_keys > 0);

	Input_Map* new_map = array_add(input_map_list);
	new_map->name  = name;
	new_map->keys  = array_new(int);
	new_map->state = KS_INACTIVE;
	for(size_t i = 0; i < num_keys; i++)
	{
		int* key = array_add(new_map->keys);
		*key = keys[i];
	}
}

void input_update(void)
{
	for(unsigned int i = 0; i < input_map_list->length; i++)
	{
		Input_Map* map = array_get(input_map_list, i);
		if(map->state == GLFW_RELEASE)
			map->state = KS_INACTIVE;
	}
}

bool input_map_remvove(const char* name)
{
	assert(name);
	bool success = false;
    int index = map_find(name);
	if(index > -1)
	{
		array_remove_at(input_map_list, (unsigned int)index);
		success = true;
	}	
	if(!success) log_error("input:map_remove", "Map %s not found", name);
	
	return success;
}

bool input_map_keys_set(const char* name, int* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);
	bool success = false;
	int index = map_find(name);
	if(index > -1)
	{
		Input_Map* map = array_get(input_map_list, (unsigned int)index);
		array_reset(map->keys, num_keys);
		for(int i = 0; i < num_keys; i++)
		{
			int* key = array_get(map->keys, (unsigned int)i);
			*key = keys[i];
		}
		success = true;
	}
	if(!success) log_error("input:map_set_keys", "Map %s not found", name);
	
	return success;
}

static int map_find(const char* name)
{
	int index = -1;
	for(unsigned int i = 0; i < input_map_list->length; i++)
	{
		Input_Map* map = array_get(input_map_list, i);
		if(strcmp(name, map->name) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}
