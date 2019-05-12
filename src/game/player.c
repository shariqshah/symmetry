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

void player_init(struct Player* player, struct Scene* scene)
{
	struct Game_State* game_state = game_state_get();
    entity_init(player, "Player", &scene->root_entity);
    player->base.active  = true;
    player->base.id      = 1;
    player->base.type    = ET_PLAYER;

    struct Hashmap* config = game_state->cvars;
    player->move_speed            = hashmap_int_get(config, "player_move_speed");
    player->move_speed_multiplier = hashmap_int_get(config, "player_move_speed_multiplier");
    player->turn_speed            = hashmap_int_get(config, "player_turn_speed");

    player->mesh = scene_static_mesh_create(scene, "Player_Mesh", player, "sphere.symbres", MAT_BLINN);

    struct Camera* player_camera = &scene->cameras[CAM_GAME];
    entity_rename(player_camera, "Player_Camera");
    player_camera->base.active = true;
    player_camera->clear_color.x = 0.6f;
    player_camera->clear_color.y = 0.6f;
    player_camera->clear_color.z = 0.9f;
    player_camera->clear_color.w = 1.f;
    player->camera_node = player_camera;

    int render_width  = hashmap_int_get(config, "render_width");
    int render_height = hashmap_int_get(config, "render_height");
    camera_attach_fbo(player_camera, render_width, render_height, true, true, true);
    transform_parent_set(player_camera, player, true);

    vec3 cam_translation = {0.f, 20.f, 2.f};
    transform_translate(player_camera, &cam_translation, TS_LOCAL);

    vec3 cam_axis = {-1.f, 0.f, 0.f};
    transform_rotate(player_camera, &cam_axis, 85.f, TS_LOCAL);

	sound_listener_set(game_state->sound, player);
}

void player_destroy(struct Player* player)
{
    entity_reset(player, player->base.id);
    player->base.active = false;
}

void player_update(struct Player* player, struct Scene* scene, float dt)
{
    float move_speed = player->move_speed;
    vec3 offset = {0.f, 0.f, 0.f};

    /* Movement */
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
		scene_ray_intersect(scene, &ray, &ray_result);
	}
}
