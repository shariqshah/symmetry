#include "gui_game.h"
#include "gui.h"
#include "game.h"
#include "scene.h"
#include "../common/log.h"
#include "../system/platform.h"
#include "event.h"
#include "input.h"

static bool show_next_level_dialog = false;
static bool show_restart_level_dialog = false;

static void gui_game_pause_menu(struct nk_context* context);
static void gui_game_next_level_dialog(struct nk_context* context);
static void gui_game_restart_level_dialog(struct nk_context* context);
static void gui_game_on_player_death(struct Event* event);
static void gui_game_on_scene_cleared(struct Event* event);

void gui_game_init(struct Gui* game_gui)
{
	gui_theme_set(game_state_get()->gui_game, GT_RED);
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_subscribe(event_manager, EVT_PLAYER_DIED, &gui_game_on_player_death);
	event_manager_subscribe(event_manager, EVT_SCENE_CLEARED, &gui_game_on_scene_cleared);
}

void gui_game_cleanup(struct Gui* game_gui)
{
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_unsubscribe(event_manager, EVT_PLAYER_DIED, &gui_game_on_player_death);
	event_manager_unsubscribe(event_manager, EVT_SCENE_CLEARED, &gui_game_on_scene_cleared);
}

void gui_game_update(struct Gui* gui_game, float dt)
{
	struct nk_context* context = &gui_game->context;
	struct Game_State* game_state = game_state_get();

	if(game_state->game_mode == GAME_MODE_GAME)
	{
		// HUD
		struct Player* player = &game_state->scene->player;
		if(nk_begin(context, "Game Gui", nk_rect(50, 50, 200, 100), NK_WINDOW_CLOSABLE))
		{
			nk_layout_row_dynamic(context, 30, 1);
			nk_labelf(context, NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE, "HP: %d", player->health);
			nk_end(context);
		}

		if(show_next_level_dialog)
			gui_game_next_level_dialog(context);

		if(show_restart_level_dialog)
			gui_game_restart_level_dialog(context);
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

		nk_end(context);
	}
	context->style.window.fixed_background.data.color.a = previous_opacity;
}

static void gui_game_next_level_dialog(struct nk_context* context)
{
	struct Game_State* game_state = game_state_get();
	struct Scene* scene = game_state->scene;
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
	if(nk_begin(context, "Scene Cleared", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		nk_window_set_focus(context, "Scene Cleared");
		if(nk_popup_begin(context, NK_POPUP_DYNAMIC, "Scene Cleared!", popup_flags, nk_recti(popup_x, popup_y, popup_width, popup_height)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			if(nk_button_label(context, "Restart Level"))
			{
				char filename[MAX_FILENAME_LEN];
				strncpy(filename, game_state->scene->filename, MAX_FILENAME_LEN);
				if(!scene_load(game_state->scene, filename, DIRT_INSTALL))
					log_error("gui_game:next_level_dialog", "Failed to reload Level");
				else
					show_next_level_dialog = false;
			}

			if(nk_button_label(context, "Next Level"))
			{
				if(strncmp(scene->next_level_filename, "\0", MAX_FILENAME_LEN) != 0)
				{
					char filename[MAX_FILENAME_LEN];
					strncpy(filename, game_state->scene->next_level_filename, MAX_FILENAME_LEN);
					if(!scene_load(game_state->scene, filename, DIRT_INSTALL))
						log_error("gui_game:next_level_dialog", "Failed to load new Level");
					else
						show_next_level_dialog = false;
				}
				else
				{
					log_error("gui_game:next_level_dialog", "No name provided for next scene");
				}
			}

			if(nk_button_label(context, "Quit"))
				game_state->quit = true;

			nk_popup_end(context);
		}

		nk_end(context);
	}
	context->style.window.fixed_background.data.color.a = previous_opacity;
}

static void gui_game_restart_level_dialog(struct nk_context* context)
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
	if(nk_begin(context, "Player Died Gui", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		nk_window_set_focus(context, "Player Died Gui");
		if(nk_popup_begin(context, NK_POPUP_DYNAMIC, "You Died", popup_flags, nk_recti(popup_x, popup_y, popup_width, popup_height)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			if(nk_button_label(context, "Restart Level"))
			{
				char filename[MAX_FILENAME_LEN];
				strncpy(filename, game_state->scene->filename, MAX_FILENAME_LEN);
				if(!scene_load(game_state->scene, filename, DIRT_INSTALL))
					log_error("gui_game:pause_menu", "Failed to reload Level");
				else
					show_restart_level_dialog = false;
			}

			if(nk_button_label(context, "Quit"))
				game_state->quit = true;

			nk_popup_end(context);
		}

		nk_end(context);
	}
	context->style.window.fixed_background.data.color.a = previous_opacity;
}

void gui_game_on_player_death(struct Event* event)
{
	struct Player_Death_Event* player_death_event = (struct Player_Death_Event*)event;
	show_restart_level_dialog = true;
	input_mouse_mode_set(MM_NORMAL);
}

void gui_game_on_scene_cleared(struct Event* event)
{
	show_next_level_dialog = true;
	input_mouse_mode_set(MM_NORMAL);
}
