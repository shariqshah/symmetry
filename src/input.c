#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "array.h"
#include "platform.h"
#include "log.h"
#include "gui.h"

/* #define KS_INACTIVE -1; 			/\* state for input map is set to KS_INACTIVE(KeyState_Inactive) when */
/* 									   the key is neither pressed nor released *\/ */

struct Input_Map
{
	const char* name;
	int* keys;
	int  state;
};

static void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift);
static void input_on_mousebutton(int button, int state, int x, int y, int8 num_clicks);
static void input_on_mousemotion(int x, int y, int xrel, int yrel);
static void input_on_mousewheel(int x, int y);
static int  map_find(const char* name);

static struct Input_Map* input_map_list;

void input_init(void)
{
	platform_keyboard_callback_set(&input_on_key);
	platform_mousebutton_callback_set(&input_on_mousebutton);
	platform_mousemotion_callback_set(&input_on_mousemotion);
	platform_mousewheel_callback_set(&input_on_mousewheel);
	
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

void input_on_mousemotion(int x, int y, int xrel, int yrel)
{
	/* TODO: This is temporary. After proper event loop is added this code should not be here */
	gui_handle_mousemotion_event(x, y, xrel, yrel);
}

void input_on_mousewheel(int x, int y)
{
	/* TODO: This is temporary. After proper event loop is added this code should not be here */
	gui_handle_mousewheel_event(x, y);
}

void input_mouse_pos_get(int* xpos, int* ypos)
{
	assert(xpos && ypos);
	platform_mouse_position_get(xpos, ypos);
}

void input_mouse_pos_set(int xpos, int ypos)
{
	platform_mouse_global_position_set(xpos, ypos);
}

void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift)
{
	if(repeat)
	{
		log_message("Repeat ignored");
		return;			/* Ignore key repeat */
	}
	
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		for(int j = 0; j < array_len(map->keys); j++)
		{
			if(map->keys[j] == key)
			{
				map->state = state;
				break;
			}
		}
	}
	/* TODO: This is temporary. After proper event loop is added this code should not be here */
	gui_handle_keyboard_event(key, state, mod_ctrl, mod_shift);
}

void input_on_mousebutton(int button, int state, int x, int y, int8 num_clicks)
{
	/* Probably add 'mouse maps', same as input maps for keyvboard but with buttons
	   Do we even need that?
	*/
	/* TODO: This is temporary. After proper event loop is added this code should not be here */
	gui_handle_mousebutton_event(button, state, x, y);
}

void input_mouse_mode_set(enum Mouse_Mode mode)
{
	platform_mouse_relative_mode_set(mode == MM_NORMAL ? 0 : 1);
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

	int result = 0;
	if(current_state == state)
	{
		result = 1;
	}
	return result;
}

int input_is_key_pressed(int key)
{
	return platform_is_key_pressed(key);
}

int input_mousebutton_state_get(uint button, int state_type)
{
	int current_state = platform_mousebutton_state_get(button);
	return state_type == current_state ? 1 : 0;
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
		if(map->state == KS_RELEASED)
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

int input_mouse_mode_get(void)
{
	int mouse_mode = MM_NORMAL;
	if(platform_mouse_relative_mode_get()) mouse_mode = MM_RELATIVE;
	return mouse_mode;
}

	
void input_mouse_delta_get(int* xpos, int* ypos)
{
	platform_mouse_delta_get(xpos, ypos);
}

