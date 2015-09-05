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
#include "scene.h"

void run(void);
void update(float dt);
void render(void);
void debug(float dt);

int player_node = -1;
int player_pitch_node = -1;

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
	geom_init();
	model_init();
	entity_init();
	scene_init();
	
	
	int forward_keys[2] = {'W', GLFW_KEY_UP};
	int backward_keys[2] = {'S', GLFW_KEY_DOWN};
	int up_keys[2] = {'Q'};
	int down_keys[2] = {'E'};
	int left_keys[2] = {'A', GLFW_KEY_LEFT};
	int right_keys[2] = {'D', GLFW_KEY_RIGHT};
	int turn_right_keys[1] = {'L'};
	int turn_left_keys[1] = {'J'};
	int turn_up_keys[1] = {'I'};
	int turn_down_keys[1] = {'K'};
	input_map_create("Move_Forward", forward_keys, 2);
	input_map_create("Move_Backward", backward_keys, 2);
	input_map_create("Move_Up", up_keys, 1);
	input_map_create("Move_Down", down_keys, 1);
	input_map_create("Move_Left", left_keys, 2);
	input_map_create("Move_Right", right_keys, 2);
	input_map_create("Turn_Right", turn_right_keys, 1);
	input_map_create("Turn_Left", turn_left_keys, 1);
	input_map_create("Turn_Up", turn_up_keys, 1);
	input_map_create("Turn_Down", turn_down_keys, 1);

	int shader = shader_create("unshaded.vert", "unshaded.frag");
	struct Entity* player = scene_add_new("player", "None");
	player_node = player->node;
	vec3 viewer_pos = {0, 0, 10};
	struct Transform* viewer_tran = entity_component_get(player, C_TRANSFORM);
	transform_set_position(viewer_tran, viewer_pos);
	struct Entity* player_pitch = scene_add_as_child("player_pitch", NULL, player);
	player_pitch_node = player_pitch->node;
	entity_component_add(player_pitch, C_CAMERA, 800, 600);
	
	struct Entity* new_ent = scene_add_new("Model_Entity", NULL);
	struct Transform* tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 position = {0, 0, -5};
	transform_translate(tran, position, TS_WORLD);
	struct Model* model = entity_component_add(new_ent, C_MODEL, "default.pamesh");
	struct Transform* model_tran = entity_component_get(new_ent, C_TRANSFORM);
	//vec3 axis = {0.f, 1.f, 0.f};
	//transform_rotate(model_tran, axis, (45.f), TS_WORLD);
	vec3 scale = {1, 1, 5};
	transform_scale(model_tran, scale);

	struct Entity* suz = scene_add_as_child("Suzanne", NULL, new_ent);
	entity_component_add(suz, C_MODEL, "suzanne.pamesh");
	struct Transform* s_tran = entity_component_get(suz, C_TRANSFORM);
	vec3 s_pos = {3, 0, 0};
	transform_translate(s_tran, s_pos, TS_WORLD);

	vec4 temp = {1, 0, 0, 1};
	mat4 mat;
	mat4_identity(mat);
	mat4_mul_vec4(temp, mat, temp);
	
	run();
}

void debug(float dt)
{
	struct Entity* entity = entity_get(player_node);
	struct Entity* entity_pitch = entity_get(player_pitch_node);
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	struct Transform* pitch_transform = entity_component_get(entity_pitch, C_TRANSFORM);
	float move_speed = 5.f, turn_speed = 50.f;
	vec3 offset = {0, 0, 0};
	float turn_up_down = 0.f;
	float turn_left_right = 0.f;
	float max_up_down = 60.f;
	static float total_up_down_rot = 0.f;
	vec3 rot_axis_up_down = {1, 0, 0};
	vec3 rot_axis_left_right = {0, 1, 0};

	/* Look around */
	if(input_map_state_get("Turn_Up", GLFW_PRESS)) turn_up_down += turn_speed;
	if(input_map_state_get("Turn_Down", GLFW_PRESS)) turn_up_down -= turn_speed;
	if(input_map_state_get("Turn_Right", GLFW_PRESS)) turn_left_right += turn_speed;
	if(input_map_state_get("Turn_Left", GLFW_PRESS)) turn_left_right -= turn_speed;

	turn_up_down *= dt;
	turn_left_right *= dt;
	total_up_down_rot += turn_up_down;
	if(total_up_down_rot >= max_up_down)
	{
		total_up_down_rot = max_up_down;
		turn_up_down = 0.f;
	}
	else if(total_up_down_rot <= -max_up_down)
	{
		total_up_down_rot = -max_up_down;
		turn_up_down = 0.f;
	}
	
	if(turn_left_right != 0.f)
	{
		transform_rotate(transform, rot_axis_left_right, -turn_left_right, TS_WORLD);
		vec3 up = {0.f};
		vec3 forward = {0.f};
		transform_get_up(transform, up);
		transform_get_forward(transform, forward);
		//log_message("Up : %.3f, %.3f, %.3f", up[0], up[1], up[2]);
		log_message("Forward : %.3f, %.3f, %.3f", forward[0], forward[1], forward[2]);
	}
	if(turn_up_down != 0.f)
	{
		transform_rotate(pitch_transform, rot_axis_up_down, -turn_up_down, TS_LOCAL);
		vec3 up = {0.f};
		vec3 forward = {0.f};
		transform_get_up(transform, up);
		transform_get_forward(transform, forward);
		//log_message("Up : %.3f, %.3f, %.3f", up[0], up[1], up[2]);
		log_message("Forward : %.3f, %.3f, %.3f", forward[0], forward[1], forward[2]);
	}
	
	/* Movement */
	if(input_map_state_get("Move_Forward", GLFW_PRESS)) offset[2] -= move_speed;
	if(input_map_state_get("Move_Backward", GLFW_PRESS)) offset[2] += move_speed;
	if(input_map_state_get("Move_Left", GLFW_PRESS)) offset[0] -= move_speed;
	if(input_map_state_get("Move_Right", GLFW_PRESS)) offset[0] += move_speed;
	if(input_map_state_get("Move_Up", GLFW_PRESS)) offset[1] += move_speed;
	if(input_map_state_get("Move_Down", GLFW_PRESS)) offset[1] -= move_speed;
	
	vec3_scale(offset, offset, dt);
	if(offset[0] != 0 || offset[2] != 0 || offset[1] != 0)
	{
		transform_translate(transform, offset, TS_LOCAL);
 		/* log_message("Position : %.3f, %.3f, %.3f", */
		/* 			transform->position[0], */
		/* 			transform->position[1], */
		/* 			transform->position[2]); */
	}

	if(input_key_state_get(GLFW_KEY_SPACE, GLFW_PRESS))
	{
		struct Entity* model = entity_get(3);
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 x_axis = {1, 0, 0};
		transform_rotate(mod_tran, x_axis, 25.f * dt, TS_WORLD);
	}

	if(input_key_state_get(GLFW_KEY_M, GLFW_PRESS))
	{
		struct Entity* model = entity_get(3);
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 y_axis = {0, 1, 0};
		transform_rotate(mod_tran, y_axis, 25.f * dt, TS_LOCAL);
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
	scene_cleanup();
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
