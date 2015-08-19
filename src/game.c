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
#include "model.h"

void run(void);
void update(float dt);
void render(void);
void debug(float dt);

struct Entity* entity = NULL;
struct Camera* active_camera = NULL;

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
	model_init();
	
	
	int up_keys[2] = {'W', GLFW_KEY_UP};
	int down_keys[2] = {'S', GLFW_KEY_DOWN};
	int left_keys[2] = {'A', GLFW_KEY_LEFT};
	int right_keys[2] = {'D', GLFW_KEY_RIGHT};
	input_map_create("Move_Up", up_keys, 2);
	input_map_create("Move_Down", down_keys, 2);
	input_map_create("Move_Left", left_keys, 2);
	input_map_create("Move_Right", right_keys, 2);

	int shader = shader_create("unshaded.vert", "unshaded.frag");
	entity = entity_create("Test", "None");
	active_camera = entity_component_add(entity, C_CAMERA, 800, 600);
	struct Entity* new_ent = entity_create("Model_Entity", NULL);
	struct Transform* tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 position = {0, 0, -5};
	transform_translate(tran, position, TS_WORLD);
	struct Model* model = entity_component_add(new_ent, C_MODEL, "default.pamesh");
	
	run();
}

void debug(float dt)
{
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	float speed = 5.f;
	vec3 offset = {0, 0, 0};
	if(input_map_state_get("Move_Up", GLFW_PRESS)) offset[2] -= speed;
	if(input_map_state_get("Move_Down", GLFW_PRESS)) offset[2] += speed;
	if(input_map_state_get("Move_Left", GLFW_PRESS)) offset[0] -= speed;
	if(input_map_state_get("Move_Right", GLFW_PRESS)) offset[0] += speed;
	
	vec3_scale(offset, offset, dt);
	if(offset[0] != 0 || offset[2] != 0)
	{
		transform_translate(transform, offset, TS_WORLD);
		log_message("Position : %.3f, %.3f, %.3f",
					transform->position[0],
					transform->position[1],
					transform->position[2]);
	}
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

	debug(dt);
}

void render(void)
{
	renderer_draw();
}

void game_cleanup(void)
{
	entity_cleanup();
	model_cleanup();
	geom_cleanup();
	transform_cleanup();
	camera_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	shader_cleanup();
}
