#include "enemy.h"
#include "entity.h"
#include "scene.h"
#include "game.h"
#include "transform.h"
#include "sound_source.h"
#include "../common/log.h"
#include "../common/hashmap.h"
#include "../common/parser.h"
#include "event.h"
#include "../system/platform.h"
#include "debug_vars.h"
#include "im_render.h"

#include <string.h>
#include <math.h>
#include <assert.h>

static void enemy_on_scene_loaded(struct Event* event, void* enemy_ptr);
static void enemy_update_physics_turret(struct Enemy* enemy, struct Game_State* game_state, float fixed_dt);
static void enemy_update_ai_turret(struct Enemy* enemy, struct Game_State* game_state, float dt);
static void enemy_state_set_turret(struct Enemy* enemy, int state);

void enemy_init(struct Enemy* enemy, int type)
{
	struct Game_State* game_state = game_state_get();
	struct Scene* scene = game_state->scene;

	enemy->base.type = ET_ENEMY;
	enemy->type = type;

	/* Initialization specific to each enemy type */
	switch(enemy->type)
	{
	case ENEMY_TURRET:
	{
		enemy->health                            = 100;
		enemy->damage                            = 10;
		enemy->Turret.yaw_direction_positive     = true;
		enemy->Turret.scan                       = false;
		enemy->Turret.turn_speed_default         = 50.f;
		enemy->Turret.turn_speed_when_targetting = 100.f;
		enemy->Turret.turn_speed_current         = enemy->Turret.turn_speed_default;
		enemy->Turret.max_yaw                    = 60.f;
		enemy->Turret.target_yaw                 = 0.f;
		enemy->Turret.pulsate_height             = 1.5f;
		enemy->Turret.pulsate_speed_scale        = 0.1f;
		enemy->Turret.attack_cooldown            = 0.05f;
		enemy->Turret.alert_cooldown             = 1.f;
		enemy->Turret.color_default              = (vec4){ 0.f, 1.f, 1.f, 1.f };
		enemy->Turret.color_alert                = (vec4){ 1.f, 1.f, 0.f, 1.f };
		enemy->Turret.color_attack               = (vec4){ 1.f, 0.f, 0.f, 1.f };
		enemy->Turret.time_elapsed_since_alert   = 0.f;
		enemy->Turret.time_elapsed_since_attack  = 0.f;
		enemy->Turret.vision_range               = 15.f;
	}
	break;
	default:
		log_error("enemy:init", "Unsupported Enemy Type");
		break;
	}

	struct Event_Manager* event_manager = game_state->event_manager;
	event_manager_subscribe_with_object(event_manager, EVT_SCENE_LOADED, &enemy_on_scene_loaded, (void*)enemy);
}

void enemy_weapon_sound_set(struct Enemy* enemy, const char* sound_filename, int type)
{
	sound_source_buffer_set(game_state_get()->sound, enemy->weapon_sound, sound_filename, type);
}

void enemy_static_mesh_set(struct Enemy* enemy, const char* geometry_filename, int material_type)
{
	struct Scene* scene = game_state_get()->scene;
	char mesh_name[MAX_ENTITY_NAME_LEN];
	memset(mesh_name, '\0', sizeof(char) * MAX_ENTITY_NAME_LEN);
	snprintf(mesh_name, MAX_ENTITY_NAME_LEN, "%s_Mesh", enemy->base.name);

	struct Static_Mesh* new_mesh = scene_static_mesh_create(scene, mesh_name, enemy, geometry_filename, material_type);
	if(new_mesh)
	{
		if(enemy->mesh) scene_static_mesh_remove(scene, enemy->mesh);
		enemy->mesh = new_mesh;
	}
}

void enemy_update(struct Enemy* enemy, struct Scene* scene, float dt)
{
	static float enemy_update_interval = 1.f / 60.f;
	static float time_elapsed_since_last_update = 0.f;

	time_elapsed_since_last_update += dt;
	if(time_elapsed_since_last_update < enemy_update_interval)
		return;

	time_elapsed_since_last_update = 0.f;
	struct Game_State* game_state = game_state_get();

	switch(enemy->type)
	{
	case ENEMY_TURRET: enemy_update_ai_turret(enemy, game_state, dt); break;
	}

}

