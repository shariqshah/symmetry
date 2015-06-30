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
	if(input_get_key_state(GLFW_KEY_A, GLFW_REPEAT))
		log_message("A released");

	if(input_get_mousebutton_state(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS))
		log_message("Left mouse released");
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
