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
#include "im_render.h"
#include "event.h"
#include "sound_source.h"

#include <float.h>

static void player_on_mousebutton_released(const struct Event* event);

void player_init(struct Player* player, struct Scene* scene)
{
	struct Game_State* game_state = game_state_get();
    entity_init(player, "Player", &scene->root_entity);
    player->base.flags            |= EF_ACTIVE;
    player->base.id               =  1;
    player->base.type             =  ET_PLAYER;
	player->base.bounding_box.min = (vec3){ -1.5f, -1.5f, -1.0f };
	player->base.bounding_box.max = (vec3){  1.5f,  1.5f,  1.0f };

    struct Hashmap* config = game_state->cvars;
    player->move_speed            = hashmap_float_get(config, "player_move_speed");
    player->move_speed_multiplier = hashmap_float_get(config, "player_move_speed_multiplier");
    player->turn_speed            = hashmap_float_get(config, "player_turn_speed");
    player->jump_speed            = hashmap_float_get(config, "player_jump_speed");
    player->gravity               = hashmap_float_get(config, "player_gravity");
    player->min_downward_distance = hashmap_float_get(config, "player_min_downward_distance");
    player->min_forward_distance  = hashmap_float_get(config, "player_min_forward_distance");
	player->grounded              = true;

    player->mesh = scene_static_mesh_create(scene, "Player_Mesh", player, "sphere.symbres", MAT_BLINN);

    struct Camera* player_camera = &scene->cameras[CAM_GAME];
    entity_rename(player_camera, "Player_Camera");
    player_camera->base.flags |= EF_ACTIVE;
    player_camera->clear_color.x = 0.6f;
    player_camera->clear_color.y = 0.6f;
    player_camera->clear_color.z = 0.9f;
    player_camera->clear_color.w = 1.f;
    player->camera = player_camera;

	struct Sound_Source* weapon_sound = scene_sound_source_create(scene, "Player_Weapon_Sound_Source", player, "sounds/bullet_1.wav", ST_WAV, false, false);
	if(weapon_sound)
		player->weapon_sound = weapon_sound;
	else
		log_error("player:init", "Could not add weapon entity to player");

	// Mark player camera and mesh as transient for now. We don't need to save them to file since we recreate them here anyway
	player->camera->base.flags |= EF_TRANSIENT;
	player->mesh->base.flags |= EF_TRANSIENT;
	player->weapon_sound->base.flags |= EF_TRANSIENT;

    transform_parent_set(player_camera, player, true);

    vec3 cam_translation = {0.f, 1.5f, 0.f};
    transform_translate(player_camera, &cam_translation, TS_LOCAL);

	sound_listener_set(game_state->sound, player_camera);
	sound_listener_update(game_state->sound);

	event_manager_subscribe(game_state->event_manager, EVT_MOUSEBUTTON_RELEASED, &player_on_mousebutton_released);
}

void player_destroy(struct Player* player)
{
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_RELEASED, &player_on_mousebutton_released);
    entity_reset(player, player->base.id);
	scene_entity_base_remove(game_state_get()->scene, &player->base);
    player->base.flags = EF_NONE;
}

