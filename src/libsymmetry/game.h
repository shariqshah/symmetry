#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

#if defined(_MSC_VER)
	#define SYMMETRY_EXPORT __declspec(dllexport)
#else
	#define SYMMETRY_EXPORT
#endif

struct Window;
struct Platform_Api;

struct Game_State
{
	struct Window* window;
	int            player_node;
	int            player_pitch_node;
	bool           is_initialized;
};


struct Game_State*   game_state_get(void);
SYMMETRY_EXPORT bool game_init(struct Window* window, struct Platform_Api* platform_api);
SYMMETRY_EXPORT void game_cleanup(void);

#endif
