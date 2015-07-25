#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "array.h"
#include "GLFW/glfw3.h"

#include "window_system.h"
#include "log.h"

#define KS_INACTIVE -1; 			/* state for input map is set to KS_INACTIVE(KeyState_Inactive) when
									   the key is neither pressed nor released */

struct Input_Map
{
	const char* name;
	int* keys;
	int  state;
};

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods);
static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos);
static int  map_find(const char* name);

static struct Input_Map* input_map_list;

void input_init(GLFWwindow* window)
{
	glfwSetMouseButtonCallback(window, input_on_mousebutton);
	glfwSetKeyCallback(window, input_on_key);
	glfwSetCursorPosCallback(window, input_on_cursor_move);

	input_map_list = array_new(struct Input_Map);
}

void input_cleanup(void)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		array_free(map->keys);
	}
	array_free(input_map_list);
}

static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos)
{
	
}

void input_cursor_pos_get(double* xpos, double* ypos)
{
	assert(xpos && ypos);
	GLFWwindow* window = window_get_active();
	glfwGetCursorPos(window, xpos, ypos);
}

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		for(int j = 0; j < array_len(map->keys); j++)
		{
			if(map->keys[i] == key)
			{
				map->state = action;
				break;
			}
		}
	}
}

static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
	/* Probably add 'mouse maps', same as input maps for keyvboard but with buttons
	   Do we even need that?
	*/
}

void input_cursor_mode_set(enum Cursor_Mode mode)
{
	GLFWwindow* window = window_get_active();
	int cursor_mode = GLFW_CURSOR_NORMAL;
	if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_HIDDEN;
	else if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_DISABLED;
	
	glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
}

int input_map_state_get(const char* map_name, int state)
{
	int current_state = KS_INACTIVE;
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(strcmp(map->name, map_name) == 0)
		{
			current_state = map->state;
			break;
		}
	}
	return state == current_state ? 1 : 0;
}

int input_key_state_get(int key, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetKey(window, key);
	return current_state == state_type ? 1 : 0;
}

int input_mousebutton_state_get(int button, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetMouseButton(window, button);
	return current_state == state_type ? 1 : 0;
}

void input_map_create(const char* name, int* keys, size_t num_keys)
{
	assert(name && keys && num_keys > 0);

	struct Input_Map* new_map = array_grow(input_map_list, struct Input_Map);
	new_map->name  = name;
	new_map->keys  = array_new(int);
	new_map->state = KS_INACTIVE;
	for(size_t i = 0; i < num_keys; i++)
	{
		array_push(new_map->keys, keys[i], int);
	}
}

void input_update(void)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(map->state == GLFW_RELEASE)
			map->state = KS_INACTIVE;
	}
}

int input_map_remove(const char* name)
{
	assert(name);
	int success = 0;
    int index = map_find(name);
	if(index > -1)
	{
		array_remove_at(input_map_list, (int)index);
		success = 1;
	}	
	if(!success) log_error("input:map_remove", "Map %s not found", name);
	
	return success;
}

int input_map_keys_set(const char* name, int* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);
	int success = 0;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		array_reset(map->keys, num_keys);
		for(int i = 0; i < num_keys; i++)
			map->keys[i] = keys[i];
		success = 1;
	}
	if(!success)
		log_error("input:map_keys_set", "Map %s not found", name);	
	return success;
}

int input_map_name_set(const char* name, const char* new_name)
{
	assert(name && new_name);
	int success = 0;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		map->name = new_name;
		success = 1;
	}
	if(!success) log_error("input:map_name_set", "Map %s not found", name);
	return success;
}

static int map_find(const char* name)
{
	int index = -1;
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(strcmp(name, map->name) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}
