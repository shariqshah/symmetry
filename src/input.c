#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "array.h"
#include "platform.h"
#include "log.h"
#include "gui.h"
#include "file_io.h"

struct Input_Map
{
	const char*             name;
	struct Key_Combination* keys;
	int                     state;
};

static void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
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

int input_load(const char* filename)
{
	int success = 0;
	/* const int MAX_KEYBIND_LEN = 128; */
	/* const int MAX_LINE_LEN    = 512; */
	/* FILE* config_file = io_file_open(filename, "r"); */
	/* if(!config_file) */
	/* { */
	/* 	log_error("input:vars_load", "Could not open %s", filename); */
	/* 	return success; */
	/* } */

	/* /\* Read line by line, ignore comments *\/ */
	/* char key_str[MAX_KEYBIND_LEN]; */
	/* char line_buffer[MAX_LINE_LEN]; */
	/* memset(key_str, '\0', MAX_KEYBIND_LEN); */
	/* memset(line_buffer, '\0', MAX_LINE_LEN); */
	/* int current_line = 0; */
	/* while(fgets(line_buffer, MAX_LINE_LEN - 1, config_file)) */
	/* { */
	/* 	current_line++; */
	/* 	line_buffer[strcspn(line_buffer, "\r\n")] = '\0'; */
		
	/* 	if(line_buffer[0] == '#' || strlen(line_buffer) == 0) */
	/* 		continue; */

	/* 	log_message("Line : %s", line_buffer); */
	/* 	memset(key_str, '\0', MAX_KEYBIND_LEN); */
	/* 	char* value_str = strstr(line_buffer, ":"); */
	/* 	if(!value_str) */
	/* 	{ */
	/* 		log_warning("Malformed value in config file %s, line %d", filename, current_line); */
	/* 		continue; */
	/* 	} */
	/* 	int key_str_len = value_str - line_buffer; */
	/* 	strncpy(key_str, line_buffer, key_str_len); */
	/* 	if(key_str_len >= MAX_KEYBIND_LEN) */
	/* 		key_str[MAX_KEYBIND_LEN - 1] = '\0'; */
	/* 	else */
	/* 		key_str[key_str_len] = '\0'; */
	/* 	value_str++; /\* Ignore the colon(:) *\/ */
		
	/* 	struct Variant* value = hashmap_value_get(cvars, key_str); */
	/* 	if(!value) */
	/* 	{ */
	/* 		log_warning("Unknown value in config file %s, line %d", filename, current_line); */
	/* 		continue; */
	/* 	} */
	/* 	variant_from_str(value, value_str, value->type); */
	/* } */
	
	/* success = 1; */
	/* fclose(config_file); */
	return success;
}

int input_save(const char* filename)
{
	
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

void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt)
{
	/* if(repeat) */
	/* { */
	/* 	return;			/\* Ignore key repeat *\/ */
	/* } */

	int mods = KMD_NONE;
	if(mod_ctrl)  mods |= KMD_CTRL;
	if(mod_shift) mods |= KMD_SHIFT;
	if(mod_alt)   mods |= KMD_ALT;
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		for(int j = 0; j < array_len(map->keys); j++)
		{
			if(map->state == KS_PRESSED &&
			   state == KS_RELEASED     &&
			   ((map->keys[j].mods & mods) == map->keys[j].mods))
			{
				map->state = state;
			}
			if(map->keys[j].key == key && ((map->keys[j].mods & mods) == map->keys[j].mods))
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
	/* Probably add 'mouse maps', same as input maps for keyboard but with buttons
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

void input_map_create(const char* name, struct Key_Combination* keys, size_t num_keys)
{
	assert(name && keys && num_keys > 0);

	struct Input_Map* new_map = array_grow(input_map_list, struct Input_Map);
	new_map->name  = name;
	new_map->keys  = array_new_cap(struct Key_Combination, num_keys);
	new_map->state = KS_INACTIVE;
	for(size_t i = 0; i < num_keys; i++)
		new_map->keys[i] = keys[i];
	log_message("Created Input Map : %s", name);
	/* { */
	/* 	new_map->keys[i].key  = keys[i].key; */
	/* 	new_map->keys[i].mods = keys[i].mods; */
	/* } */
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

int input_map_keys_set(const char* name, struct Key_Combination* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);
	int success = 0;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		if(array_len(map->keys) != num_keys)
			array_reset(map->keys, num_keys);
		for(int i = 0; i < num_keys; i++)
			map->keys[i] = keys[i];
		
		map->state = KS_INACTIVE;
		success    = 1;
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

