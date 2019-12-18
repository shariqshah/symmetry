#include "console.h"
#include "gui.h"
#include "game.h"
#include "../common/log.h"
#include "../system/platform.h"
#include "scene.h"
#include "entity.h"
#include "debug_vars.h"
#include "../system/file_io.h"

#include <assert.h>
#include <string.h>
#include <nuklear.h>

static struct nk_color console_message_color[CMT_MAX];

static int console_filter(const struct nk_text_edit *box, nk_rune unicode);

static void console_command_scene_empty(struct Console* console, const char* command);
static void console_command_scene_save(struct Console* console, const char* command);
static void console_command_scene_load(struct Console* console, const char* command);
static void console_command_entity_save(struct Console* console, const char* command);
static void console_command_entity_load(struct Console* console, const char* command);
static void console_command_debug_vars_toggle(struct Console* console, const char* command);
static void console_command_debug_vars_location_set(struct Console* console, const char* command);
static void console_command_help(struct Console* console, const char* command);

void console_init(struct Console* console)
{
    assert(console);

    console_message_color[CMT_MESSAGE] = nk_rgb(255, 255, 255);
    console_message_color[CMT_WARNING] = nk_rgb(255, 255, 0);
    console_message_color[CMT_ERROR]   = nk_rgb(255, 0, 0);
    console_message_color[CMT_COMMAND] = nk_rgb(114, 173, 224);
    console_message_color[CMT_NONE]    = nk_rgb(255, 0, 255);
	
    console->visible               = false;
    console->scroll_to_bottom      = true;
    console->text_region_height    = 22.f;
    console->line_height           = 20.f;
    console->current_message_index = -1;

    memset(console->console_command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
	for(int i = 0; i < MAX_CONSOLE_MESSAGES; i++)
	{
		memset(console->console_messages[i].message, '\0', MAX_CONSOLE_MESSAGE_LEN);
		console->console_messages[i].type = CMT_NONE;
	}

	console->console_commands = hashmap_new();
	hashmap_ptr_set(console->console_commands, "scene_empty", &console_command_scene_empty);
	hashmap_ptr_set(console->console_commands, "scene_save", &console_command_scene_save);
	hashmap_ptr_set(console->console_commands, "scene_load", &console_command_scene_load);
	hashmap_ptr_set(console->console_commands, "entity_save", &console_command_entity_save);
	hashmap_ptr_set(console->console_commands, "entity_load", &console_command_entity_load);
	hashmap_ptr_set(console->console_commands, "debug_vars_toggle", &console_command_debug_vars_toggle);
	hashmap_ptr_set(console->console_commands, "debug_vars_location", &console_command_debug_vars_location_set);
	hashmap_ptr_set(console->console_commands, "help", &console_command_help);
}

void console_toggle(struct Console* console)
{
    console->visible = !console->visible;
    if(console->visible) console->scroll_to_bottom = true;
}

void console_update(struct Console* console, struct Gui* gui_state, float dt)
{
    if(!console->visible) return;

    struct nk_context* context = &gui_state->context;
    struct Game_State* game_state = game_state_get();

    int win_width = 0, win_height = 0;
    window_get_drawable_size(game_state->window, &win_width, &win_height);
    int half_height = win_height / 2;

    if(nk_begin_titled(context, "Console", "Console", nk_recti(0, 0, win_width, half_height), NK_WINDOW_SCROLL_AUTO_HIDE))
    {
		nk_layout_row_dynamic(context, nk_window_get_height(context) - console->text_region_height * 2, 1);
		if(nk_group_begin(context, "Log", NK_WINDOW_BORDER))
		{
			for(int i = 0; i <= console->current_message_index; i++)
			{
				nk_layout_row_dynamic(context, console->line_height, 1);
				nk_labelf_colored(context, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, console_message_color[console->console_messages[i].type], console->console_messages[i].message);
			}

			if(console->scroll_to_bottom == true) // scroll console message area to the bottom if required
			{
				*context->current->layout->offset_y = (nk_uint)context->current->layout->at_y;
				console->scroll_to_bottom = false;
			}	
			nk_group_end(context);
		}

		//Edit-string/Textfield for command
		nk_layout_row_dynamic(context, console->text_region_height, 1);
		int edit_flags = NK_EDIT_GOTO_END_ON_ACTIVATE | NK_EDIT_FIELD | NK_EDIT_SIG_ENTER;
		nk_edit_focus(context, edit_flags);
		int edit_state = nk_edit_string_zero_terminated(context, edit_flags, console->console_command_text, MAX_CONSOLE_MESSAGE_LEN, console_filter);
		if(edit_state & NK_EDIT_COMMITED)
		{
			if(++console->current_message_index >= MAX_CONSOLE_MESSAGES)
				console->current_message_index = 0;
			
			snprintf(console->console_messages[console->current_message_index].message, MAX_CONSOLE_MESSAGE_LEN, "> %s", console->console_command_text);
			console->console_messages[console->current_message_index].type = CMT_COMMAND;
			console->scroll_to_bottom = true;

			/* Check if a valid command is entered and call the related function or print an error message */
			static char command_text[MAX_CONSOLE_MESSAGE_LEN];
			static char command_params[MAX_CONSOLE_MESSAGE_LEN];
			memset(command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
			memset(command_params, '\0', MAX_CONSOLE_MESSAGE_LEN);
			sscanf(console->console_command_text, "%s %[^\n]", command_text, command_params);
			if(hashmap_value_exists(console->console_commands, command_text))
			{
				Console_Command_Handler command_handler = hashmap_ptr_get(console->console_commands, command_text);
				command_handler(console, command_params);
			}
			else
			{
				log_warning("Invalid command '%s'", command_text);
			}
			memset(console->console_command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
		}
    }
    nk_end(context);
}

void console_destroy(struct Console* console)
{
	hashmap_free(console->console_commands);
}

int console_filter(const struct nk_text_edit *box, nk_rune unicode)
{
    NK_UNUSED(box);
	if(unicode > 128 || unicode == 96) // Ignore tilde or anything other than ascii
		return nk_false;
	else
		return nk_true;
}

void console_on_log_message(struct Console* console, const char* message, va_list args)
{
	if(++console->current_message_index >= MAX_CONSOLE_MESSAGES)
		console->current_message_index = 0;
	vsnprintf(console->console_messages[console->current_message_index].message, MAX_CONSOLE_MESSAGE_LEN, message, args);
	console->console_messages[console->current_message_index].type = CMT_MESSAGE;
	console->scroll_to_bottom = true;
}

void console_on_log_warning(struct Console* console, const char* warning_message, va_list args)
{
	if(++console->current_message_index >= MAX_CONSOLE_MESSAGES)
		console->current_message_index = 0;
	vsnprintf(console->console_messages[console->current_message_index].message, MAX_CONSOLE_MESSAGE_LEN, warning_message, args);
	console->console_messages[console->current_message_index].type = CMT_WARNING;
	console->scroll_to_bottom = true;
}

void console_on_log_error(struct Console* console, const char* context, const char* error, va_list args)
{
	if(++console->current_message_index >= MAX_CONSOLE_MESSAGES)
		console->current_message_index = 0;
	int loc = snprintf(console->console_messages[console->current_message_index].message, MAX_CONSOLE_MESSAGE_LEN, "(%s)", context);
	vsnprintf(console->console_messages[console->current_message_index].message + loc, MAX_CONSOLE_MESSAGE_LEN - loc, error, args);
	console->console_messages[console->current_message_index].type = CMT_ERROR;
	console->scroll_to_bottom = true;
}

void console_command_entity_save(struct Console* console, const char* command)
{
	char filename[MAX_FILENAME_LEN];
	char entity_name[MAX_ENTITY_NAME_LEN];
	memset(filename, '\0', MAX_FILENAME_LEN);
	memset(entity_name, '\0', MAX_ENTITY_NAME_LEN);

	int params_read = sscanf(command, "%s %s", entity_name, filename);
	if(params_read != 2)
	{
		log_warning("Invalid parameters for command");
		log_warning("Usage: entity_save [entity name] [file name]");
		return;
	}

	struct Entity* entity = scene_find(game_state_get()->scene, entity_name);
	if(!entity)
	{
		log_error("entity_save", "No entity named '%s' in current scene", entity_name);
		return;
	}

	char full_filename[MAX_FILENAME_LEN];
	snprintf(full_filename, MAX_FILENAME_LEN, "entities/%s.symtres", filename);
	if(!entity_save(entity, full_filename, DIRT_INSTALL))
		log_error("entity_save", "Command failed");
}

void console_command_entity_load(struct Console* console, const char* command)
{
	char filename[MAX_FILENAME_LEN];
	char new_entity_name[MAX_ENTITY_NAME_LEN];
	memset(filename, '\0', MAX_FILENAME_LEN);
	memset(new_entity_name, '\0', MAX_ENTITY_NAME_LEN);

	int params_read = sscanf(command, "%s %s", filename, new_entity_name);
	if(params_read < 1 && params_read > 2)
	{
		log_warning("Invalid parameters for command");
		log_warning("Usage: entity_load [file name] [optional: new entity name]");
		return;
	}

	char full_filename[MAX_FILENAME_LEN];
	snprintf(full_filename, MAX_FILENAME_LEN, "entities/%s.symtres", filename);
	struct Entity* new_entity = entity_load(full_filename, DIRT_INSTALL);
	if(!new_entity)
	{
		log_error("entity_load", "Could not create entity from '%s'", full_filename);
		return;
	}

	if(params_read == 2) entity_rename(new_entity, new_entity_name);
}

void console_command_help(struct Console* console, const char* command)
{
	char* key = NULL;
	Console_Command_Handler value;
	log_message("======================================");
	log_message("Available Commands");
	log_message("======================================");
	HASHMAP_FOREACH(console->console_commands, key, value)
	{
		log_message("%s", key);
	}
	log_message("======================================");
}

void console_command_scene_save(struct Console* console, const char* command)
{
	char filename[MAX_FILENAME_LEN];
	memset(filename, '\0', MAX_FILENAME_LEN);

	int params_read = sscanf(command, "%s", filename);
	if(params_read != 1)
	{
		log_warning("Invalid parameters for command");
		log_warning("Usage: scene_save [file name]");
		return;
	}

	char full_filename[MAX_FILENAME_LEN];
	snprintf(full_filename, MAX_FILENAME_LEN, "scenes/%s.symtres", filename);
	if(!scene_save(game_state_get()->scene, full_filename, DIRT_INSTALL))
		log_error("scene_save", "Command failed");
}

void console_command_scene_load(struct Console* console, const char* command)
{
	char filename[MAX_FILENAME_LEN];
	memset(filename, '\0', MAX_FILENAME_LEN);

	int params_read = sscanf(command, "%s", filename);
	if(params_read != 1)
	{
		log_warning("Invalid parameters for command");
		log_warning("Usage: scene_load [file name]");
		return;
	}

	char full_filename[MAX_FILENAME_LEN];
	snprintf(full_filename, MAX_FILENAME_LEN, "scenes/%s.symtres", filename);
	if(!scene_load(game_state_get()->scene, full_filename, DIRT_INSTALL))
		log_error("scene_load", "Command failed");
}

void console_command_scene_empty(struct Console* console, const char* command)
{
	char filename[MAX_FILENAME_LEN];
	memset(filename, '\0', MAX_FILENAME_LEN);

	struct Scene* scene = game_state_get()->scene;
	scene_destroy(scene);
	scene_post_update(scene);
	scene_init(scene);
}

void console_command_debug_vars_toggle(struct Console* console, const char* command)
{
	struct Debug_Vars* debug_vars = game_state_get()->debug_vars;
	debug_vars->visible = !debug_vars->visible;
}

void console_command_debug_vars_location_set(struct Console* console, const char* command)
{
	struct Debug_Vars* debug_vars = game_state_get()->debug_vars;
	int location = debug_vars->location;
	int params_read = sscanf(command, "%d", &location);
	if(params_read != 1)
	{
		log_warning("Invalid parameters for command");
		log_warning("Usage: debug_vars_location [Location 0-4]");
		return;
	}

	debug_vars_location_set(debug_vars, location);
}
