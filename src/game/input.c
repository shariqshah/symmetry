#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "../common/array.h"
#include "../common/log.h"
#include "gui.h"
#include "../common/string_utils.h"
#include "../common/hashmap.h"
#include "../common/variant.h"
#include "../common/parser.h"
#include "../system/platform.h"
#include "../system/file_io.h"
#include "event.h"
#include "game.h"

//static void input_on_key(int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
static void input_on_key(const struct Event* event);

static struct Hashmap* key_bindings = NULL;

void input_init(void)
{
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_subscribe(event_manager, EVT_KEY_PRESSED, &input_on_key);
	event_manager_subscribe(event_manager, EVT_KEY_RELEASED, &input_on_key);

	key_bindings = hashmap_create();

	/* Default keys for fallback */
	struct Key_Binding forward_keys           = {KEY_W,      KMOD_NONE, KEY_UP,     KMOD_NONE, KS_INACTIVE};
	struct Key_Binding backward_keys          = {KEY_S,      KMOD_NONE, KEY_DOWN,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding up_keys                = {KEY_Q,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding down_keys              = {KEY_E,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding left_keys              = {KEY_A,      KMOD_NONE, KEY_LEFT,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding right_keys             = {KEY_D,      KMOD_NONE, KEY_RIGHT,  KMOD_NONE, KS_INACTIVE};
	struct Key_Binding turn_right_keys        = {KEY_L,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding turn_left_keys         = {KEY_H,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding turn_up_keys           = {KEY_K,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding turn_down_keys         = {KEY_J,      KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding jump_keys              = {KEY_SPACE,  KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding sprint_keys            = {KEY_LSHIFT, KMOD_NONE, KEY_RSHIFT, KMOD_NONE, KS_INACTIVE};
	struct Key_Binding editor_toggle_keys     = {KEY_F1,     KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding pause_keys             = {KEY_ESCAPE, KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding console_toggle_keys    = {KEY_TILDE,  KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding debug_vars_toggle_keys = {KEY_F2,     KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding debug_vars_cycle_keys  = {KEY_F3,     KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding win_fullscreen_keys    = {KEY_F11,    KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	struct Key_Binding win_max_keys           = {KEY_F12,    KMOD_NONE, KEY_NONE,   KMOD_NONE, KS_INACTIVE};
	input_map_create("Move_Forward",      forward_keys);
	input_map_create("Move_Backward",     backward_keys);
	input_map_create("Move_Up",           up_keys);
	input_map_create("Move_Down",         down_keys);
	input_map_create("Move_Left",         left_keys);
	input_map_create("Move_Right",        right_keys);
	input_map_create("Turn_Right",        turn_right_keys);
	input_map_create("Turn_Left",         turn_left_keys);
	input_map_create("Turn_Up",           turn_up_keys);
	input_map_create("Turn_Down",         turn_down_keys);
	input_map_create("Sprint",            sprint_keys);
	input_map_create("Jump",              jump_keys);
	input_map_create("Pause",             pause_keys);
	input_map_create("Editor_Toggle",     editor_toggle_keys);
	input_map_create("Console_Toggle",    console_toggle_keys);
	input_map_create("Debug_Vars_Toggle", debug_vars_toggle_keys);
	input_map_create("Debug_Vars_Cycle",  debug_vars_cycle_keys);
	input_map_create("Window_Fullscreen", win_fullscreen_keys);
	input_map_create("Window_Maximize",   win_max_keys);

	if(!input_keybinds_load("keybindings.symtres", DIRT_USER))
	{
		log_error("input:init", "Failed to load keybindings");
		log_message("Reverting to default keybindings");
		if(!input_keybinds_load("keybindings.symtres", DIRT_INSTALL))
		{
			log_error("input:init", "Failed to load default keybindings");
			input_keybinds_save("keybindings.symtres", DIRT_INSTALL);
		}

		input_keybinds_save("keybindings.symtres", DIRT_USER);
	}
}

void input_cleanup(void)
{
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_KEY_PRESSED, &input_on_key);
	char* key = NULL;
	struct Variant* value = NULL;
	HASHMAP_FOREACH(key_bindings, key, value)
	{
		free(value->val_voidptr);
	}

	hashmap_free(key_bindings);
}

bool input_keybinds_load(const char* filename, int directory_type)
{
    FILE* key_file = io_file_open(directory_type, filename, "rb");
	if(!key_file)
	{
		log_error("input:keybinds_load", "Could not open %s", filename);
		return false;
	}

	struct Parser* parser = parser_load_objects(key_file, filename);
	if(!parser)
	{
		log_error("input:keybinds_load", "Failed to parse %s", filename);
		fclose(key_file);
		return false;
	}

	for(int i = 0; i < array_len(parser->objects); i++)
	{
		struct Parser_Object* object = &parser->objects[i];

		const char* name_temp = NULL;
		struct Key_Binding key_binding = 
		{
			.state          = KS_INACTIVE,
			.key_primary    = KEY_NONE,
			.mods_primary   = KMD_NONE,
			.key_secondary  = KEY_NONE,
			.mods_secondary = KMD_NONE
		};

		if(hashmap_value_exists(object->data, "name")) name_temp = hashmap_str_get(object->data, "name");
		if(hashmap_value_exists(object->data, "key_primary"))   
		{
			int key = platform_key_from_name(hashmap_str_get(object->data, "key_primary"));
			if(key != KEY_UNKNOWN)
			{
				key_binding.key_primary = key;
			}
		}

		if(hashmap_value_exists(object->data, "key_secondary")) 
		{
			int key = platform_key_from_name(hashmap_str_get(object->data, "key_secondary"));
			if(key != KEY_UNKNOWN)
			{
				key_binding.key_secondary = key;
			}
		}

		if(hashmap_value_exists(object->data, "mods_primary_ctrl"))
		{
			if(hashmap_bool_get(object->data, "mods_primary_ctrl"))
				key_binding.mods_primary |= KMD_CTRL;
		}

		if(hashmap_value_exists(object->data, "mods_primary_shift"))
		{
			if(hashmap_bool_get(object->data, "mods_primary_shift"))
				key_binding.mods_primary |= KMD_SHIFT;
		}

		if(hashmap_value_exists(object->data, "mods_primary_alt"))
		{
			if(hashmap_bool_get(object->data, "mods_primary_alt"))
				key_binding.mods_primary |= KMD_ALT;
		}

		if(hashmap_value_exists(object->data, "mods_secondary_ctrl"))
		{
			if(hashmap_bool_get(object->data, "mods_secondary_ctrl"))
				key_binding.mods_secondary |= KMD_CTRL;
		}

		if(hashmap_value_exists(object->data, "mods_secondary_shift"))
		{
			if(hashmap_bool_get(object->data, "mods_secondary_shift"))
				key_binding.mods_secondary = KMD_SHIFT;
		}

		if(hashmap_value_exists(object->data, "mods_secondary_alt"))
		{
			if(hashmap_bool_get(object->data, "mods_secondary_alt"))
				key_binding.mods_secondary = KMD_ALT;
		}

		input_map_create(name_temp, key_binding);
	}

	parser_free(parser);
	fclose(key_file);
	return true;
}

bool input_keybinds_save(const char* filename, int directory_type)
{
	struct Parser* parser = parser_new();
	if(!parser)
	{
		log_error("input:keybinds_save", "Could not create Parser");
		return false;
	}


	char* key = NULL;
	struct Variant* value = NULL;

	HASHMAP_FOREACH(key_bindings, key, value)
	{
		struct Key_Binding* key_binding = (struct Key_Binding*)value->val_voidptr;
		struct Parser_Object* object = parser_object_new(parser, PO_KEY);
		if(!object)
		{
			log_error("input:keybinds_save", "Failed to create Parser_object for Map '%s'", key);
			continue;
		}
		bool mods_primary_ctrl  = ((key_binding->mods_primary & KMD_CTRL)  == KMD_CTRL)  ? true : false;
		bool mods_primary_shift = ((key_binding->mods_primary & KMD_SHIFT) == KMD_SHIFT) ? true : false;
		bool mods_primary_alt   = ((key_binding->mods_primary & KMD_ALT)   == KMD_ALT)   ? true : false;

		bool mods_secondary_ctrl  = ((key_binding->mods_secondary & KMD_CTRL)  == KMD_CTRL)  ? true : false;
		bool mods_secondary_shift = ((key_binding->mods_secondary & KMD_SHIFT) == KMD_SHIFT) ? true : false;
		bool mods_secondary_alt   = ((key_binding->mods_secondary & KMD_ALT)   == KMD_ALT)   ? true : false;

		hashmap_str_set(object->data, "name", key);
		hashmap_str_set(object->data, "key_primary", key_binding->key_primary == KEY_NONE ? "NONE" : platform_key_name_get(key_binding->key_primary));
		hashmap_bool_set(object->data, "mods_primary_ctrl",  mods_primary_ctrl);
		hashmap_bool_set(object->data, "mods_primary_shift", mods_primary_shift);
		hashmap_bool_set(object->data, "mods_primary_alt",   mods_primary_alt);

		hashmap_str_set(object->data, "key_secondary", key_binding->key_secondary == KEY_NONE ? "NONE" : platform_key_name_get(key_binding->key_secondary));
		hashmap_bool_set(object->data, "mods_secondary_ctrl",  mods_secondary_ctrl);
		hashmap_bool_set(object->data, "mods_secondary_shift", mods_secondary_shift);
		hashmap_bool_set(object->data, "mods_secondary_alt",   mods_secondary_alt);
	}

	log_message("Keybindings saved to %s", filename);


	bool write_success = false;
	FILE* key_file = io_file_open(directory_type, filename, "w");
	if(!key_file)
	{
		log_error("input:keybinds_save", "Could not open %s", filename);
	}
	else
	{
		if(parser_write_objects(parser, key_file, filename))
		{
			log_message("Input Maps written to %s", filename);
			write_success = true;
		}
		else
		{
			log_error("Failed to write Input Maps to %s", filename);
		}
	}

	parser_free(parser);
	fclose(key_file);
	return write_success;
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

void input_on_key(const struct Event* event)
{
	assert(event->type == EVT_KEY_PRESSED || event->type == EVT_KEY_RELEASED);

	int  key       = event->key.key;
	int  state     = event->key.state;
	bool mod_ctrl  = event->key.mod_ctrl;
	bool mod_shift = event->key.mod_shift;;
	bool mod_alt   = event->key.mod_alt;;

	int mods = KMD_NONE;
	if(mod_ctrl)  mods |= KMD_CTRL;
	if(mod_shift) mods |= KMD_SHIFT;
	if(mod_alt)   mods |= KMD_ALT;

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	char* map_key = NULL;
	struct Variant* value = NULL;
	HASHMAP_FOREACH(key_bindings, map_key, value)
	{
		struct Key_Binding* key_binding = (struct Key_Binding*)value->val_voidptr;
		//Check with primary key
		if(key_binding->key_primary == key && (key_binding->mods_primary & mods) == key_binding->mods_primary)
		{
			key_binding->state = event->type == EVT_KEY_PRESSED ? KS_PRESSED : KS_RELEASED;
			struct Event* input_map_event = event_manager_create_new_event(event_manager);
			input_map_event->type = event->type == EVT_KEY_PRESSED ? EVT_INPUT_MAP_PRESSED : EVT_INPUT_MAP_RELEASED;
			strncpy(&input_map_event->input_map.name, map_key, MAX_HASH_KEY_LEN);
			event_manager_send_event(event_manager, input_map_event);
			break;
		}

		//If not, then check with secondary key
		if(key_binding->key_secondary == key && (key_binding->mods_secondary & mods) == key_binding->mods_secondary)
		{
			key_binding->state = event->type == EVT_KEY_PRESSED ? KS_PRESSED : KS_RELEASED;
			struct Event* input_map_event = event_manager_create_new_event(event_manager);
			input_map_event->type = event->type == EVT_KEY_PRESSED ? EVT_INPUT_MAP_PRESSED : EVT_INPUT_MAP_RELEASED;
			strncpy(&input_map_event->input_map.name, map_key, MAX_HASH_KEY_LEN);
			event_manager_send_event(event_manager, input_map_event);
			break;
		}
	}
}

void input_mouse_mode_set(enum Mouse_Mode mode)
{
    platform_mouse_relative_mode_set(mode == MM_NORMAL ? 0 : 1);
}

bool input_map_state_get(const char* name, int state)
{
	struct Key_Binding* key_binding = (struct Key_Binding*)hashmap_ptr_get(key_bindings, name);
	if(!key_binding)
	{
		log_error("input:map_state_get", "Map '%s' not found", name);
		return false;
	}

	return (key_binding->state == state);
}

bool input_map_create(const char* name, struct Key_Binding key_combination)
{
	//Check if a key map already exists
	if(hashmap_value_exists(key_bindings, name))
	{
		log_warning("Removing existing Map '%s' and replacing with new one", name);
		input_map_remove(name);
	}

	struct Key_Binding* new_keybinding = malloc(sizeof(*new_keybinding));
	if(!new_keybinding)
	{
		log_error("input:map_create", "Out of memory");
		return false;
	}

	memcpy(new_keybinding, &key_combination, sizeof(key_combination));
	hashmap_ptr_set(key_bindings, name, new_keybinding);
	log_message("Created new input map '%s'", name);
	return true;
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

void input_post_update(void)
{
	struct Variant* value = NULL;
	char* key = NULL;
	HASHMAP_FOREACH(key_bindings, key, value)
	{
		struct Key_Binding* key_binding = (struct Key_Binding*)value->val_voidptr;
		if(key_binding->state == KS_RELEASED)
			key_binding->state = KS_INACTIVE;
	}
}

bool input_map_remove(const char* name)
{
	assert(name);
	
	if(hashmap_value_exists(key_bindings, name))
	{
		struct Key_Binding* current_key = (struct Key_Binding*)hashmap_ptr_get(key_bindings, name);
		free(current_key);
		hashmap_value_remove(key_bindings, name);
	}
	else
	{
		log_error("input:map_remove", "Map '%s' not found", name);
		return false;
	}

	return true;
}

bool input_map_keys_set(const char* name, struct Key_Binding key_combination)
{
	assert(name);
	struct Key_Binding* existing_combination = (struct Key_Binding*)hashmap_ptr_get(key_bindings, name);
	if(!existing_combination)
	{
		log_error("input:map_keys_set", "Map '%s' not found", name);
		return false;
	}

	memcpy(existing_combination, &key_combination, sizeof(key_combination));
	existing_combination->state = KS_INACTIVE;
	return true;
}

bool input_map_name_set(const char* name, const char* new_name)
{
	//assert(name && new_name);
	//bool success = false;
	//int index = map_find(name);
	//if(index > -1)
	//{
	//	struct Key_Binding* map = &key_bindings[index];
	//	map->name = str_new(new_name);
	//	success = true;
	//}
	//if(!success) log_error("input:map_name_set", "Map %s not found", name);
	//return success;
	return false;
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

