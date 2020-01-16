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

#include <float.h>

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
	player->grounded              = true;

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

	//if(player_camera->fbo == -1)
	//{
	//	int render_width = hashmap_int_get(config, "render_width");
	//	int render_height = hashmap_int_get(config, "render_height");
	//	window_get_drawable_size(game_state->window, &render_width, &render_height);
	//	camera_attach_fbo(player_camera, render_width, render_height, true, true, true);
	//}

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
	float gravity        = 0.1f;
	float dampening_x    = 0.08f;
	float dampening_z    = 0.08f;
	float jump_velocity  = 20.f;
    float move_speed     = player->move_speed;
	static vec3 velocity = { 0.f, 0.f, 0.f };
	vec3 max_velocity    = { player->move_speed * player->move_speed_multiplier, jump_velocity, player->move_speed * player->move_speed_multiplier };
	static bool jumping  = false;

	// If we started jumping last frame, set jumpig to false
	if(jumping) jumping = false;

    if(input_map_state_get("Sprint",        KS_PRESSED))  move_speed *= player->move_speed_multiplier;
    if(input_map_state_get("Move_Forward",  KS_PRESSED))  velocity.z   -= move_speed;
    if(input_map_state_get("Move_Backward", KS_PRESSED))  velocity.z   += move_speed;
    if(input_map_state_get("Move_Left",     KS_PRESSED))  velocity.x   -= move_speed;
    if(input_map_state_get("Move_Right",    KS_PRESSED))  velocity.x   += move_speed;
	if(input_map_state_get("Jump", KS_PRESSED))
	{
		if(player->grounded)
		{
			velocity.y += jump_velocity;
			jumping = true;
			player->grounded = false;
		}
	}

	// Dampen Velocity
	if(velocity.x > 0.f)
		velocity.x -= dampening_x;
	else if(velocity.x < 0.f)
		velocity.x += dampening_x;

	//if(velocity.y >= 0.f)
	velocity.y -= gravity;

	if(velocity.z > 0.f)
		velocity.z -= dampening_z;
	else if(velocity.z < 0.f)
		velocity.z += dampening_z;

	// Clamp velocity to min/max
	if(velocity.x > max_velocity.x)
		velocity.x = max_velocity.x;
	else if(velocity.x < -max_velocity.x)
		velocity.x = -max_velocity.x;

	if(velocity.y > max_velocity.y)
		velocity.y = max_velocity.y;
	if(velocity.y < -max_velocity.y)
		velocity.y = -max_velocity.y;

	if(velocity.z > max_velocity.z)
		velocity.z = max_velocity.z;
	if(velocity.z < -max_velocity.z)
		velocity.z = -max_velocity.z;


	/* Check for collisions ahead */
	int mouse_x = 0, mouse_y = 0;
	platform_mouse_position_get(&mouse_x, &mouse_y);
	//struct Ray forward_ray = camera_screen_coord_to_ray(player->camera_node, 0, 0);
	struct Ray forward_ray = { 0 };
	transform_get_absolute_position(player, &forward_ray.origin);
	transform_get_absolute_forward(player->camera_node, &forward_ray.direction);

	// Get all the entities that intersect then check the distance if it is less than
	// or equal to min_collision_distance then we are colliding
	float min_collision_distance = 5.0f;
	struct Raycast_Result ray_result;
	scene_ray_intersect(scene, &forward_ray, &ray_result, ERM_STATIC_MESH);
	debug_vars_show_int("Colliding Entities", ray_result.num_entities_intersected);
	if(ray_result.num_entities_intersected > 0)
	{
		for(int i = 0; i < ray_result.num_entities_intersected; i++)
		{
			struct Entity* colliding_entity = ray_result.entities_intersected[i];
			float distance = bv_distance_ray_bounding_box(&forward_ray, &colliding_entity->derived_bounding_box);
			debug_vars_show_float("Collision ahead", distance);
			if(distance > 0.f && distance <= min_collision_distance && colliding_entity != player->mesh)
			{
				vec3 intersection_point = forward_ray.direction;
				vec3_scale(&intersection_point, &intersection_point, distance);
				vec3_add(&intersection_point, &intersection_point, &forward_ray.origin);

				struct Bounding_Box* box = &colliding_entity->derived_bounding_box;
				vec3 normal = bv_bounding_box_normal_from_intersection_point(box, &forward_ray, intersection_point);

				struct Ray normal_ray;
				normal_ray.origin = intersection_point;
				normal_ray.direction = normal;
				im_ray(&normal_ray, 5.f, (vec4) { 1.f, 0.f, 0.f, 1.f }, 3);

				float dot = (vec3_dot(&velocity, &normal));
				vec3 norm_scaled = { 0.f };
				vec3_scale(&norm_scaled, &normal, dot);
				vec3_sub(&velocity, &velocity, &norm_scaled);
				debug_vars_show_vec3("Normal", &normal);
				debug_vars_show_float("Dot", dot);
			}
		}
	}

	/* Check for collisions below */
	struct Ray downward_ray;
	transform_get_absolute_position(player->mesh, &downward_ray.origin);
	vec3_fill(&downward_ray.direction, 0.f, -1.f, 0.f);
	struct Raycast_Result down_ray_result;
	scene_ray_intersect(scene, &downward_ray, &down_ray_result, ERM_STATIC_MESH);
	if(down_ray_result.num_entities_intersected > 0)
	{
		float min_downward_distance = 2.f;
		for(int i = 0; i < down_ray_result.num_entities_intersected; i++)
		{
			struct Entity* colliding_entity = down_ray_result.entities_intersected[i];
			if(colliding_entity == player->mesh)
				continue;
			float distance = bv_distance_ray_bounding_box(&downward_ray, &colliding_entity->derived_bounding_box);
			debug_vars_show_float("Collision below", distance);
			if(distance > 0.f && distance <= min_downward_distance && !jumping)
			{
				velocity.y = 0.f;
				player->grounded = true;
			}
		}
	}

	float min_velocity = 0.0001f;
	float fract_part = 0.f;
	double int_part = 0.f;
	double int_part2 = 0.f;
	fract_part = modf(velocity.x, &int_part);
	if(fabsf(fract_part) < min_velocity) velocity.x = 0.f;
	fract_part = modf(velocity.z, &int_part2);
	if(fabsf(fract_part) < min_velocity) velocity.z = 0.f;

	debug_vars_show_vec3("velocity", &velocity);
	debug_vars_show_bool("Grounded", player->grounded);

    vec3 offset = {0.f, 0.f, 0.f};
	vec3_assign(&offset, &velocity);
    vec3_scale(&offset, &offset, dt);
	if(offset.x != 0 || offset.z != 0)
	{
		quat_mul_vec3(&offset, &player->camera_node->base.transform.rotation, &offset);
		offset.y = 0.f;
	}

	if(velocity.y != 0.f)
		offset.y = velocity.y * dt;

	transform_translate(player, &offset, TS_LOCAL);

	/* Aiming and Projectiles*/
	if(input_mousebutton_state_get(MSB_RIGHT, KS_PRESSED))
	{
		log_message("Right Click");
		int mouse_x = 0, mouse_y = 0;
		platform_mouse_position_get(&mouse_x, &mouse_y);
		struct Ray bullet_ray = camera_screen_coord_to_ray(player->camera_node, mouse_x, mouse_y);

		struct Raycast_Result bullet_ray_result;
		scene_ray_intersect(scene, &bullet_ray, &bullet_ray_result, ERM_STATIC_MESH);
		if(bullet_ray_result.num_entities_intersected > 0)
		{
			for(int i = 0; i < bullet_ray_result.num_entities_intersected; i++)
			{
				struct Entity* colliding_entity = bullet_ray_result.entities_intersected[i];
				if(colliding_entity == player->mesh)
					continue;
				float distance = bv_distance_ray_bounding_box(&bullet_ray, &colliding_entity->derived_bounding_box);
				if(distance > 0.f)
				{
					vec3 collision_point = bullet_ray.direction;
					vec3_scale(&collision_point, &collision_point, distance);
					vec3_add(&collision_point, &collision_point, &bullet_ray.origin);
					struct Static_Mesh* bullet = scene_static_mesh_create(game_state_get()->scene, "bullet", NULL, "cube.symbres", MAT_UNSHADED);
					transform_set_position(bullet, &collision_point);
				}
			}
		}
	}

	debug_vars_show_float("Frame Time", dt * 100000.f);
}