void enemy_update_physics(struct Enemy* enemy, struct Scene* scene, float fixed_dt)
{
	struct Game_State* game_state = game_state_get();
	switch(enemy->type)
	{
	case ENEMY_TURRET: enemy_update_physics_turret(enemy, game_state, fixed_dt); break;
	}

}

void enemy_reset(struct Enemy* enemy)
{
	enemy->type = -1;
	enemy->damage = 0;
	enemy->health = 0;

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_unsubscribe_with_object(event_manager, EVT_SCENE_LOADED, &enemy_on_scene_loaded, (void*)enemy);
}

struct Enemy* enemy_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity)
{
	int enemy_type = -1;
	struct Enemy* new_enemy = NULL;
	struct Scene* scene = game_state_get()->scene;

	if(hashmap_value_exists(object->data, "enemy_type")) enemy_type = hashmap_int_get(object->data, "enemy_type");

	if(enemy_type != -1)
	{
		new_enemy = scene_enemy_create(scene, name, parent_entity, enemy_type); // Create enemy with default values then read and update from file if necessary
		if(!new_enemy)
			return new_enemy;
		if(hashmap_value_exists(object->data, "health")) new_enemy->health = hashmap_int_get(object->data, "health");
		if(hashmap_value_exists(object->data, "damage")) new_enemy->damage = hashmap_int_get(object->data, "damage");

		switch(new_enemy->type)
		{
		case ENEMY_TURRET:
		{
			if(hashmap_value_exists(object->data, "turn_speed_default")) new_enemy->Turret.turn_speed_default = hashmap_float_get(object->data, "turn_speed_default");
			if(hashmap_value_exists(object->data, "max_yaw")) new_enemy->Turret.max_yaw = hashmap_float_get(object->data, "max_yaw");
			if(hashmap_value_exists(object->data, "pulsate_speed_scale")) new_enemy->Turret.pulsate_speed_scale = hashmap_float_get(object->data, "pulsate_speed_scale");
			if(hashmap_value_exists(object->data, "pulsate_height")) new_enemy->Turret.pulsate_height = hashmap_float_get(object->data, "pulsate_height");
			if(hashmap_value_exists(object->data, "attack_cooldown")) new_enemy->Turret.attack_cooldown = hashmap_float_get(object->data, "attack_cooldown");
			if(hashmap_value_exists(object->data, "alert_cooldown")) new_enemy->Turret.alert_cooldown = hashmap_float_get(object->data, "alert_cooldown");
			if(hashmap_value_exists(object->data, "vision_range")) new_enemy->Turret.vision_range = hashmap_float_get(object->data, "vision_range");
			if(hashmap_value_exists(object->data, "color_default")) new_enemy->Turret.color_default = hashmap_vec4_get(object->data, "color_default");
			if(hashmap_value_exists(object->data, "color_alert")) new_enemy->Turret.color_alert = hashmap_vec4_get(object->data, "color_alert");
			if(hashmap_value_exists(object->data, "color_attack")) new_enemy->Turret.color_attack = hashmap_vec4_get(object->data, "color_attack");
			if(hashmap_value_exists(object->data, "turn_direction_positive")) new_enemy->Turret.yaw_direction_positive = hashmap_bool_get(object->data, "turn_direction_positive");
		}
		break;
		}
	}
	return new_enemy;
}

