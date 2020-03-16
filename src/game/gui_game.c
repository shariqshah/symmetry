#include "gui_game.h"
#include "gui.h"
#include "game.h"
#include "scene.h"
#include "../common/log.h"
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
	int row_height = 30;
	int popup_x = 0;
	int popup_y = 0;
	int popup_width = 300;
	int popup_height = 200;
	int display_width = 0;
	int display_height = 0;
	int popup_flags = NK_WINDOW_TITLE | NK_WINDOW_BORDER;
	window_get_drawable_size(game_state_get()->window, &display_width, &display_height);
	popup_x = (display_width / 2) - (popup_width / 2);
	popup_y = (display_height / 2) - (popup_height / 2);

	int background_window_flags = NK_WINDOW_BACKGROUND;
	int previous_opacity = context->style.window.fixed_background.data.color.a;
	context->style.window.fixed_background.data.color.a = 120;
	if(nk_begin(context, "Pause Gui", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		nk_window_set_focus(context, "Pause Gui");
		if(nk_popup_begin(context, NK_POPUP_DYNAMIC, "Game Paused", popup_flags, nk_recti(popup_x, popup_y, popup_width, popup_height)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			int fullscreen = window_fullscreen_get(game_state->window) ? 1 : 0;
			if(nk_checkbox_label(context, "Fullscreen", &fullscreen))
				window_fullscreen_set(game_state->window, (bool)fullscreen);
			nk_label(context, "Resolution", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			if(nk_button_label(context, "Restart Level"))
			{
				char filename[MAX_FILENAME_LEN];
				strncpy(filename, game_state->scene->filename, MAX_FILENAME_LEN);
				if(!scene_load(game_state->scene, filename, DIRT_INSTALL))
					log_error("gui_game:pause_menu", "Failed to reload Level");
			}

			if(nk_button_label(context, "Quit"))
				game_state->quit = true;

			nk_popup_end(context);
		}
		if(nk_button_label(context, "Button"))
		{
			log_message("Pressed!");
		}

		nk_end(context);
	}
}
