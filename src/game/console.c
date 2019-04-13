#include "console.h"
#include "gui.h"
#include "game.h"
#include "../common/log.h"
#include "../system/platform.h"

#include <assert.h>
#include <string.h>
#include <nuklear.h>

static struct nk_color console_message_color[CMT_MAX];

static int console_filter(const struct nk_text_edit *box, nk_rune unicode);

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
}

void console_toggle(struct Console* console)
{
    console->visible = !console->visible;
    if(console->visible) console->scroll_to_bottom = true;
}

void console_update(struct Console* console, struct Gui_State* gui_state, float dt)
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
			memset(console->console_command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
			console->scroll_to_bottom = true;
		}
    }
    nk_end(context);
}

void console_destroy(struct Console* console)
{
	
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