void enemy_write(struct Enemy* enemy, struct Hashmap* entity_data)
{
	hashmap_int_set(entity_data, "enemy_type", enemy->type);
	hashmap_int_set(entity_data, "health", enemy->health);
	hashmap_int_set(entity_data, "damage", enemy->damage);

	switch(enemy->type)
	{
	case ENEMY_TURRET:
	{
		hashmap_float_set(entity_data, "turn_speed_default", enemy->Turret.turn_speed_default);
		hashmap_float_set(entity_data, "turn_speed_when_targetting", enemy->Turret.turn_speed_default);
		hashmap_float_set(entity_data, "max_yaw", enemy->Turret.max_yaw);
		hashmap_float_set(entity_data, "pulsate_speed_scale", enemy->Turret.pulsate_speed_scale);
		hashmap_float_set(entity_data, "pulsate_height", enemy->Turret.pulsate_height);
		hashmap_float_set(entity_data, "attack_cooldown", enemy->Turret.attack_cooldown);
		hashmap_float_set(entity_data, "alert_cooldown", enemy->Turret.alert_cooldown);
		hashmap_float_set(entity_data, "vision_range", enemy->Turret.vision_range);
		hashmap_vec4_set(entity_data, "color_default", &enemy->Turret.color_default);
		hashmap_vec4_set(entity_data, "color_alert", &enemy->Turret.color_alert);
		hashmap_vec4_set(entity_data, "color_attack", &enemy->Turret.color_attack);
		hashmap_bool_set(entity_data, "turn_direction_positive", enemy->Turret.yaw_direction_positive);
	}
	break;
	}
}

void enemy_on_scene_loaded(struct Event* event, void* enemy_ptr)
{
	struct Enemy* enemy = (struct Enemy*)enemy_ptr;

	// Assign pointers to static_mesh and sound_source child entities
	struct Entity* enemy_mesh[MAX_ENEMY_MESHES] = { NULL };
	struct Entity* enemy_sound_sources[MAX_ENEMY_SOUND_SOURCES] = { NULL };
	if(entity_get_num_children_of_type(enemy, ET_STATIC_MESH, &enemy_mesh, MAX_ENEMY_MESHES) == MAX_ENEMY_MESHES)
	{
		enemy->mesh = enemy_mesh[0];
	}
	else
	{
		log_error("enemy:on_scene_load", "Could not find %d child mesh entities for enemy %s", MAX_ENEMY_MESHES, enemy->base.name);
	}

	if(entity_get_num_children_of_type(enemy, ET_SOUND_SOURCE, &enemy_sound_sources, MAX_ENEMY_SOUND_SOURCES) == MAX_ENEMY_SOUND_SOURCES)
	{
		enemy->weapon_sound = enemy_sound_sources[0];
		enemy->ambient_sound = enemy_sound_sources[1];
	}
	else
	{
		log_error("enemy:on_scene_load", "Could not find %d child sound source entities for enemy %s", MAX_ENEMY_SOUND_SOURCES, enemy->base.name);
	}

	// Do other post-scene-load initialization stuff per enemy type here
	switch(enemy->type)
	{
	case ENEMY_TURRET:
		enemy_state_set_turret(enemy, TURRET_DEFAULT);
		break;
	}
}

void enemy_update_physics_turret(struct Enemy* enemy, struct Game_State* game_state, float fixed_dt)
{
	/* Turning/Rotation */
	static vec3 yaw_axis = { 0.f, 1.f, 0.f };
	if(enemy->Turret.scan)
	{
		float current_yaw = quat_get_yaw(&enemy->base.transform.rotation);

		float yaw = enemy->Turret.turn_speed_current * 1.f * fixed_dt;
		if(!enemy->Turret.yaw_direction_positive)
			yaw *= -1.f;

		current_yaw += yaw;
		if(current_yaw >= enemy->Turret.max_yaw)
		{
			yaw = 0.f;
			enemy->Turret.yaw_direction_positive = false;
		}
		else if(current_yaw <= -enemy->Turret.max_yaw)
		{
			yaw = 0.f;
			enemy->Turret.yaw_direction_positive = true;
		}

		if(yaw != 0.f)
			transform_rotate(enemy, &yaw_axis, yaw, TS_LOCAL);
	}
	else if(!enemy->Turret.scan)
	{
		float epsilon = 0.5f;
		float current_yaw = quat_get_yaw(&enemy->base.transform.rotation);
		float difference = enemy->Turret.target_yaw - current_yaw;
		if(fabsf(difference) > epsilon)
		{
			float yaw = enemy->Turret.turn_speed_current * 1.f * fixed_dt;
			if(current_yaw > enemy->Turret.target_yaw)
				yaw *= -1.f;

			transform_rotate(enemy, &yaw_axis, yaw, TS_LOCAL);
		}
	}

	/* Movement */
	if(enemy->Turret.pulsate)
	{
		float ticks = (float)platform_ticks_get();
		vec3 translation = { 0.f };
		vec3_assign(&translation, &enemy->mesh->base.transform.position);
		translation.y += sinf(TO_RADIANS(ticks * enemy->Turret.pulsate_speed_scale)) * enemy->Turret.pulsate_height * fixed_dt ;
		transform_set_position(enemy->mesh, &translation);
	}
}

