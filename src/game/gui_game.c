#include "gui_game.h"
#include "game.h"
#include "scene.h"
#include "../common/log.h"
#include "../system/platform.h"
#include "event.h"
#include "input.h"
#include "texture.h"
#include "door.h"

#define SKIN_TEXTURE_WIDTH  256
#define SKIN_TEXTURE_HEIGHT 256


static void gui_game_pause_menu(struct nk_context* context);
static void gui_game_next_level_dialog(struct nk_context* context);
static void gui_game_restart_level_dialog(struct nk_context* context);
static void gui_game_on_player_death(struct Event* event);
static void gui_game_on_scene_cleared(struct Event* event);

void gui_game_init(struct Game_Gui* game_gui)
{
	game_gui->gui = calloc(1, sizeof(*game_gui->gui));
	gui_init(game_gui->gui);
	game_gui->show_next_level_dialog    = false;
	game_gui->show_restart_level_dialog = false;
	game_gui->skin.skin_texture         = texture_create_from_file("gui_skin.tga", TU_DIFFUSE);
	game_gui->skin.button               = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(32,  32, 128, 48));
	game_gui->skin.button_active        = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(32,  32, 128, 48));
	game_gui->skin.button_hover         = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(32,  32, 128, 48));
	game_gui->skin.hp                   = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(32,  0,  32, 32));
	game_gui->skin.key                  = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(0,   32, 32, 32));
	game_gui->skin.crosshair            = nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(112, 0, 32, 32));

	struct nk_context* context = &game_gui->gui->context;
	context->style.button.normal = nk_style_item_image(game_gui->skin.button);
	context->style.button.active = nk_style_item_image(game_gui->skin.button_active);
	context->style.button.hover = nk_style_item_image(game_gui->skin.button_hover);

	context->style.window.border_color = nk_rgba_f(0.f, 0.f, 0.f, 1.f);
	context->style.window.background = nk_rgba_f(0.f, 0.f, 0.f, 1.f);
	game_gui->skin.hud_background = nk_style_item_image(nk_subimage_id(game_gui->skin.skin_texture, SKIN_TEXTURE_WIDTH, SKIN_TEXTURE_HEIGHT, nk_recti(32, 32, 128, 48)));
	game_gui->skin.menu_background = nk_style_item_color(nk_rgba_f(0.2f, 0.2f, 0.2f, 0.6f));
	//context->style.window.fixed_background = nk_style_item_color(nk_rgba_f(0.f, 0.f, 0.f, 0.05f));

	gui_font_set(game_gui->gui, "6809_chargen.ttf", 30.f);

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_subscribe(event_manager, EVT_PLAYER_DIED, &gui_game_on_player_death);
	event_manager_subscribe(event_manager, EVT_SCENE_CLEARED, &gui_game_on_scene_cleared);
}

void gui_game_cleanup(struct Game_Gui* game_gui)
{
	texture_remove(game_gui->skin.skin_texture);
	gui_cleanup(game_gui->gui);
	free(game_gui->gui);

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_unsubscribe(event_manager, EVT_PLAYER_DIED, &gui_game_on_player_death);
	event_manager_unsubscribe(event_manager, EVT_SCENE_CLEARED, &gui_game_on_scene_cleared);
}

