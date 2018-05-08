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
struct Renderer;
struct Scene;
struct Entity;
struct Player;

struct Game_State
{
	bool             is_initialized;
	struct Window*   window;
	struct Player*   player_node;
	struct Entity*   player_pitch_node;
	struct Renderer* renderer;
	struct Scene*    scene;
};


struct Game_State*   game_state_get(void);
SYMMETRY_EXPORT bool game_init(struct Window* window, struct Platform_Api* platform_api);
SYMMETRY_EXPORT void game_cleanup(void);

#endif
