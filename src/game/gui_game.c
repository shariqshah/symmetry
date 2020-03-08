#include "gui_game.h"
#include "gui.h"
#include "game.h"
#include "../system/platform.h"

static void gui_game_pause_menu(struct nk_context* context);

void gui_game_init(struct Gui* game_gui)
{
	gui_theme_set(game_state_get()->gui_game, GT_RED);
}

void gui_game_cleanup(struct Gui* game_gui)
{

}

void gui_game_update(struct Gui* gui_game, float dt)
{
	struct nk_context* context = &gui_game->context;
	struct Game_State* game_state = game_state_get();

	if(game_state->game_mode == GAME_MODE_GAME)
	{
		if(nk_begin(context, "Game Gui", nk_rect(50, 50, 400, 200), NK_WINDOW_CLOSABLE))
		{
			nk_layout_row_dynamic(context, 30, 1);
			nk_label(context, "Hello from the game gui!", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
			nk_end(context);
		}
	}
	else if(game_state->game_mode == GAME_MODE_PAUSE)
	{
		gui_game_pause_menu(context);
	}

}

void gui_game_pause_menu(struct nk_context* context)
{
	struct Game_State* game_state = game_state_get();
	int window_width = 0, window_height = 0;
	window_get_drawable_size(game_state->window, &window_width, &window_height);
	if(nk_begin(context, "Pause Gui", nk_rect(0, 0, window_width, window_height), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(context, 30, 1);
		nk_label(context, "Hello from the Pause Menu!", NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE);
		nk_end(context);
	}
}
