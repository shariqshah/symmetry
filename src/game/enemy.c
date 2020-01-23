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

#include <string.h>
#include <math.h>

static void enemy_on_scene_loaded(struct Event* event, void* enemy_ptr);
static void enemy_update_physics_turret(struct Enemy* enemy, struct Game_State* game_state, float fixed_dt);
static void enemy_update_ai_turret(struct Enemy* enemy, struct Game_State* game_state, float dt);

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
		enemy->health = 100;
		enemy->damage = 10;
		enemy->Turret.turn_direction_positive = true;
		enemy->Turret.turn_speed = 10.f;
		enemy->Turret.max_turn_angle = 60.f;
		break;
	}
	default:
		log_error("enemy:init", "Unsupported Enemy Type");
		break;
	}

	struct Event_Manager* event_manager = game_state->event_manager;
	event_manager_subscribe_with_object(event_manager, EVT_SCENE_LOADED, &enemy_on_scene_loaded, (void*)enemy);
}

void enemy_weapon_sound_set(struct Enemy* enemy, const char* sound_filename, int type)
{
	sound_source_buffer_set(game_state_get()->sound, sound_filename, type);
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
			if(hashmap_value_exists(object->data, "turn_speed")) new_enemy->Turret.turn_speed = hashmap_float_get(object->data, "turn_speed");
			if(hashmap_value_exists(object->data, "max_turn_angle")) new_enemy->Turret.max_turn_angle = hashmap_float_get(object->data, "max_turn_angle");
			if(hashmap_value_exists(object->data, "turn_direction_positive")) new_enemy->Turret.turn_direction_positive = hashmap_bool_get(object->data, "turn_direction_positive");
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
		hashmap_float_set(entity_data, "turn_speed", enemy->Turret.turn_speed);
		hashmap_float_set(entity_data, "max_turn_angle", enemy->Turret.max_turn_angle);
		hashmap_bool_set(entity_data, "turn_direction_positive", enemy->Turret.turn_direction_positive);
	}
	break;
	}
}

void enemy_on_scene_loaded(struct Event* event, void* enemy_ptr)
{
	struct Enemy* enemy = (struct Enemy*)enemy_ptr;

	// Assign pointers to static_mesh and sound_source child entities
	for(int i = 0; i < array_len(enemy->base.transform.children); i++)
	{
		struct Entity* child = enemy->base.transform.children[i];
		if(child->type == ET_STATIC_MESH)
			enemy->mesh = (struct Static_Mesh*)child;
		else if(child->type == ET_SOUND_SOURCE)
			enemy->weapon_sound = (struct Sound_Source*)child;
	}

	if(enemy->mesh) enemy->mesh->base.flags |= EF_TRANSIENT;
	if(enemy->weapon_sound) enemy->weapon_sound->base.flags |= EF_TRANSIENT;

	// Do other post-scene-load initialization stuff per enemy type here
}

void enemy_update_physics_turret(struct Enemy* enemy, struct Game_State* game_state, float dt)
{
	/* Turning/Rotation */
	static vec3 turn_axis = { 0.f, 1.f, 0.f };
	float current_yaw = quat_get_yaw(&enemy->base.transform.rotation);

	float yaw = enemy->Turret.turn_speed * 1.f * dt;
	if(!enemy->Turret.turn_direction_positive)
		yaw *= -1.f;

	current_yaw += yaw;
	if(current_yaw >= enemy->Turret.max_turn_angle)
	{
		yaw = 0.f;
		enemy->Turret.turn_direction_positive = false;
	}
	else if(current_yaw <= -enemy->Turret.max_turn_angle)
	{
		yaw = 0.f;
		enemy->Turret.turn_direction_positive = true;
	}

	if(yaw != 0.f)
		transform_rotate(enemy, &turn_axis, yaw, TS_LOCAL);

	/* Movement */
	float ticks = (float)platform_ticks_get();
	float pulsate_speed = 50.f;
	vec3 translation = { 0.f };
	transform_get_absolute_position(enemy, &translation);
	translation.y += sinf(TO_RADIANS(ticks)) * dt * pulsate_speed;
	debug_vars_show_float("T", sinf(TO_RADIANS(ticks * dt * pulsate_speed)));
	transform_set_position(enemy, &translation);
	//transform_translate(enemy, &translation, TS_LOCAL);
}

void enemy_update_ai_turret(struct Enemy* enemy, struct Game_State* game_state, float dt)
{

}