void enemy_update_ai_turret(struct Enemy* enemy, struct Game_State* game_state, float dt)
{
	struct Scene* scene = game_state->scene;

	struct Ray turret_ray;
	transform_get_absolute_position(enemy->mesh, &turret_ray.origin);
	transform_get_forward(enemy, &turret_ray.direction);

	//quat rot = { 0.f, 0.f, 0.f, 1.f };
	//quat_assign(&rot, &enemy->base.transform.rotation);
	//quat_axis_angle(&rot, &UNIT_X, -90);
	//vec4 color = { 0.f, 0.f, 1.f, 1.f };
	//im_arc(enemy->Turret.vision_range, -enemy->Turret.max_yaw, enemy->Turret.max_yaw, 32, false, turret_ray.origin, rot, color, 5);
	switch(enemy->current_state)
	{
	case TURRET_DEFAULT:
	{
		im_ray(&turret_ray, enemy->Turret.vision_range, enemy->Turret.color_default, 4);
		struct Entity* player = scene_ray_intersect_closest(scene, &turret_ray, ERM_PLAYER);
		if(player)
		{
			float distance = scene_entity_distance(scene, player, enemy);
			if(distance <= enemy->Turret.vision_range)
			{
				enemy_state_set_turret(enemy, TURRET_ALERT);
			}
		}
	}
	break;
	case TURRET_ALERT:
	{
		enemy->Turret.time_elapsed_since_alert += dt;
		if(enemy->Turret.time_elapsed_since_alert >= enemy->Turret.alert_cooldown)
		{
			enemy_state_set_turret(enemy, TURRET_DEFAULT);
			break;
		}
		im_ray(&turret_ray, enemy->Turret.vision_range, enemy->Turret.color_alert, 4);
		struct Entity* player = scene_ray_intersect_closest(scene, &turret_ray, ERM_PLAYER);
		if(player)
		{
			float distance = scene_entity_distance(scene, player, enemy);
			if(distance <= enemy->Turret.vision_range)
			{
				enemy_state_set_turret(enemy, TURRET_ATTACK);
			}
		}
	}
	break;
	case TURRET_ACQUIRE_TARGET:
	{
		struct Entity* player = scene_ray_intersect_closest(scene, &turret_ray, ERM_PLAYER);
		if(player)
		{
			float distance = scene_entity_distance(scene, &scene->player, enemy);
			if(distance <= enemy->Turret.vision_range)
			{
				enemy_state_set_turret(enemy, TURRET_ATTACK);
				break;
			}
		}

		float distance = scene_entity_distance(scene, &scene->player, enemy);
		if(distance <= enemy->Turret.vision_range)
		{
			vec3 player_pos = { 0.f };
			vec3 dir_to_player = { 0.f };
			transform_get_absolute_position(&scene->player, &player_pos);
			vec3_sub(&dir_to_player, &player_pos, &turret_ray.origin);
			vec3_norm(&dir_to_player, &dir_to_player);
			im_ray_origin_dir(turret_ray.origin, dir_to_player, 10.f, (vec4) { 0.f, 1.f, 0.f, 1.f }, 5);
			float yaw_required_to_face_player = floorf(vec3_signed_angle(&dir_to_player, &turret_ray.direction, &UNIT_Y));
			float current_yaw = quat_get_yaw(&enemy->base.transform.rotation);
			float new_target_yaw = current_yaw - yaw_required_to_face_player;
			if(fabsf(floorf(new_target_yaw)) > enemy->Turret.max_yaw)
			{
				enemy_state_set_turret(enemy, TURRET_ALERT);
			}
			else
			{
				float difference = fabsf(enemy->Turret.target_yaw - new_target_yaw);
				if(difference > 1.f)
					enemy->Turret.target_yaw = new_target_yaw;
			}
		}
		else
		{
			enemy_state_set_turret(enemy, TURRET_ALERT);
		}
	}
	break;
	case TURRET_ATTACK:
	{
		enemy->Turret.time_elapsed_since_attack += dt;
		if(enemy->Turret.time_elapsed_since_attack >= enemy->Turret.attack_cooldown)
		{
			im_ray(&turret_ray, enemy->Turret.vision_range, enemy->Turret.color_attack, 4);
			struct Entity* player = scene_ray_intersect_closest(scene, &turret_ray, ERM_PLAYER);
			if(player)
			{
				float distance = scene_entity_distance(scene, player, enemy);
				if(distance <= enemy->Turret.vision_range)
				{
					enemy->Turret.time_elapsed_since_attack = 0.f;
					sound_source_play(game_state->sound, enemy->weapon_sound);
				}
				else
				{
					enemy_state_set_turret(enemy, TURRET_ACQUIRE_TARGET);
				}
			}
			else
			{
				enemy_state_set_turret(enemy, TURRET_ACQUIRE_TARGET);
			}
		}
	}
	break;
	}

}

