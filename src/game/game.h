#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

struct Window;
struct Renderer;
struct Scene;
struct Entity;
struct Player;
struct Console;
struct Gui;
struct Event_Manager;
struct Editor;
struct Hashmap;
struct Sound;

enum Game_Mode
{
	GAME_MODE_GAME = 0,
	GAME_MODE_EDITOR,
	GAME_MODE_PAUSE
};

struct Game_State
{
	bool                  is_initialized;
	bool                  quit;
	bool                  update_scene;
	int                   game_mode;
	float                 fixed_delta_time;
	struct Window*        window;
	struct Renderer*      renderer;
	struct Scene*         scene;
	struct Console*       console;
	struct Gui*           gui_editor;
	struct Gui*           gui_game;
	struct Event_Manager* event_manager;
	struct Editor*        editor;
	struct Hashmap*       cvars;
	struct Sound*         sound;
	struct Debug_Vars*    debug_vars;
	struct Hashmap*       scene_init_func_table;
	struct Hashmap*       scene_cleanup_func_table;
};


struct Game_State* game_state_get(void);
bool               game_init(struct Window* window, struct Hashmap* cvars);
bool               game_run(void);
void               game_cleanup(void);
void               game_mode_set(int new_mode);

#endif
