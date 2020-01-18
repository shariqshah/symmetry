#include "sound_source.h"
#include "entity.h"
#include "../system/sound.h"
#include "transform.h"
#include "../common/log.h"

static void sound_source_validate_instance(struct Sound* sound, struct Sound_Source* entity)
{
	if(!sound_source_instance_is_valid(sound, entity->source_instance))
	{
		entity->source_instance = sound_source_instance_create(sound, entity->source_buffer, true);
		vec3 abs_position = { 0.f };
		transform_get_absolute_position(entity, &abs_position);
		sound_source_instance_update_position(sound, entity->source_instance, abs_position);
	}
}

void sound_source_play(struct Sound* sound, struct Sound_Source* entity)
{
	sound_source_validate_instance(sound, entity);
	sound_source_instance_rewind(sound, entity->source_instance);
	sound_source_instance_play(sound, entity->source_instance);
}

void sound_source_pause(struct Sound* sound, struct Sound_Source* entity)
{
	sound_source_validate_instance(sound, entity);
	sound_source_instance_pause(sound, entity->source_instance);
}

void sound_source_stop(struct Sound* sound, struct Sound_Source* entity)
{
	sound_source_validate_instance(sound, entity);
	sound_source_instance_stop(sound, entity->source_instance);
}

void sound_source_update(struct Sound* sound, struct Sound_Source* entity)
{
	if(sound_source_instance_is_valid(sound, entity->source_instance))
	{
		vec3 abs_position = { 0.f };
		transform_get_absolute_position(entity, &abs_position);
		sound_source_instance_update_position(sound, entity->source_instance, abs_position);
	}
}
