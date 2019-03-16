#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

struct Window;
struct Renderer;
struct Scene;
struct Entity;
struct Player;
struct Console;
struct Gui_State;

enum Game_Mode
{
	GAME_MODE_GAME = 0,
	GAME_MODE_EDITOR
};

struct Game_State
{
	bool              is_initialized;
	int               game_mode;
	struct Window*    window;
	struct Renderer*  renderer;
	struct Scene*     scene;
	struct Console*   console;
	struct Gui_State* gui;
};


struct Game_State* game_state_get(void);
bool               game_init(struct Window* window);
bool               game_run(void);
void               game_cleanup(void);

#endif
