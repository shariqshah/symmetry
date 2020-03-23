#include "door.h"
#include "entity.h"
#include "scene.h"
#include "game.h"
#include "transform.h"
#include "trigger.h"
#include "event.h"
#include "sound_source.h"
#include "../common/log.h"
#include "../common/parser.h"
#include "../common/hashmap.h"

static void door_on_scene_loaded(struct Event* event, void* door_ptr);
static void door_on_trigger(struct Event* event, void* door_ptr);

void door_init(struct Door* door, int mask)
{
	struct Game_State* game_state = game_state_get();
	struct Event_Manager* event_manager = game_state->event_manager;

	door->base.type      = ET_DOOR;
	door->mask           = mask;
	door->speed          = 20.f;
	door->open_position  = -5.f;
	door->close_position = 0.f;
	door->state          = DOOR_CLOSED;

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

	new_door = scene_door_create(scene, name, parent_entity, DOOR_KEY_MASK_NONE);
	if(hashmap_value_exists(object->data, "door_speed"))          new_door->speed          = hashmap_float_get(object->data, "door_speed");
	if(hashmap_value_exists(object->data, "door_state"))          new_door->state          = hashmap_int_get(object->data, "door_state");
	if(hashmap_value_exists(object->data, "door_mask"))           new_door->mask           = hashmap_int_get(object->data, "door_mask");
	if(hashmap_value_exists(object->data, "door_open_position"))  new_door->open_position  = hashmap_float_get(object->data, "door_open_position");
	if(hashmap_value_exists(object->data, "door_close_position")) new_door->close_position = hashmap_float_get(object->data, "door_close_position");

	return new_door;

}

void door_write(struct Door* door, struct Hashmap* entity_data)
{
	hashmap_int_set(entity_data, "door_state", door->state);
	hashmap_int_set(entity_data, "door_mask", door->mask);
	hashmap_float_set(entity_data, "door_speed", door->speed);
	hashmap_float_set(entity_data, "door_open_position", door->open_position);
	hashmap_float_set(entity_data, "door_close_position", door->close_position);
}

void door_update(struct Door* door, struct Scene* scene, float dt)
{
	struct Game_State* game_state = game_state_get();
	switch(door->state)
	{
	case DOOR_CLOSED:
		if(door->trigger->triggered)
		{
			door->state = DOOR_OPENING;
			sound_source_buffer_set(game_state->sound, door->sound, "sounds/door_open.wav", ST_WAV);
			sound_source_play(game_state->sound, door->sound);
		}
		break;
	case DOOR_OPEN:
		if(!door->trigger->triggered)
		{
			door->state = DOOR_CLOSING;
			sound_source_buffer_set(game_state->sound, door->sound, "sounds/door_close.wav", ST_WAV);
			sound_source_play(game_state->sound, door->sound);
		}
		break;
	case DOOR_OPENING:
		if(door->mesh->base.transform.position.x >= door->open_position)
			transform_translate(door->mesh, &(vec3) { door->speed* dt * -1.f, 0.f, 0.f }, TS_LOCAL);
		else
			door->state = DOOR_OPEN;
		break;
	case DOOR_CLOSING:
		if(door->mesh->base.transform.position.x <= door->close_position)
			transform_translate(door->mesh, &(vec3) { door->speed* dt * 1.f, 0.f, 0.f }, TS_LOCAL);
		else
			door->state = DOOR_CLOSED;
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

void door_on_trigger(struct Event* event, void* door_ptr)
{
	struct Game_State* game_state = game_state_get();
	struct Door* door = (struct Door*)door_ptr;
	//log_message("Trigger %s triggered for door %s", door->trigger->base.name, door->base.name);
	switch(door->state)
	{
	case DOOR_CLOSED:
		break;
	case DOOR_OPEN:
		break;
	case DOOR_OPENING:
	case DOOR_CLOSING:
		break;
	}
}
