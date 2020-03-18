#include "door.h"
#include "entity.h"
#include "scene.h"
#include "game.h"
#include "transform.h"
#include "trigger.h"
#include "event.h"
#include "../common/log.h"
#include "../common/parser.h"
#include "../common/hashmap.h"

static void door_on_scene_loaded(struct Event* event, void* door_ptr);
static void door_on_trigger(struct Event* event, void* trigger_ptr);

void door_init(struct Door* door, int mask)
{
	struct Game_State* game_state = game_state_get();
	struct Event_Manager* event_manager = game_state->event_manager;

	door->base.type = ET_DOOR;
	door->mask = mask;
	door->speed = 20.f;
	door->state = DOOR_CLOSED;

	event_manager_subscribe_with_object(event_manager, EVT_SCENE_LOADED, &door_on_scene_loaded, (void*)door);
}

void door_reset(struct Door* door)
{
	door->state = -1;
	door->speed = 0.f;

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_unsubscribe_with_object(event_manager, EVT_TRIGGER, &door_on_trigger, (void*)door);
	event_manager_unsubscribe_with_object(event_manager, EVT_SCENE_LOADED, &door_on_scene_loaded, (void*)door);
}

struct Door* door_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity)
{
	struct Door* new_door = NULL;
	struct Scene* scene = game_state_get()->scene;

	new_door = scene_door_create(scene, name, parent_entity, DOOR_KEY_NONE);
	if(hashmap_value_exists(object->data, "door_speed")) new_door->speed = hashmap_float_get(object->data, "door_speed");
	if(hashmap_value_exists(object->data, "door_state")) new_door->state = hashmap_int_get(object->data, "door_state");
	if(hashmap_value_exists(object->data, "door_mask"))  new_door->mask  = hashmap_int_get(object->data, "door_mask");

	return new_door;

}

void door_write(struct Door* door, struct Hashmap* entity_data)
{
	hashmap_int_set(entity_data, "door_state", door->state);
	hashmap_int_set(entity_data, "door_mask", door->mask);
	hashmap_float_set(entity_data, "door_speed", door->speed);
}

void door_update(struct Door* door, struct Scene* scene, float dt)
{
	switch(door->state)
	{
	case DOOR_CLOSED:
	case DOOR_OPEN:
		break;
	case DOOR_OPENING:
		break;
	case DOOR_CLOSING:
		break;
	}
}

void door_on_scene_loaded(struct Event* event, void* door_ptr)
{
	struct Door* door              = (struct Door*)door_ptr;
	struct Entity* door_mesh[1]    = { NULL };
	struct Entity* door_sound[1]   = { NULL };
	struct Entity* door_trigger[1] = { NULL };

	if(entity_get_num_children_of_type(door, ET_STATIC_MESH, &door_mesh, 1) == 1)
		door->mesh = door_mesh[0];
	else
		log_error("door:on_scene_load", "Could not find mesh entity for door %s", door->base.name);

	if(entity_get_num_children_of_type(door, ET_SOUND_SOURCE, &door_sound, 1) == 1)
		door->sound = door_sound[0];
	else
		log_error("door:on_scene_load", "Could not find sound entity for door %s", door->base.name);

	if(entity_get_num_children_of_type(door, ET_TRIGGER, &door_trigger, 1) == 1)
	{
		door->trigger = door_trigger[0];
		struct Event_Manager* event_manager = game_state_get()->event_manager;
		event_manager_subscribe_with_object(event_manager, EVT_TRIGGER, &door_on_trigger, (void*)door);
	}
	else
	{
		log_error("door:on_scene_load", "Could not find trigger entity for door %s", door->base.name);
	}

}

void door_on_trigger(struct Event* event, void* trigger_ptr)
{

}
