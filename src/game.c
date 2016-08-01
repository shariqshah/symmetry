#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <GLFW/glfw3.h>

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
#include "framebuffer.h"
#include "light.h"

void run(void);
void update(float dt);
void render(void);
void debug(float dt);
void scene_setup(void);

int player_node = -1;
int player_pitch_node = -1;

void game_init(void)
{
	/* TODO: Implement dealing with init failures */
	GLFWwindow* window = window_get_active();
	/* Init systems */
	input_init(window);
	io_file_init("/mnt/Dev/Projects/symmetry/assets/");/* TODO: Implement proper way of getting binary directory */
	shader_init();
	texture_init();
	framebuffer_init();
	geom_init();
	renderer_init(window);
	transform_init();
	light_init();
	camera_init();
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
	int sprint_keys[2] = {GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT};
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
	input_map_create("Sprint", sprint_keys, 2);
	
	struct Entity* player = scene_add_new("player", "None");
	player_node = player->node;
	vec3 viewer_pos = {5, 4, 20};
	struct Transform* viewer_tran = entity_component_get(player, C_TRANSFORM);
	struct Model* player_model = entity_component_add(player, C_MODEL, "sphere.pamesh", NULL);
	vec4 color = {0.f, 1.f, 1.f, 1.f };
	model_set_material_param(player_model, "diffuse_color", &color);
	vec4_fill(&color, 1.f, 1.f, 1.f, 1.f);
	transform_set_position(viewer_tran, &viewer_pos);
	int render_width, render_height;
	render_width = 1024;
	render_height = 768;
	struct Camera* camera = entity_component_add(player, C_CAMERA, render_width, render_height);
	camera_attach_fbo(camera, render_width, render_height, 1, 1);
	vec4_fill(&camera->clear_color, 0.3f, 0.6f, 0.9f, 1.0f);
	camera_set_primary_viewer(camera);
	
	struct Entity* new_ent = scene_add_new("Model_Entity", NULL);
	struct Transform* tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 position = {0, 0, -5};
	transform_translate(tran, &position, TS_WORLD);
	struct Model* box_model = entity_component_add(new_ent, C_MODEL, "torus.pamesh", "Blinn_Phong");
	model_set_material_param(box_model, "diffuse_color", &color);
	int tex = texture_create_from_file("white.tga", TU_DIFFUSE);
	model_set_material_param(box_model, "diffuse_texture", &tex);
	struct Transform* model_tran = entity_component_get(new_ent, C_TRANSFORM);
	vec3 scale = {1, 1, 1};
	transform_scale(model_tran, &scale);

	int parent_node = new_ent->node;
	int num_suz = 100;
	srand(time(NULL));
	for(int i = 0; i < num_suz; i++)
	{
		int x = rand() % num_suz;
		int y = rand() % num_suz;
		int z = rand() % num_suz;
		x++; y++; z++;
		struct Entity* suz = scene_add_as_child("Suzanne", NULL, parent_node);
		struct Model* suz_model = entity_component_add(suz, C_MODEL, "suzanne.pamesh", "Blinn_Phong");
		model_set_material_param(suz_model, "diffuse_color", &color);
		float spec_str = 80.f;
		model_set_material_param(suz_model, "specular_strength", &spec_str);
		struct Transform* s_tran = entity_component_get(suz, C_TRANSFORM);
		vec3 s_pos = {x, 5, z};
		transform_translate(s_tran, &s_pos, TS_WORLD);
	}
	

	struct Entity* ground = scene_add_new("Ground", NULL);
	struct Model* ground_model = entity_component_add(ground, C_MODEL, "plane.pamesh", "Blinn_Phong");
	model_set_material_param(ground_model, "diffuse_color", &color);
	int white_tex = texture_create_from_file("white.tga", TU_DIFFUSE);
	model_set_material_param(ground_model, "diffuse_texture", &white_tex);
	float spec_str = 80.f;
	model_set_material_param(ground_model, "specular_strength", &spec_str);
	struct Transform* ground_tran = entity_component_get(ground, C_TRANSFORM);
	vec3 pos = {0, -15, 0};
	vec3 scale_ground = {200.f, 200.f, 200.f};
	transform_set_position(ground_tran, &pos);
	transform_scale(ground_tran, &scale_ground);

	/* struct Entity* screen = scene_add_new("Screen", NULL); */
	/* struct Model* screen_model = entity_component_add(screen, C_MODEL, NULL, NULL); */
	/* screen_model->geometry_index = geom_find("Quad"); */
	/* struct Entity* screen_camera = scene_add_as_child("Screen_Camera", NULL, screen->node); */
	/* struct Transform* screen_camera_tran = entity_component_get(screen_camera, C_TRANSFORM); */
	/* transform_rotate(screen_camera_tran, &UNIT_Y, 180.f, TS_WORLD); */
	/* struct Camera* cam = entity_component_add(screen_camera, C_CAMERA, 50, 50); */
	/* camera_attach_fbo(cam, 50, 50, 1, 1); */
	/* model_set_material_param(screen_model, "diffuse_color", &color); */
	/* model_set_material_param(screen_model, "diffuse_texture", &cam->render_tex); */

	const int MAX_LIGHTS = 3;
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		int x = rand() % MAX_LIGHTS;
		int z = rand() % MAX_LIGHTS;
		x++; z++;
		struct Entity* light_ent = scene_add_new("Light_Ent", NULL);
		struct Transform* light_tran = entity_component_get(light_ent, C_TRANSFORM);
		vec3 lt_pos = {x * 20, 0, z * 20};
		transform_set_position(light_tran, &lt_pos);
		struct Light* light_comp = entity_component_add(light_ent, C_LIGHT, LT_POINT);
		vec3_fill(&light_comp->color, 1.f / (float)x, 1.f / ((rand() % 10) + 1.f), 1.f / (float)z);
		light_comp->intensity = 1.f;
	}

	/* struct Entity* sun = scene_add_new("Sun", NULL); */
	/* struct Light* sun_light = entity_component_add(sun, C_LIGHT, LT_DIR); */
	/* sun_light->intensity = 0.8f; */
}

