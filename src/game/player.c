#include "player.h"
#include "scene.h"
#include "input.h"
#include "../common/utils.h"
#include "transform.h"
#include "camera.h"
#include "bounding_volumes.h"
#include "../common/hashmap.h"
#include "../common/log.h"
#include "entity.h"
#include "../system/config_vars.h"
#include "../system/platform.h"
#include "game.h"
#include "debug_vars.h"
#include "geometry.h"

void player_init(struct Player* player, struct Scene* scene)
{
	struct Game_State* game_state = game_state_get();
    entity_init(player, "Player", &scene->root_entity);
    player->base.flags   |= EF_ACTIVE;
    player->base.id      =  1;
    player->base.type    =  ET_PLAYER;

    struct Hashmap* config = game_state->cvars;
    player->move_speed            = hashmap_int_get(config, "player_move_speed");
    player->move_speed_multiplier = hashmap_int_get(config, "player_move_speed_multiplier");
    player->turn_speed            = hashmap_int_get(config, "player_turn_speed");

    player->mesh = scene_static_mesh_create(scene, "Player_Mesh", player, "sphere.symbres", MAT_BLINN);

    struct Camera* player_camera = &scene->cameras[CAM_GAME];
    entity_rename(player_camera, "Player_Camera");
    player_camera->base.flags |= EF_ACTIVE;
    player_camera->clear_color.x = 0.6f;
    player_camera->clear_color.y = 0.6f;
    player_camera->clear_color.z = 0.9f;
    player_camera->clear_color.w = 1.f;
    player->camera_node = player_camera;

	// Mark player camera and mesh as transient for now. We don't need to save them to file since we recreate them here anyway
	player->camera_node->base.flags |= EF_TRANSIENT;
	player->mesh->base.flags |= EF_TRANSIENT;

	if(player_camera->fbo == -1)
	{
		int render_width = hashmap_int_get(config, "render_width");
		int render_height = hashmap_int_get(config, "render_height");
		window_get_drawable_size(game_state->window, &render_width, &render_height);
		camera_attach_fbo(player_camera, render_width, render_height, true, true, true);
	}

    transform_parent_set(player_camera, player, true);

    vec3 cam_translation = {0.f, 1.5f, 0.f};
    transform_translate(player_camera, &cam_translation, TS_LOCAL);

    //vec3 cam_axis = {-1.f, 0.f, 0.f};
    //transform_rotate(player_camera, &cam_axis, 85.f, TS_LOCAL);

	sound_listener_set(game_state->sound, player_camera);
	sound_listener_update(game_state->sound);
}

void player_destroy(struct Player* player)
{
    entity_reset(player, player->base.id);
	scene_entity_base_remove(game_state_get()->scene, &player->base);
    player->base.flags = EF_NONE;
}

void player_update(struct Player* player, struct Scene* scene, float dt)
{
    /* Look around */
    static float total_pitch = 0.f;
    float pitch              = 0.f;
    float yaw                = 0.f;
    float max_pitch          = 80.f;
	vec3  rot_axis_pitch     = { 1, 0, 0 };
	vec3  rot_axis_yaw       = { 0, 1, 0 };

    if(input_map_state_get("Turn_Up",    KS_PRESSED)) pitch += player->turn_speed;
    if(input_map_state_get("Turn_Down",  KS_PRESSED)) pitch -= player->turn_speed;
    if(input_map_state_get("Turn_Right", KS_PRESSED)) yaw   += player->turn_speed;
    if(input_map_state_get("Turn_Left",  KS_PRESSED)) yaw   -= player->turn_speed;

	int cursor_yaw, cursor_pitch;
	input_mouse_delta_get(&cursor_yaw, &cursor_pitch);
	if(input_mouse_mode_get() != MM_RELATIVE)
	{
		input_mouse_mode_set(MM_RELATIVE);
		cursor_yaw = cursor_pitch = 0;
	}

	pitch = -cursor_pitch * player->turn_speed * dt;
	yaw = cursor_yaw * player->turn_speed * dt;

    total_pitch += pitch;
    if(total_pitch >= max_pitch)
    {
		total_pitch = max_pitch;
		pitch = 0.f;
    }
    else if(total_pitch <= -max_pitch)
    {
		total_pitch = -max_pitch;
		pitch = 0.f;
    }

    if(yaw != 0.f)
		transform_rotate(player, &rot_axis_yaw, -yaw, TS_WORLD);

    if(pitch != 0.f)
		transform_rotate(player->camera_node, &rot_axis_pitch, pitch, TS_LOCAL);

    /* Movement */
    float move_speed = player->move_speed;
    vec3 offset = {0.f, 0.f, 0.f};

    if(input_map_state_get("Sprint",        KS_PRESSED)) move_speed *= player->move_speed_multiplier;
    if(input_map_state_get("Move_Forward",  KS_PRESSED)) offset.z   -= move_speed;
    if(input_map_state_get("Move_Backward", KS_PRESSED)) offset.z   += move_speed;
    if(input_map_state_get("Move_Left",     KS_PRESSED)) offset.x   -= move_speed;
    if(input_map_state_get("Move_Right",    KS_PRESSED)) offset.x   += move_speed;
    if(input_map_state_get("Move_Up",       KS_PRESSED)) offset.y   += move_speed;
    if(input_map_state_get("Move_Down",     KS_PRESSED)) offset.y   -= move_speed;

    vec3_scale(&offset, &offset, dt);
	if(offset.x != 0 || offset.y != 0 || offset.z != 0)
	{
		quat_mul_vec3(&offset, &player->camera_node->base.transform.rotation, &offset);
		transform_translate(player, &offset, TS_LOCAL);
	}

	/* Aiming and Projectiles*/
	if(input_mousebutton_state_get(MSB_RIGHT, KS_PRESSED))
	{
		int mouse_x = 0, mouse_y = 0;
		platform_mouse_position_get(&mouse_x, &mouse_y);
		struct Ray ray = camera_screen_coord_to_ray(player->camera_node, mouse_x, mouse_y);
		log_message("Ray: %.3f, %.3f, %.3f", ray.direction.x, ray.direction.y, ray.direction.z);

		struct Raycast_Result ray_result;
		scene_ray_intersect(scene, &ray, &ray_result, ERM_ALL);
	}

	vec3 mesh_abs = { 0.f, 0.f, 0.f };
	transform_get_absolute_position(player->mesh, &mesh_abs);
	debug_vars_show_vec3("Player Position", &player->base.transform.position);
	debug_vars_show_vec3("Mesh Position", &mesh_abs);
	debug_vars_show_vec3("Min", &player->mesh->base.derived_bounding_box.min);
	debug_vars_show_vec3("Max", &player->mesh->base.derived_bounding_box.max);
	struct Geometry* geom = geom_get(player->mesh->model.geometry_index);
	debug_vars_show_vec3("Geom Min", &geom->bounding_box.min);
	debug_vars_show_vec3("Geom Max", &geom->bounding_box.max);
	debug_vars_show_texture("Player Camera Render", player->camera_node->render_tex);
	debug_vars_show_texture("Player Camera Depth", player->camera_node->depth_tex);
}
