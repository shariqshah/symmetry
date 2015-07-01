#include "game.h"
#include "GLFW/glfw3.h"

#include "window_system.h"
#include "input.h"
#include "renderer.h"
#include "log.h"

void run(void);
void update(void);
void render(void);

void game_init(void)
{
	GLFWwindow* window = window_get_active();
	input_init(window);
	renderer_init(window);
	int keys[2] = {GLFW_KEY_W, GLFW_KEY_UP};
	int keys2[2] = {GLFW_KEY_S, GLFW_KEY_DOWN};
	int keys3[1] = {GLFW_KEY_J};
	int keys4[1] = {GLFW_KEY_K};
	input_map_create("MoveUp", keys, 2);
	input_map_create("MoveDown", keys2, 2);
	input_map_create("Test", keys3, 1);
	input_map_create("Test2", keys4, 1);
	run();
}

void run(void)
{
	while(!window_should_close())
	{
		update();
		render();
		window_swap_buffers();
		window_poll_events();
	}
}

void update(void)
{
	if(input_map_state_get("MoveUp", GLFW_RELEASE))
		log_message("MoveUp pressed!");

	if(input_map_state_get("MoveDown", GLFW_RELEASE))
	{
		input_map_remvove("Test");
		log_message("MoveDown pressed!");
	}

	if(input_map_state_get("Test", GLFW_RELEASE))
		log_message("Test released");

	if(input_map_state_get("Test2", GLFW_RELEASE))
	{
		int keys[] = {GLFW_KEY_R};
		input_map_keys_set("Test2", keys, 1);
		log_message("Test2 released!");
	}

	input_update();
}

void render(void)
{
	renderer_draw();
}

void game_cleanup(void)
{
	input_cleanup();
	renderer_cleanup();
}
