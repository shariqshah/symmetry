#include "sound_source.h"
#include "entity.h"
#include "../system/sound.h"
#include "transform.h"
#include "../common/log.h"

void sound_source_validate_instance(struct Sound* sound, struct Sound_Source* entity)
{
	if(!sound_source_instance_is_valid(sound, entity->source_instance))
	{
		entity->source_instance = sound_source_instance_create(sound, entity->source_buffer, true);
		sound_source_apply_params_to_instance(sound, entity);
	}
}

void sound_source_apply_params_to_instance(struct Sound* sound, struct Sound_Source* entity)
{
	// This function assumes that the handle to the sound source is valid 
	vec3 abs_position = { 0.f };
	transform_get_absolute_position(entity, &abs_position);
	sound_source_instance_update_position(sound, entity->source_instance, abs_position);
	sound_source_instance_loop_set(sound, entity->source_instance, entity->loop);
	sound_source_instance_min_max_distance_set(sound, entity->source_instance, entity->min_distance, entity->max_distance);
	sound_source_instance_attenuation_set(sound, entity->source_instance, entity->attenuation_type, entity->rolloff_factor);
	sound_source_instance_volume_set(sound, entity->source_instance, entity->volume);
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

void sound_source_update_position(struct Sound* sound, struct Sound_Source* entity)
{
	if(sound_source_instance_is_valid(sound, entity->source_instance))
	{
		vec3 abs_position = { 0.f };
		transform_get_absolute_position(entity, &abs_position);
		sound_source_instance_update_position(sound, entity->source_instance, abs_position);
	}
}

void sound_source_buffer_set(struct Sound* sound, struct Sound_Source* entity, const char* filename, int type)
{
	struct Sound_Source_Buffer* new_buffer = sound_source_buffer_create(sound, filename, type);
	if(new_buffer)
	{
		sound_source_instance_destroy(sound, entity->source_instance);
		if(entity->source_buffer)
			sound_source_buffer_destroy(sound, entity->source_buffer);

		entity->source_buffer = new_buffer;
		entity->type = type;
		entity->source_instance = sound_source_instance_create(sound, entity->source_buffer, true);
		sound_source_apply_params_to_instance(sound, entity);
	}
	else
	{
		log_error("sound_source:buffer_set", "Failed to set buffer for %s", entity->base.name);
	}
}

bool sound_source_is_paused(struct Sound* sound, struct Sound_Source* entity)
{
	sound_source_validate_instance(sound, entity);
	return sound_source_instance_is_paused(sound, entity->source_instance);
}
