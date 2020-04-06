#ifndef GUI_GAME_H
#define GUI_GAME_H

#include "gui.h"

struct Game_Gui
{
	struct Gui* gui;
	bool        show_next_level_dialog;
	bool        show_restart_level_dialog;

	struct
	{
		int             skin_texture;
		struct nk_image button;
		struct nk_image button_hover;
		struct nk_image button_active;
		struct nk_image check;
		struct nk_image check_cursor;
		struct nk_image hp;
		struct nk_image key;
	} skin;
};

struct Door;

void gui_game_init(struct Game_Gui* game_gui);
void gui_game_cleanup(struct Game_Gui* game_gui);
void gui_game_update(struct Game_Gui* gui_game, float dt);
void gui_game_show_door_locked_dialog(struct Game_Gui* game_gui, struct Door* door);

#endif