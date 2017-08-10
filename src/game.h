#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

struct Window;

struct Game_State
{
	struct Window* window;
	int            player_node;
	int            player_pitch_node;
	bool           is_initialized;
};


struct Game_State* game_state_get(void);
int                game_init(struct Window* window);
void               game_cleanup(void);

#endif
