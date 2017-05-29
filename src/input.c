#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "array.h"
#include "platform.h"
#include "log.h"
#include "gui.h"
#include "file_io.h"
#include "string_utils.h"

struct Input_Map
{
	char*                   name;
	struct Key_Combination* keys;
	int                     state;
};

static void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
static void input_on_mousebutton(int button, int state, int x, int y, int8 num_clicks);
static void input_on_mousemotion(int x, int y, int xrel, int yrel);
static void input_on_mousewheel(int x, int y);
static int  map_find(const char* name);

static struct Input_Map* input_map_list = NULL;

void input_init(void)
{
	platform_keyboard_callback_set(&input_on_key);
	platform_mousebutton_callback_set(&input_on_mousebutton);
	platform_mousemotion_callback_set(&input_on_mousemotion);
	platform_mousewheel_callback_set(&input_on_mousewheel);
	
	input_map_list = array_new(struct Input_Map);
	if(!input_keybinds_load("keybindings.cfg", DT_USER))
	{
		log_error("input:init", "Failed to load keybindings");
		log_message("Reverting to default keybindings");
		if(!input_keybinds_load("keybindings.cfg", DT_INSTALL))
		{
			log_error("input:init", "Failed to load default keybindings");
		}
		else
		{
			input_keybinds_save("keybindings.cfg");
		}
	}

	/* struct Key_Combination forward_keys[2]      = {{KEY_W, KMD_NONE}, {KEY_UP, KMD_ALT | KMD_SHIFT}}; */
	/* struct Key_Combination backward_keys[2]     = {{KEY_S, KMD_NONE}, {KEY_DOWN, KMD_NONE}}; */
	/* struct Key_Combination up_keys[2]           = {KEY_Q}; */
	/* struct Key_Combination down_keys[2]         = {KEY_E}; */
	/* struct Key_Combination left_keys[2]         = {KEY_A, KEY_LEFT}; */
	/* struct Key_Combination right_keys[2]        = {KEY_D, KEY_RIGHT}; */
	/* struct Key_Combination turn_right_keys[1]   = {KEY_L}; */
	/* struct Key_Combination turn_left_keys[1]    = {KEY_J}; */
	/* struct Key_Combination turn_up_keys[1]      = {KEY_I}; */
	/* struct Key_Combination turn_down_keys[1]    = {KEY_K}; */
	/* struct Key_Combination sprint_keys[2]       = {KEY_LSHIFT, KEY_RSHIFT}; */
	/* struct Key_Combination recompute_keys[2]    = {KEY_F5, KEY_H}; */
	/* struct Key_Combination ed_toggle_keys[1]    = {KEY_F1}; */
	/* struct Key_Combination win_fullscr_keys[1]  = {KEY_F11}; */
	/* struct Key_Combination win_max_keys[1]      = {KEY_F12}; */
	/* input_map_create("Move_Forward",      forward_keys,     2); */
	/* input_map_create("Move_Backward",     backward_keys,    2); */
	/* input_map_create("Move_Up",           up_keys,          1); */
	/* input_map_create("Move_Down",         down_keys,        1); */
	/* input_map_create("Move_Left",         left_keys,        2); */
	/* input_map_create("Move_Right",        right_keys,       2); */
	/* input_map_create("Turn_Right",        turn_right_keys,  1); */
	/* input_map_create("Turn_Left",         turn_left_keys,   1); */
	/* input_map_create("Turn_Up",           turn_up_keys,     1); */
	/* input_map_create("Turn_Down",         turn_down_keys,   1); */
	/* input_map_create("Sprint",            sprint_keys,      2); */
	/* input_map_create("Recompute",         recompute_keys,   2); */
	/* input_map_create("Editor_Toggle",     ed_toggle_keys,   1); */
	/* input_map_create("Window_Fullscreen", win_fullscr_keys, 1); */
	/* input_map_create("Window_Maximize",   win_max_keys,     1); */
}

void input_cleanup(void)
{
	//input_keybinds_save("keybindings.cfg");
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		//log_message("Map : %s, Num keys : %d", map->name, array_len(map->keys));
		if(map->name) free(map->name);
		array_free(map->keys);
	}
	array_free(input_map_list);
}

