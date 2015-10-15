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
#include "utils.h"
#include "texture.h"
#include "material.h"

void run(void);
void update(float dt);
void render(void);
void debug(float dt);
void scene_setup(void);

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
	texture_init();
	transform_init();
	camera_init();
	geom_init();
	material_init();
	model_init();
	entity_init();
	scene_init();

	/* Debug scene setup */
	scene_setup();

	run();
}

void scene_setup(void)
{
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
	
	struct Entity* player = scene_add_new("player", "None");
	player_node = player->node;
	vec3 viewer_pos = {0, 0, 10};
	struct Transform* viewer_tran = entity_component_get(player, C_TRANSFORM);
	transform_set_position(viewer_tran, &viewer_pos);
	entity_component_add(player, C_CAMERA, 800, 600);
	
	struct Entity* new_ent = scene_add_new("Model_Entity", NULL);
	struct Transform* tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 position = {0, 0, -5};
	vec4 color = {1.f, 1.f, 1.f, 1.f };
	transform_translate(tran, &position, TS_WORLD);
	struct Model* box_model = entity_component_add(new_ent, C_MODEL, "default.pamesh");
	struct Transform* model_tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 scale = {1, 1, 2};
	transform_scale(model_tran, &scale);

	struct Entity* suz = scene_add_as_child("Suzanne", NULL, new_ent);
	struct Model* suz_model = entity_component_add(suz, C_MODEL, "suzanne.pamesh");
	struct Transform* s_tran = entity_component_get(suz, C_TRANSFORM);
	vec3 s_pos = {3, 0, 0};
	transform_translate(s_tran, &s_pos, TS_WORLD);

	struct Entity* ground = scene_add_new("Ground", NULL);
	struct Model* ground_model = entity_component_add(ground, C_MODEL, "plane.pamesh");
	struct Transform* ground_tran = entity_component_get(ground, C_TRANSFORM);
	vec3 pos = {0, -3, -3};
	vec3 scale_ground = {0.5f, 0.5f, 3.f};
	transform_set_position(ground_tran, &pos);
	transform_scale(ground_tran, &scale_ground);

	/* Set material params */
	model_set_material_param(ground_model, "diffuse_color", &color);
	model_set_material_param(suz_model, "diffuse_color", &color);
	model_set_material_param(box_model, "diffuse_color", &color);
}

void debug(float dt)
{
	struct Entity* entity = entity_get(player_node);
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
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

	double cursor_lr, cursor_ud;
	input_cursor_pos_get(&cursor_lr, &cursor_ud);
	
	if(input_mousebutton_state_get(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS))
	{
		input_cursor_mode_set(CM_LOCKED);
		const double scale = 0.25;
		turn_up_down = -cursor_ud * turn_speed * dt * scale;
		turn_left_right = cursor_lr * turn_speed * dt * scale;
		input_cursor_pos_set(0.0, 0.0);
	}
	else
	{
		input_cursor_mode_set(CM_NORMAL);
		turn_up_down *= dt;
		turn_left_right *= dt;
	}
	
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
		transform_rotate(transform, &rot_axis_left_right, -turn_left_right, TS_WORLD);
		vec3 up = {0.f, 0.f, 0.f};
		vec3 forward = {0.f, 0.f, 0.f};
		vec3 lookat = {0.f, 0.f, 0.f};
		transform_get_up(transform, &up);
		transform_get_forward(transform, &forward);
		transform_get_lookat(transform, &lookat);
		/* log_message("Up : %s", tostr_vec3(&up)); */
		/* log_message("FR : %s", tostr_vec3(&forward)); */
	}
	if(turn_up_down != 0.f)
	{
		transform_rotate(transform, &rot_axis_up_down, turn_up_down, TS_LOCAL);
		vec3 up = {0.f, 0.f, 0.f};
		vec3 forward = {0.f, 0.f, 0.f};
		vec3 lookat = {0.f, 0.f, 0.f};
		transform_get_up(transform, &up);
		transform_get_forward(transform, &forward);
		transform_get_lookat(transform, &lookat);
		/* log_message("Up : %s", tostr_vec3(&up)); */
		/* log_message("FR : %s", tostr_vec3(&forward)); */
	}
	
	/* Movement */
	if(input_map_state_get("Move_Forward", GLFW_PRESS)) offset.z -= move_speed;
	if(input_map_state_get("Move_Backward", GLFW_PRESS)) offset.z += move_speed;
	if(input_map_state_get("Move_Left", GLFW_PRESS)) offset.x -= move_speed;
	if(input_map_state_get("Move_Right", GLFW_PRESS)) offset.x += move_speed;
	if(input_map_state_get("Move_Up", GLFW_PRESS)) offset.y += move_speed;
	if(input_map_state_get("Move_Down", GLFW_PRESS)) offset.y -= move_speed;

	vec3_scale(&offset, &offset, dt);
	if(offset.x != 0 || offset.y != 0 || offset.z != 0)
	{
		transform_translate(transform, &offset, TS_LOCAL);
 		log_message("Position : %s", tostr_vec3(&transform->position));
	}

	if(input_key_state_get(GLFW_KEY_SPACE, GLFW_PRESS))
	{
		struct Entity* model = entity_get(2);
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 x_axis = {1, 0, 0};
		transform_rotate(mod_tran, &x_axis, 25.f * dt, TS_WORLD);
	}

	if(input_key_state_get(GLFW_KEY_M, GLFW_PRESS))
	{
		struct Entity* model = entity_get(2);
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 y_axis = {0, 0, 1};
		transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_LOCAL);
	}

	if(input_key_state_get(GLFW_KEY_N, GLFW_PRESS))
	{
		/* struct Entity* model = entity_get(3); */
		/* struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM); */
		/* vec3 amount = {0, 0, -5 * dt}; */
		/* transform_translate(mod_tran, amount, TS_LOCAL); */
		struct Entity* model = entity_get(2);
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 y_axis = {0, 0, 1};
		transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_WORLD);
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
	material_cleanup();
	geom_cleanup();
	transform_cleanup();
	camera_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	texture_cleanup();
	shader_cleanup();
}
