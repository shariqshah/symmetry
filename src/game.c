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
#include "transform.h"
#include "camera.h"

void run(void);
void update(float dt);
void render(void);
void debug(float dt);

struct Entity* entity = NULL;

void game_init(void)
{
	GLFWwindow* window = window_get_active();
	/* Init systems */
	input_init(window);
	renderer_init(window);
	io_file_init("/mnt/Dev/Projects/Symmetry/assets/");/* TODO: Implement proper way of getting binary directory */
	shader_init();
	transform_init();
	camera_init();
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
	entity = entity_create("Test", "None");
	
	run();
}

void debug(float dt)
{
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	vec3 offset = {0, 5, 0};
	vec3_scale(offset, offset, dt);
	transform_translate(transform, offset, TS_WORLD);
	log_message("Position : %.3f, %.3f, %.3f", transform->position[0], transform->position[1], transform->position[2]);
}

void run(void)
{
	double last_time = glfwGetTime();
	while(!window_should_close())
	{
		double curr_time = glfwGetTime();
		float delta_time = (float)(curr_time - last_time);
		last_time = curr_time;
		
		update(delta_time);
		render();
		window_swap_buffers();
		window_poll_events();
	}
}

void update(float dt)
{
	input_update();
	if(input_key_state_get(GLFW_KEY_ESCAPE, GLFW_PRESS))
		window_set_should_close(1);

	if(input_map_state_get("MoveUp", GLFW_PRESS))
		debug(dt);
}

void render(void)
{
	renderer_draw();
}

void game_cleanup(void)
{
	entity_cleanup();
	geom_cleanup();
	transform_cleanup();
	camera_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	shader_cleanup();
}