bool input_keybinds_load(const char* filename, int directory_type)
{
	bool success = false;
	const int MAX_KEYBIND_LEN = 128;
	const int MAX_LINE_LEN    = 512;
	FILE* config_file = io_file_open(directory_type, filename, "r");
	if(!config_file)
	{
		log_error("input:keybinds_load", "Could not open %s", filename);
		return success;
	}

	/* Read line by line, ignore comments */
	char key_str[MAX_KEYBIND_LEN];
	char line_buffer[MAX_LINE_LEN];
	memset(key_str, '\0', MAX_KEYBIND_LEN);
	memset(line_buffer, '\0', MAX_LINE_LEN);
	int current_line = 0;
	while(fgets(line_buffer, MAX_LINE_LEN - 1, config_file))
	{
		current_line++;
		line_buffer[strcspn(line_buffer, "\r\n")] = '\0';
		
		if(line_buffer[0] == '#' || strlen(line_buffer) == 0)
			continue;

		//log_message("Line : %s", line_buffer);
		memset(key_str, '\0', MAX_KEYBIND_LEN);
		char* value_str = strstr(line_buffer, ":");
		if(!value_str)
		{
			log_warning("Malformed value in config file %s, line %d", filename, current_line);
			continue;
		}
		
		value_str++; /* Ignore the colon(:) and set the pointer after it */
		
		if(sscanf(line_buffer, " %1024[^: ] : %*s", key_str) != 1)
		{
			log_warning("Unable to read key in keybindings file %s, line %d", filename, current_line);
			continue;
		}

		char* val = strtok(value_str, ",");
		if(!val)
		{
			log_warning("Unable to parse keys for keybinding %s in file %s, line %d", key_str, filename, current_line);
			continue;
		}

		while(val)
		{
			//log_message("Key read : %s", val);

			/* Check if there are any Modifiers */
			int       modifiers        = KMD_NONE;
			char*     keys             = strstr(val, "-");
			char*     start_loc        = val;
			const int max_key_str_len  = 20;
			char      key_name[max_key_str_len];
			int       skip_to_next     = 0;
			while(keys)
			{
				memset(key_name, '\0', max_key_str_len);
				strncpy(key_name, start_loc, (keys - start_loc));
				//log_message("key_name : %s", key_name);

				int key_modifier = platform_key_from_name(key_name);

				if(key_modifier == KEY_UNKNOWN)
				{
					log_warning("Unrecognized key %s in keybindings file %s, at line %d", key_name, filename, current_line);
					skip_to_next = 1;
					break;
				}

				switch(key_modifier)
				{
				case KEY_LSHIFT: case KEY_RSHIFT: modifiers |= KMD_SHIFT; break;
				case KEY_LCTRL:  case KEY_RCTRL:  modifiers |= KMD_CTRL;  break;
				case KEY_LALT:   case KEY_RALT:   modifiers |= KMD_ALT;   break;
				};
				
				++keys;
				start_loc = keys;
				keys = strstr(keys, "-");
			}
			if(skip_to_next)
			{
				val = strtok(NULL, ",");
				continue;
			}
			
			/* Copy the last key after the hyphen */
			strncpy(key_name, start_loc, max_key_str_len);
			int key = platform_key_from_name(key_name);
			if(key == KEY_UNKNOWN)
			{
				log_warning("Unrecognized key %s in keybindings file %s, at line %d", key_name, filename, current_line);
				val = strtok(NULL, ",");
				continue;
			}

			struct Key_Combination key_comb = { .key = key, .mods = modifiers};
			input_map_create(key_str, &key_comb, 1);
			
			val = strtok(NULL, ",");
		}
	}
	
	success = true;
	fclose(config_file);
	return success;
}

bool input_keybinds_save(const char* filename)
{
	bool success = false;

	FILE* config_file = io_file_open(DT_USER, filename, "w");
	if(!config_file)
	{
		log_error("input:keybinds_save", "Could not open %s", filename);
		return success;
	}

	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		fprintf(config_file, "%s : ", map->name);
		for(int j = 0; j < array_len(map->keys); j++)
		{
			if(j != 0) fprintf(config_file, ", ");
			struct Key_Combination* key_comb = &map->keys[j];
			if((key_comb->mods & KMD_ALT)   == KMD_ALT)   fprintf(config_file, "%s-", platform_key_name_get(KEY_LALT));
			if((key_comb->mods & KMD_SHIFT) == KMD_SHIFT) fprintf(config_file, "%s-", platform_key_name_get(KEY_LSHIFT));
			if((key_comb->mods & KMD_CTRL)  == KMD_CTRL)  fprintf(config_file, "%s-", platform_key_name_get(KEY_LCTRL));
			fprintf(config_file, "%s", platform_key_name_get(key_comb->key));
		}
		fprintf(config_file, "\n");
	}

	fclose(config_file);
	log_message("Keybindings saved to %s", filename);
	success = true;
	return success;
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

bool input_map_state_get(const char* map_name, int state)
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

	bool result = false;
	if(current_state == state)
	{
		result = true;
	}
	return result;
}

bool input_is_key_pressed(int key)
{
	return platform_is_key_pressed(key);
}

bool input_mousebutton_state_get(uint button, int state_type)
{
	int current_state = platform_mousebutton_state_get(button);
	return state_type == current_state ? true : false;
}

void input_map_create(const char* name, struct Key_Combination* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);	
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		for(int i = 0; i < num_keys; i++)
		{
			struct Key_Combination* new_comb = array_grow(map->keys, struct Key_Combination);
			*new_comb = keys[i];
			log_message("Added new Key combination to input map : %s", name);
		}
	}
	else
	{
		struct Input_Map* new_map = array_grow(input_map_list, struct Input_Map);
		new_map->name  = str_new(name);
		new_map->keys  = array_new_cap(struct Key_Combination, num_keys);
		new_map->state = KS_INACTIVE;
		for(int i = 0; i < num_keys; i++)
			new_map->keys[i] = keys[i];
		log_message("Created Input Map : %s", name);
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

bool input_map_remove(const char* name)
{
	assert(name);
	bool success = false;
    int index = map_find(name);
	if(index > -1)
	{
		array_remove_at(input_map_list, (int)index);
		success = true;
	}	
	if(!success) log_error("input:map_remove", "Map %s not found", name);
	
	return success;
}

bool input_map_keys_set(const char* name, struct Key_Combination* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);
	bool success = false;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		if(array_len(map->keys) != num_keys)
			array_reset(map->keys, num_keys);
		for(int i = 0; i < num_keys; i++)
			map->keys[i] = keys[i];
		
		map->state = KS_INACTIVE;
		success    = true;
	}
	if(!success)
		log_error("input:map_keys_set", "Map %s not found", name);	
	return success;
}

bool input_map_name_set(const char* name, const char* new_name)
{
	assert(name && new_name);
	bool success = false;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		map->name = str_new(new_name);
		success = true;
	}
	if(!success) log_error("input:map_name_set", "Map %s not found", name);
	return success;
}

int map_find(const char* name)
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