void enemy_state_set_turret(struct Enemy* enemy, int state)
{
	assert(state >= 0 && state < TURRET_STATE_MAX);
	struct Model* model = &enemy->mesh->model;
	enemy->current_state = state;

	switch(enemy->current_state)
	{
	case TURRET_DEFAULT:
	{
		vec4_assign(&model->material_params[MMP_DIFFUSE_COL].val_vec4, &enemy->Turret.color_default);
		sound_source_play(game_state_get()->sound, enemy->ambient_sound);
		enemy->Turret.time_elapsed_since_alert = 0.f;
		enemy->Turret.time_elapsed_since_attack = 0.f;
		enemy->Turret.pulsate = true;
		enemy->Turret.scan = false;
		enemy->Turret.target_yaw = 0.f;
		enemy->Turret.turn_speed_current = enemy->Turret.turn_speed_default;
		vec3 default_position = { 0.f };
		transform_set_position(enemy->mesh, &default_position);
	}
	break;
	case TURRET_ALERT:
	{
		enemy->Turret.pulsate = false;
		enemy->Turret.turn_speed_current = enemy->Turret.turn_speed_default;
		enemy->Turret.scan = true;
		enemy->Turret.target_yaw = enemy->Turret.yaw_direction_positive ? enemy->Turret.max_yaw : -enemy->Turret.max_yaw;
		vec4_assign(&model->material_params[MMP_DIFFUSE_COL].val_vec4, &enemy->Turret.color_alert);
		enemy->Turret.time_elapsed_since_alert = 0.f;
	}
	break;
	case TURRET_ACQUIRE_TARGET:
	{
		vec4_assign(&model->material_params[MMP_DIFFUSE_COL].val_vec4, &enemy->Turret.color_attack);
		enemy->Turret.scan = false;
		enemy->Turret.turn_speed_current = enemy->Turret.turn_speed_when_targetting;
	}
	break;
	case TURRET_ATTACK:
	{
		enemy->Turret.pulsate = false;
		enemy->Turret.scan = false;
		float current_yaw = quat_get_yaw(&enemy->base.transform.rotation);
		enemy->Turret.target_yaw = current_yaw;
		vec4_assign(&model->material_params[MMP_DIFFUSE_COL].val_vec4, &enemy->Turret.color_attack);
		enemy->Turret.time_elapsed_since_attack = enemy->Turret.attack_cooldown;
	}
	break;
	}
}
