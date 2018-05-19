#include "console.h"
#include "gui.h"
#include "game.h"
#include "../common/log.h"
#include "../common/common.h"

#include <assert.h>
#include <string.h>
#include <nuklear.h>

static struct nk_color color_normal;

static int console_filter(const struct nk_text_edit *box, nk_rune unicode);

void console_init(struct Console* console)
{
	assert(console);

	color_normal = nk_rgb(255, 255, 255);
	
	console->visible               = false;
	console->text_region_height    = 22.f;
	console->line_height           = 20.f;
	console->current_message_index = -1;

	memset(console->console_command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
	for(int i = 0; i < MAX_CONSOLE_MESSAGES; i++)
		memset(console->console_messages[i], '\0', MAX_CONSOLE_MESSAGE_LEN);
}

void console_toggle(struct Console* console)
{
	console->visible = !console->visible;
}

void console_update(struct Console* console, struct Gui_State* gui_state, float dt)
{
	if(!console->visible) return;

	struct nk_context* context = &gui_state->context;
	struct Game_State* game_state = game_state_get();

	int win_width = 0, win_height = 0;
	platform->window.get_drawable_size(game_state->window, &win_width, &win_height);
	int half_height = win_height / 2;

	if(nk_begin_titled(context, "Console", "Console", nk_recti(0, 0, win_width, half_height), NK_WINDOW_SCROLL_AUTO_HIDE))
	{
		nk_layout_row_dynamic(context, nk_window_get_height(context) - console->text_region_height * 2, 1);
		if(nk_group_begin(context, "Log", NK_WINDOW_SCROLL_AUTO_HIDE))
		{
			for(int i = 0; i <= console->current_message_index; i++)
			{
				nk_layout_row_dynamic(context, console->line_height, 1);
				nk_labelf_colored(context, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, color_normal, console->console_messages[i]);
			}
			nk_group_end(context);
		}

		nk_layout_row_dynamic(context, console->text_region_height, 1);
		int edit_flags = NK_EDIT_GOTO_END_ON_ACTIVATE | NK_EDIT_FIELD | NK_EDIT_SIG_ENTER;
		nk_edit_focus(context, edit_flags);
		int edit_state = nk_edit_string_zero_terminated(context, edit_flags, console->console_command_text, MAX_CONSOLE_MESSAGE_LEN, console_filter);
		if(edit_state & NK_EDIT_COMMITED)
		{
			log_message("New message entered : %s", console->console_command_text);
			memset(console->console_command_text, '\0', MAX_CONSOLE_MESSAGE_LEN);
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
	vsnprintf(console->console_messages[console->current_message_index], MAX_CONSOLE_MESSAGE_LEN, message, args);
}