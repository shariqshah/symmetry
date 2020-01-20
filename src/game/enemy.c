#include "enemy.h"
#include "entity.h"
#include "scene.h"
#include "game.h"
#include "sound_source.h"
#include "../common/log.h"
#include "../common/hashmap.h"
#include "../common/parser.h"

#include <string.h>

void enemy_init(struct Enemy* enemy, int type)
{
	struct Game_State* game_state = game_state_get();
	struct Scene* scene = game_state->scene;

	enemy->base.type = ET_ENEMY;
	enemy->type = type;

	char weapon_name_buffer[MAX_ENTITY_NAME_LEN];
	char mesh_name_buffer[MAX_ENTITY_NAME_LEN];
	memset(weapon_name_buffer, '\0', MAX_ENTITY_NAME_LEN);
	memset(mesh_name_buffer, '\0', MAX_ENTITY_NAME_LEN);

	snprintf(weapon_name_buffer, MAX_ENTITY_NAME_LEN, "%s_Weapon_Sound", enemy->base.name);
	snprintf(mesh_name_buffer, MAX_ENTITY_NAME_LEN, "%s_Mesh", enemy->base.name);

	struct Sound_Source* weapon_sound = NULL;
	struct Static_Mesh* mesh = NULL;

	/* Initialization specific to each enemy type */
	switch(enemy->type)
	{
	case ENEMY_TURRET:
	{
		enemy->Turret.turn_speed = 10.f;
		enemy->health = 100;
		enemy->damage = 10;
		weapon_sound = scene_sound_source_create(scene, weapon_name_buffer, enemy, "sounds/bullet_1.wav", ST_WAV, false, false);
		mesh = scene_static_mesh_create(scene, mesh_name_buffer, enemy, "suzanne.symbres", MAT_BLINN);
		break;
	}
	default:
		log_error("enemy:init", "Unsupported Enemy Type");
		break;
	}

	enemy->weapon_sound = weapon_sound ? weapon_sound : NULL;
	if(!weapon_sound)
		log_error("enemy:init", "Failed to add weapon sound for %s", enemy->base.name);

	enemy->mesh = mesh ? mesh : NULL;
	if(!mesh)
		log_error("enemy:init", "Failed to add mesh from file for %s", enemy->base.name);

	if(enemy->mesh) enemy->mesh->base.flags |= EF_TRANSIENT;
	if(enemy->weapon_sound) enemy->weapon_sound->base.flags |= EF_TRANSIENT;
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
	static float enemy_update_interval = 1.f / 2.f;
	static float time_elapsed_since_last_update = 0.f;

	time_elapsed_since_last_update += dt;
	if(time_elapsed_since_last_update < enemy_update_interval)
		return;

	time_elapsed_since_last_update = 0.f;
	struct Game_State* game_state = game_state_get();

	log_message("Enemy_update");
	sound_source_play(game_state->sound, enemy->weapon_sound);
}

void enemy_reset(struct Enemy* enemy)
{
	enemy->type = -1;
	enemy->damage = 0;
	enemy->health = 0;
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
	}
	break;
	}
}
