#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "GLFW/glfw3.h"
#include "game.h"

#include "window_system.h"
#include "input.h"
#include "renderer.h"
#include "log.h"
#include "file_io.h"
#include "shader.h"
#include "entity.h"
#include "geometry.h"
#include "array.h"

void run(void);
void update(void);
void render(void);

void game_init(void)
{
	GLFWwindow* window = window_get_active();
	/* Init systems */
	input_init(window);
	renderer_init(window);
	io_file_init("/mnt/Dev/Projects/Symmetry/assets/");/* TODO: Implement proper way of getting binary directory */
	shader_init();
	entity_init();
	geom_init();
	
	
	int keys[2] = {'W', GLFW_KEY_UP};
	int keys2[2] = {'S', GLFW_KEY_DOWN};
	int keys3[1] = {'J'};
	int keys4[1] = {'K'};
	input_map_create("MoveUp", keys, 2);
	input_map_create("MoveDown", keys2, 2);
	input_map_create("Test", keys3, 1);
	input_map_create("Test2", keys4, 1);

	int shader = shader_create("phong.vert", "phong.frag");
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
	input_update();
	if(input_key_state_get(GLFW_KEY_ESCAPE, GLFW_PRESS))
		window_set_should_close(1);
}

void render(void)
{
	renderer_draw();
}

void game_cleanup(void)
{
	entity_cleanup();
	geom_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	shader_cleanup();
}
