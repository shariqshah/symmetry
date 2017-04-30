#ifndef game_H
#define game_H

#include "platform.h"

struct Game_State
{
	struct Window* window;
	int            player_node;
	int            player_pitch_node;
	int            is_initialized;
};


struct Game_State* game_state_get(void);
int                game_init(struct Window* window);
void               game_cleanup(void);

#endif