void gui_game_update(struct Game_Gui* game_gui, float dt)
{
	struct nk_context* context = &game_gui->gui->context;
	struct Game_State* game_state = game_state_get();

	if(game_state->game_mode == GAME_MODE_GAME)
	{
		// HUD
		context->style.window.fixed_background = game_gui->skin.hud_background;
		const float	   key_opacity_full    = 1.f;
		const float	   key_opacity_reduced = 0.3f;
		int			   hud_offset_x        = 50;
		int			   hud_offset_y        = 50;
		int			   hp_gui_width        = 130;
		int			   hp_gui_height       = 48;
		struct Player* player              = &game_state->scene->player;
		if(nk_begin(context, "HP Gui", nk_recti(hud_offset_x, hud_offset_y, hp_gui_width, hp_gui_height), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(context, 40, 2);
			nk_image_color(context, game_gui->skin.hp, nk_rgba_f(1.f, 1.f, 1.f, 1.f));
			nk_labelf(context, NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE, "%d", player->health);
			nk_end(context);
		}

		int key_gui_width  = 192;
		int key_gui_height = 48;
		int display_width  = 0;
		int display_height = 0;
		window_get_drawable_size(game_state_get()->window, &display_width, &display_height);
		int key_gui_pos_x = display_width - (key_gui_width + hud_offset_x);
		int key_gui_pos_y = hud_offset_y;
		if(nk_begin(context, "Key Gui", nk_recti(key_gui_pos_x, key_gui_pos_y, key_gui_width, key_gui_height), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(context, 40, 3);
			nk_image_color(context, game_gui->skin.key, nk_rgba_f(KEY_INDICATOR_COLOR_RED.x, KEY_INDICATOR_COLOR_RED.y, KEY_INDICATOR_COLOR_RED.z, (player->key_mask & DOOR_KEY_MASK_RED) == DOOR_KEY_MASK_RED ? key_opacity_full : key_opacity_reduced));
			nk_image_color(context, game_gui->skin.key, nk_rgba_f(KEY_INDICATOR_COLOR_GREEN.x, KEY_INDICATOR_COLOR_GREEN.y, KEY_INDICATOR_COLOR_GREEN.z, (player->key_mask & DOOR_KEY_MASK_GREEN) == DOOR_KEY_MASK_GREEN ? key_opacity_full : key_opacity_reduced));
			nk_image_color(context, game_gui->skin.key, nk_rgba_f(KEY_INDICATOR_COLOR_BLUE.x, KEY_INDICATOR_COLOR_BLUE.y, KEY_INDICATOR_COLOR_BLUE.z, (player->key_mask & DOOR_KEY_MASK_BLUE) == DOOR_KEY_MASK_BLUE ? key_opacity_full : key_opacity_reduced));
			
			nk_end(context);
		}

		int crosshair_width = 50;
		int crosshair_height = 32;
		int crosshair_x = (display_width / 2) - (crosshair_width / 2);
		int crosshair_y = (display_height / 2) - (crosshair_height / 2);
		context->style.window.fixed_background = nk_style_item_hide();
		if(nk_begin(context, "Crosshair", nk_recti(crosshair_x, crosshair_y, crosshair_width, crosshair_height), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR))
		{
			nk_layout_row_dynamic(context, crosshair_height, 1);
			nk_image_color(context, game_gui->skin.crosshair, nk_rgba_f(1.f, 1.f, 1.f, 0.3f));
			nk_end(context);
		}

		if(game_gui->show_next_level_dialog)
			gui_game_next_level_dialog(context);

		if(game_gui->show_restart_level_dialog)
			gui_game_restart_level_dialog(context);
	}
	else if(game_state->game_mode == GAME_MODE_PAUSE)
	{
		gui_game_pause_menu(context);
	}

}

void gui_game_show_door_locked_dialog(struct Game_Gui* game_gui, struct Door* door)
{
	struct nk_context* context = &game_gui->gui->context;
	struct Game_State* game_state = game_state_get();
	if(game_state->game_mode == GAME_MODE_GAME)
	{
		struct Player* player = &game_state->scene->player;
		int label_flags = NK_TEXT_ALIGN_CENTERED | NK_TEXT_ALIGN_MIDDLE;
		int key_needed_gui_width = 350;
		int key_needed_gui_height = 48;
		int display_width  = 0;
		int display_height = 0;
		window_get_drawable_size(game_state->window, &display_width, &display_height);
		int key_needed_gui_y = 50;
		int keys_needed      = 0;
		bool red_needed      = false;
		bool green_needed    = false;
		bool blue_needed     = false;
		float starting_ratio = 0.65f;

		if((player->key_mask & DOOR_KEY_MASK_RED) != DOOR_KEY_MASK_RED && (door->mask & DOOR_KEY_MASK_RED) == DOOR_KEY_MASK_RED)
		{
			red_needed = true;
			keys_needed++;
			key_needed_gui_width += 100;
			starting_ratio -= 0.1f;
		}

		if((player->key_mask & DOOR_KEY_MASK_GREEN) != DOOR_KEY_MASK_GREEN && (door->mask & DOOR_KEY_MASK_GREEN) == DOOR_KEY_MASK_GREEN)
		{
			green_needed = true;
			keys_needed++;
			key_needed_gui_width += 100;
			starting_ratio -= 0.1f;
		}

		if((player->key_mask & DOOR_KEY_MASK_BLUE) != DOOR_KEY_MASK_BLUE && (door->mask & DOOR_KEY_MASK_BLUE) == DOOR_KEY_MASK_BLUE)
		{
			blue_needed = true;
			keys_needed++;
			key_needed_gui_width += 100;
			starting_ratio -= 0.1f;
		}
		int key_needed_gui_x = (display_width / 2) - (key_needed_gui_width / 2);

		if(nk_begin(context, "Key Needed Gui", nk_recti(key_needed_gui_x, key_needed_gui_y, key_needed_gui_width, key_needed_gui_height), NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR))
		{
			context->style.window.fixed_background = game_gui->skin.menu_background;
			nk_layout_row_begin(context, NK_DYNAMIC, 40, keys_needed + 2);
			nk_layout_row_push(context, starting_ratio);
			nk_label(context, "YOU NEED THE", label_flags);
			
			if(red_needed)
			{
				nk_layout_row_push(context, 0.15f);
				nk_label_colored(context, "RED", label_flags, nk_rgba_fv(&KEY_INDICATOR_COLOR_RED));
			}

			if(green_needed)
			{
				nk_layout_row_push(context, 0.15f);
				nk_label_colored(context, "GREEN", label_flags, nk_rgba_fv(&KEY_INDICATOR_COLOR_GREEN));
			}

			if(blue_needed)
			{
				nk_layout_row_push(context, 0.15f);
				nk_label_colored(context, "BLUE", label_flags, nk_rgba_fv(&KEY_INDICATOR_COLOR_BLUE));
			}

			nk_layout_row_push(context, 0.25f);
			nk_label(context, keys_needed > 1 ? "KEYS" : "KEY", label_flags);
			nk_end(context);
		}
	}
}

void gui_game_pause_menu(struct nk_context* context)
{
	struct Game_State* game_state     = game_state_get();
	struct Game_Gui*   game_gui       = game_state->gui_game;
	int                row_height     = 30;
	int                popup_x        = 0;
	int                popup_y        = 0;
	int                popup_width    = 300;
	int                popup_height   = 200;
	int                display_width  = 0;
	int                display_height = 0;
	int                popup_flags    = NK_WINDOW_TITLE | NK_WINDOW_BORDER;
	window_get_drawable_size(game_state_get()->window, &display_width, &display_height);
	popup_x = (display_width / 2) - (popup_width / 2);
	popup_y = (display_height / 2) - (popup_height / 2);

	int background_window_flags = NK_WINDOW_BACKGROUND;
	if(nk_begin(context, "Pause Gui", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		context->style.window.fixed_background = game_gui->skin.menu_background;
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
}

static void gui_game_next_level_dialog(struct nk_context* context)
{
	struct Game_State* game_state = game_state_get();
	struct Game_Gui* game_gui = game_state->gui_game;
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
	if(nk_begin(context, "Scene Cleared", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		context->style.window.fixed_background = game_gui->skin.menu_background;
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
					game_gui->show_next_level_dialog = false;
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
						game_gui->show_next_level_dialog = false;
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
}

static void gui_game_restart_level_dialog(struct nk_context* context)
{
	struct Game_State* game_state = game_state_get();
	struct Game_Gui* game_gui = game_state->gui_game;
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
	if(nk_begin(context, "Player Died Gui", nk_rect(0, 0, display_width, display_height), background_window_flags))
	{
		context->style.window.fixed_background = game_gui->skin.menu_background;
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
					game_gui->show_restart_level_dialog = false;
			}

			if(nk_button_label(context, "Quit"))
				game_state->quit = true;

			nk_popup_end(context);
		}

		nk_end(context);
	}
}

void gui_game_on_player_death(struct Event* event)
{
	struct Player_Death_Event* player_death_event = (struct Player_Death_Event*)event;
	game_state_get()->gui_game->show_restart_level_dialog = true;
	input_mouse_mode_set(MM_NORMAL);
}

void gui_game_on_scene_cleared(struct Event* event)
{
	game_state_get()->gui_game->show_next_level_dialog = true;
	input_mouse_mode_set(MM_NORMAL);
}