void debug(float dt)
{
	//struct Entity* entity = entity_get(player_node);
	struct Entity* entity = !input_key_state_get('C', GLFW_PRESS) ? entity_get(player_node) : scene_find("Screen_Camera");
	struct Camera* cam = entity_component_get(entity, C_CAMERA);
	camera_set_primary_viewer(cam);
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	float move_speed = 5.f, move_scale = 3.f, turn_speed = 50.f;
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
	
	if(input_mousebutton_state_get(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS))
	{
		input_cursor_mode_set(CM_LOCKED);
		const double scale = 0.25;
		double cursor_lr, cursor_ud;
		input_cursor_pos_get(&cursor_lr, &cursor_ud);
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
	if(input_map_state_get("Sprint", GLFW_PRESS)) move_speed *= move_scale;
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
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		vec3 x_axis = {1, 0, 0};
		transform_rotate(mod_tran, &x_axis, 25.f * dt, TS_WORLD);
	}

	if(input_key_state_get(GLFW_KEY_M, GLFW_PRESS))
	{
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		//vec3 y_axis = {0, 0, 1};
		//transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_LOCAL);
		vec3 amount = {0, 0, -5 * dt};
		transform_translate(mod_tran, &amount, TS_LOCAL);
	}

	if(input_key_state_get(GLFW_KEY_N, GLFW_PRESS))
	{
		struct Entity* model = scene_find("Model_Entity");
		struct Transform* mod_tran = entity_component_get(model, C_TRANSFORM);
		/* vec3 y_axis = {0, 0, 1}; */
		/* transform_rotate(mod_tran, &y_axis, 25.f * dt, TS_WORLD); */
		vec3 amount = {0, 0, 5 * dt};
		transform_translate(mod_tran, &amount, TS_LOCAL);
	}	

	/* if(input_key_state_get(GLFW_KEY_C, GLFW_PRESS)) */
	/* { */
	/* 	struct Entity* cam_ent = scene_find("Screen_Camera"); */
	/* 	struct Camera* cam = entity_component_get(cam_ent, C_CAMERA); */
	/* 	camera_set_primary_viewer(cam); */
	/* } */

	/* if(input_key_state_get(GLFW_KEY_V, GLFW_PRESS)) */
	/* { */
	/* 	struct Camera* cam = entity_component_get(entity, C_CAMERA); */
	/* 	camera_set_primary_viewer(cam); */
	/* } */
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
	light_cleanup();
	transform_cleanup();
	camera_cleanup();
	input_cleanup();
	renderer_cleanup();
	io_file_cleanup();
	framebuffer_cleanup();
	texture_cleanup();
	shader_cleanup();
}
