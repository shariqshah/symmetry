#ifndef GUI_GAME_H
#define GUI_GAME_H

struct Gui;

void gui_game_init(struct Gui* game_gui);
void gui_game_cleanup(struct Gui* game_gui);
void gui_game_update(struct Gui* gui_game, float dt);

#endif