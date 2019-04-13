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

enum Game_Mode
{
	GAME_MODE_GAME = 0,
	GAME_MODE_EDITOR
};

struct Game_State
{
	bool                  is_initialized;
	int                   game_mode;
	struct Window*        window;
	struct Renderer*      renderer;
	struct Scene*         scene;
	struct Console*       console;
	struct Gui*     gui;
	struct Event_Manager* event_manager;
	struct Editor*        editor;
};


struct Game_State* game_state_get(void);
bool               game_init(struct Window* window);
bool               game_run(void);
void               game_cleanup(void);

#endif