void player_update_physics(struct Player* player, struct Scene* scene, float fixed_dt)
{
    /* Look around */
	float total_pitch    = quat_get_pitch(&player->camera->base.transform.rotation);
    float pitch          = 0.f;
    float yaw            = 0.f;
    float max_pitch      = 80.f;
	vec3  rot_axis_pitch = { 1, 0, 0 };
	vec3  rot_axis_yaw   = { 0, 1, 0 };

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

	pitch = -cursor_pitch * player->turn_speed * fixed_dt;
	yaw = cursor_yaw * player->turn_speed * fixed_dt;

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
		transform_rotate(player->camera, &rot_axis_pitch, pitch, TS_LOCAL);

    /* Movement */
    float move_speed     = player->move_speed;
	vec3 move_direction  = { 0.f };

	static bool jumping = false;
	static float move_speed_vertical = 0.f;

	// If we started jumping last frame, set jumpig to false
	if(jumping) jumping = false;

    if(input_map_state_get("Sprint",        KS_PRESSED))  move_speed *= player->move_speed_multiplier;
    if(input_map_state_get("Move_Forward",  KS_PRESSED))  move_direction.z -= 1.f;
    if(input_map_state_get("Move_Backward", KS_PRESSED))  move_direction.z += 1.f;
    if(input_map_state_get("Move_Left",     KS_PRESSED))  move_direction.x -= 1.f;
    if(input_map_state_get("Move_Right",    KS_PRESSED))  move_direction.x += 1.f;
	if(input_map_state_get("Jump", KS_PRESSED))
	{
		if(player->grounded)
		{
			move_speed_vertical += player->jump_speed;
			jumping = true;
			player->grounded = false;
		}
	}

	vec3_norm(&move_direction, &move_direction);
	if(move_direction.x != 0 || move_direction.z != 0)
		quat_mul_vec3(&move_direction, &player->base.transform.rotation, &move_direction);

	// Check for collisions in the directions we want to move
	int mouse_x = 0, mouse_y = 0;
	platform_mouse_position_get(&mouse_x, &mouse_y);
	struct Ray forward_ray = { 0 };
	transform_get_absolute_position(player, &forward_ray.origin);
	vec3_assign(&forward_ray.direction, &move_direction);

	// Get all the entities that intersect then check the distance if it is less than
	// or equal to min_collision_distance then we are colliding
	struct Raycast_Result ray_result;
	scene_ray_intersect(scene, &forward_ray, &ray_result, ERM_STATIC_MESH);
	debug_vars_show_int("Colliding Entities", ray_result.num_entities_intersected);
	if(ray_result.num_entities_intersected > 0)
	{
		for(int i = 0; i < ray_result.num_entities_intersected; i++)
		{
			struct Entity* colliding_entity = ray_result.entities_intersected[i];
			float distance = bv_distance_ray_bounding_box(&forward_ray, &colliding_entity->derived_bounding_box);
			if(distance > 0.f && distance <= player->min_forward_distance && colliding_entity != player->mesh)
			{
				vec3 intersection_point = forward_ray.direction;
				vec3_scale(&intersection_point, &intersection_point, distance);
				vec3_add(&intersection_point, &intersection_point, &forward_ray.origin);

				struct Bounding_Box* box = &colliding_entity->derived_bounding_box;
				vec3 normal = bv_bounding_box_normal_from_intersection_point(box, intersection_point);

				im_ray_origin_dir(intersection_point, normal, 5.f, (vec4) { 1.f, 0.f, 0.f, 1.f }, 3);
				im_ray(&forward_ray, player->min_forward_distance, (vec4) { 0.f, 1.f, 0.f, 1.f }, 3);

				float dot = (vec3_dot(&move_direction, &normal));
				vec3 norm_scaled = { 0.f };
				vec3_scale(&normal, &normal, dot);
				vec3_sub(&move_direction, &move_direction, &normal);
				debug_vars_show_vec3("Normal", &normal);
				debug_vars_show_vec3("Dir", &move_direction);
				debug_vars_show_float("Dot", dot);
			}
		}
	}

	// Check for collisions below
	move_speed_vertical += player->gravity;
	struct Raycast_Result down_ray_result;
	struct Ray downward_ray;
	transform_get_absolute_position(player->mesh, &downward_ray.origin);
	vec3_fill(&downward_ray.direction, 0.f, -1.f, 0.f);
	scene_ray_intersect(scene, &downward_ray, &down_ray_result, ERM_STATIC_MESH);
	if(down_ray_result.num_entities_intersected > 0)
	{
		for(int i = 0; i < down_ray_result.num_entities_intersected; i++)
		{
			struct Entity* colliding_entity = down_ray_result.entities_intersected[i];
			if(colliding_entity == player->mesh)
				continue;
			float distance = bv_distance_ray_bounding_box(&downward_ray, &colliding_entity->derived_bounding_box);
			if(distance > 0.f && distance <= player->min_downward_distance && !jumping)
			{
				move_speed_vertical = 0.f;
				player->grounded = true;
			}
		}
	}

	// Apply speed to direction then translate
    vec3 translation = {0.f, 0.f, 0.f};
	vec3_assign(&translation, &move_direction);

	translation.x *= move_speed * fixed_dt;
	translation.z *= move_speed * fixed_dt;
	translation.y = move_speed_vertical * fixed_dt;
	transform_translate(player, &translation, TS_WORLD);

	debug_vars_show_bool("Grounded", player->grounded);

}

void player_on_mousebutton_released(const struct Event* event)
{
	int button = event->mousebutton.button;
	int state = event->mousebutton.state;
	struct Game_State* game_state = game_state_get();
	struct Scene* scene = game_state->scene;
	struct Player* player = &scene->player;

	if(game_state->game_mode != GAME_MODE_GAME)
		return;

	/* Aiming and Projectiles*/
	if(button == MSB_LEFT)
	{
		log_message("Right Click");
		int half_width = 0, half_height = 0;
		window_get_drawable_size(game_state->window, &half_width, &half_height);
		half_width /= 2;
		half_height /= 2;
		struct Ray bullet_ray = camera_screen_coord_to_ray(player->camera, half_width, half_height);

		struct Entity* colliding_entity = scene_ray_intersect_closest(scene, &bullet_ray, ERM_STATIC_MESH);

		if(!colliding_entity || colliding_entity == player->mesh)
			return;

		float distance = bv_distance_ray_bounding_box(&bullet_ray, &colliding_entity->derived_bounding_box);
		if(distance > 0.f)
		{
			vec3 collision_point = bullet_ray.direction;
			vec3_scale(&collision_point, &collision_point, distance);
			vec3_add(&collision_point, &collision_point, &bullet_ray.origin);
			//struct Static_Mesh* bullet = scene_static_mesh_create(game_state_get()->scene, "bullet", NULL, "cube.symbres", MAT_UNSHADED);
			struct Light* bullet = entity_load("Spot", DIRT_INSTALL, true);
			if(bullet) transform_set_position(bullet, &collision_point);
			sound_source_play(game_state->sound, player->weapon_sound);
		}

	}
}
