#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

struct Window;
struct Platform_Api;

struct Game_State
{
	struct Window* window;
	int            player_node;
	int            player_pitch_node;
	bool           is_initialized;
};


struct Game_State* game_state_get(void);
bool               game_init(struct Window* window, struct Platform_Api* platform_api);
void               game_cleanup(void);
void               game_test(const char* str);

#endif
