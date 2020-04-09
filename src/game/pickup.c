#include "pickup.h"
#include "entity.h"
#include "game.h"
#include "event.h"
#include "../common/hashmap.h"
#include "../common/parser.h"
#include "scene.h"
#include "../common/log.h"
#include "material.h"
#include "door.h"
#include "transform.h"
#include "sound_source.h"

static void pickup_on_scene_loaded(struct Event* event, void* pickup_ptr);
static void pickup_on_trigger(struct Event* event, void* pickup_ptr, void* trigger_ptr);

void pickup_init(struct Pickup* pickup, int type)
{
	pickup->base.type  = ET_PICKUP;
	pickup->type       = type;
	pickup->spin_speed = 5.f;
	pickup->picked_up  = false;
	switch(type)
	{
	case PICKUP_KEY:    pickup->key_type = DOOR_KEY_MASK_NONE; break;
	case PICKUP_HEALTH: pickup->health   = 0; break;
	}

	struct Game_State* game_state = game_state_get();
	event_manager_subscribe_with_subscriber(game_state->event_manager, EVT_SCENE_LOADED, &pickup_on_scene_loaded, (void*)pickup);
}

void pickup_reset(struct Pickup* pickup)
{
	struct Game_State* game_state = game_state_get();
	event_manager_unsubscribe_with_subscriber(game_state->event_manager, EVT_SCENE_LOADED, &pickup_on_scene_loaded, (void*)pickup);
	event_manager_unsubscribe_with_subscriber_sender(game_state->event_manager, EVT_TRIGGER, &pickup_on_trigger, (void*)pickup, (void*)pickup->trigger);
}

struct Pickup* pickup_read(struct Parser_Object* parser_object, const char* name, struct Entity* parent_entity)
{
	struct Pickup* new_pickup = NULL;
	struct Scene* scene = game_state_get()->scene;

	new_pickup = scene_pickup_create(scene, name, parent_entity, PICKUP_HEALTH);
	if(hashmap_value_exists(parser_object->data, "pickup_type")) new_pickup->type = hashmap_int_get(parser_object->data, "pickup_type");
	if(hashmap_value_exists(parser_object->data, "pickup_spin_speed")) new_pickup->spin_speed = hashmap_float_get(parser_object->data, "pickup_spin_speed");
	switch(new_pickup->type)
	{
	case PICKUP_KEY:    if(hashmap_value_exists(parser_object->data, "pickup_key_type")) new_pickup->key_type = hashmap_int_get(parser_object->data, "pickup_key_type"); break;
	case PICKUP_HEALTH: if(hashmap_value_exists(parser_object->data, "pickup_health"))   new_pickup->health   = hashmap_int_get(parser_object->data, "pickup_health");   break;
	}
	return new_pickup;
}

void pickup_write(struct Pickup* pickup, struct Hashmap* pickup_data)
{
	hashmap_int_set(pickup_data, "pickup_type", pickup->type);
	hashmap_float_set(pickup_data, "pickup_spin_speed", pickup->spin_speed);

	switch(pickup->type)
	{
	case PICKUP_KEY: hashmap_int_set(pickup_data, "pickup_key_type", pickup->key_type); break;
	case PICKUP_HEALTH: hashmap_int_set(pickup_data, "pickup_health", pickup->health); break;
	}
}

void pickup_on_scene_loaded(struct Event* event, void* pickup_ptr)
{
	struct Pickup* pickup = (struct Pickup*)pickup_ptr;
	struct Trigger* pickup_trigger[1] = { NULL };
	struct Static_Mesh* pickup_mesh[1] = { NULL };
	struct Sound_Source* sound_source[1] = { NULL };

	if(entity_get_num_children_of_type(pickup, ET_TRIGGER, &pickup_trigger, 1) == 1)
	{
		pickup->trigger = pickup_trigger[0];
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_subscribe_with_subscriber_sender(event_manager, EVT_TRIGGER, &pickup_on_trigger, (void*)pickup, pickup->trigger);
	}
	else
	{
		log_error("pickup:on_scene_loaded", "Could not find trigger for pickup %s", pickup->base.name);
	}

	if(entity_get_num_children_of_type(pickup, ET_STATIC_MESH, &pickup_mesh, 1) == 1)
	{
		pickup->mesh = pickup_mesh[0];
		if(pickup->type == PICKUP_KEY)
		{
			switch(pickup->key_type)
			{
			case DOOR_KEY_MASK_RED:   vec4_assign(&pickup->mesh->model.material_params[MMP_DIFFUSE_COL].val_vec4, &KEY_INDICATOR_COLOR_RED);   break;
			case DOOR_KEY_MASK_GREEN: vec4_assign(&pickup->mesh->model.material_params[MMP_DIFFUSE_COL].val_vec4, &KEY_INDICATOR_COLOR_GREEN); break;
			case DOOR_KEY_MASK_BLUE:  vec4_assign(&pickup->mesh->model.material_params[MMP_DIFFUSE_COL].val_vec4, &KEY_INDICATOR_COLOR_BLUE);  break;
			}
		}
	}
	else
	{
		log_error("pickup:on_scene_loaded", "Could not find mesh for pickup %s", pickup->base.name);
	}

	if(entity_get_num_children_of_type(pickup, ET_SOUND_SOURCE, &sound_source, 1) == 1)
		pickup->sound = sound_source[0];
	else
		log_error("pickup:on_scene_loaded", "Could not find mesh for pickup %s", pickup->base.name);
}

void pickup_update(struct Pickup* pickup, float dt)
{
	transform_rotate(pickup->mesh, &UNIT_Y, pickup->spin_speed * dt, TS_WORLD);
	if(pickup->picked_up &&  sound_source_is_paused(game_state_get()->sound, pickup->sound))
		scene_pickup_remove(game_state_get()->scene, pickup);
}

void pickup_on_trigger(struct Event* event, void* pickup_ptr, void* trigger_ptr)
{
	struct Pickup* pickup = (struct Pickup*) pickup_ptr;
	if(!pickup->picked_up)
	{
		switch(event->trigger.triggering_entity->type)
		{
		case ET_PLAYER: player_on_pickup(event->trigger.triggering_entity, pickup); break;
		case ET_ENEMY:
			// Handle this if we add enemies that can move around
			break;
		}

		sound_source_play(game_state_get()->sound, pickup->sound);
		pickup->mesh->base.flags |= EF_SKIP_RENDER; // Hide mesh to ensure effect is instantaneous
		pickup->picked_up = true;
	}
}