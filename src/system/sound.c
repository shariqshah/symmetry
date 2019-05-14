#include "sound.h"
#include "../common/log.h"
#include "../common/array.h"
#include "../common/hashmap.h"
#include "../common/variant.h"
#include "../common/string_utils.h"

#include "../game/entity.h"
#include "../game/transform.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <soloud_c.h>

bool sound_init(struct Sound* sound)
{
	sound->soloud_context = Soloud_create();

	if(!sound->soloud_context)
	{
		log_error("sound:init", "Failed to create sound context");
		return false;
	}

    Soloud_initEx(sound->soloud_context, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_SDL2, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);
	Soloud_setGlobalVolume(sound->soloud_context, 4);

	Soloud_set3dListenerParameters(sound->soloud_context,
								   0.f, 0.f,  0.f,  // Position
								   0.f, 0.f, -1.f,  // At
								   0.f, 1.f,  0.f); // Up
	sound->listener = NULL;

	log_message("Sound initialized with %s", Soloud_getBackendString(sound->soloud_context));

	for(int i = 0; i < MAX_SOUND_BUFFERS; i++)
	{
		sound->sound_buffers[i].type = ST_NONE;
		memset(sound->sound_buffers[i].filename, '\0', MAX_FILENAME_LEN);
		sound->sound_buffers[i].wav = NULL;
	}
	return true;
}

void sound_listener_update(struct Sound* sound)
{	
	vec3 position = { 0.f, 0.f,  0.f };
	vec3 at       = { 0.f, 0.f, -1.f };
	vec3 up       = { 0.f, 1.f,  0.f };
	if(sound->listener)
	{
		transform_get_absolute_position(sound->listener, &position);
		transform_get_absolute_forward(sound->listener, &at);
		transform_get_absolute_up(sound->listener, &up);
	}
	
	Soloud_set3dListenerParameters(sound->soloud_context,
								   position.x, position.y, position.z,  // Position
								   at.x, at.y, at.z,  // At
								   up.x,  up.y,  up.z); // Up
}

void sound_listener_set(struct Sound* sound, struct Entity* listener)
{
	sound->listener = listener;
}

void sound_master_volume_set(struct Sound* sound, float volume)
{
	if(volume < 0.f) volume = 0.f;
	sound->master_volume = volume;
	Soloud_setGlobalVolume(sound->soloud_context, sound->master_volume);
}

void sound_update_3d(struct Sound* sound)
{
	sound_listener_update(sound);
	Soloud_update3dAudio(sound->soloud_context);
}

void sound_cleanup(struct Sound* sound)
{
	for(int i = 0; i < MAX_SOUND_BUFFERS; i++ )
	{
		struct Sound_Source_Buffer* source = &sound->sound_buffers[i];
		if(source->type != ST_NONE)
			sound_source_destroy(sound, source);

	}

	Soloud_deinit(sound->soloud_context);
	Soloud_destroy(sound->soloud_context);

	sound->master_volume = 0.f;
	sound->soloud_context = NULL;
}

void sound_source_instance_destroy(struct Sound* sound, uint source_instance)
{
	Soloud_stop(sound->soloud_context, source_instance);
}

void sound_source_instance_update_position(struct Sound* sound, uint source_instance, vec3 abs_pos)
{
	Soloud_set3dSourceParameters(sound->soloud_context, source_instance, abs_pos.x, abs_pos.y, abs_pos.z);
}

uint sound_source_instance_create(struct Sound* sound, struct Sound_Source_Buffer* source, bool is3d)
{
	assert(source);
	uint source_instance = 0;
	if(is3d)
	{
		source_instance = Soloud_play3dEx(sound->soloud_context,
								 source->type == ST_WAV ? source->wav : source->wavstream,
								 0.f, 0.f, 0.f,
								 0.f, 0.f, 0.f,
								 1.f,
								 true,
								 0);
	}
	else
	{
		source_instance = Soloud_playEx(sound->soloud_context, source->type == ST_WAV ? source->wav : source->wavstream, 1.f, 0.0f, true, 0);
	}
	return source_instance;
}

void sound_source_instance_volume_set(struct Sound* sound, uint source_instance, float volume)
{
	if(volume < 0.f) volume = 0.f;
	Soloud_setVolume(sound->soloud_context, source_instance, volume);
}

void sound_source_instance_loop_set(struct Sound* sound, uint source_instance, bool loop)
{
	Soloud_setLooping(sound->soloud_context, source_instance, loop);
}

void sound_source_instance_play(struct Sound* sound, uint source_instance)
{
	Soloud_setPause(sound->soloud_context, source_instance, false);
}

void sound_source_instance_pause(struct Sound* sound, uint source_instance)
{
	Soloud_setPause(sound->soloud_context, source_instance, true);
}

void sound_source_instance_rewind(struct Sound* sound, uint source_instance)
{
	Soloud_seek(sound->soloud_context, source_instance, 0.0);
}

void sound_source_instance_stop(struct Sound* sound, uint source_instance)
{
	Soloud_stop(sound->soloud_context, source_instance);
}

void sound_source_instance_min_max_distance_set(struct Sound* sound, uint source_instance, float min_distance, float max_distance)
{
	Soloud_set3dSourceMinMaxDistance(sound->soloud_context, source_instance, min_distance, max_distance);
}

void sound_source_instance_attenuation_set(struct Sound* sound, uint source_instance, int attenuation_type, float rolloff_factor)
{
	Soloud_set3dSourceAttenuation(sound->soloud_context, source_instance, attenuation_type, rolloff_factor);
}

float sound_source_instance_volume_get(struct Sound* sound, uint source_instance)
{
	return Soloud_getVolume(sound->soloud_context, source_instance);
}

bool sound_source_instance_loop_get(struct Sound* sound, uint source_instance)
{
	return Soloud_getLooping(sound->soloud_context, source_instance);
}

bool sound_source_instance_is_paused(struct Sound* sound, uint source_instance)
{
	return Soloud_getPause(sound->soloud_context, source_instance);
}

struct Sound_Source_Buffer* sound_source_create(struct Sound* sound, const char* filename, int type)
{
	if(!filename) 
		return NULL;

	struct Sound_Source_Buffer* source = sound_source_get(sound, filename);

	// See if we've already loaded this file otherwise, get the next empty slot.
	// If we can't find an empty slot, print error and return NULL
	if(source)
	{
		return source;
	}
	else
	{
		for(int i = 0; i < MAX_SOUND_BUFFERS; i++)
		{
			if(sound->sound_buffers[i].type == ST_NONE)
			{
				source = &sound->sound_buffers[i];
				break;
			}
		}

		if(!source)
		{
			log_error("sound:source_create", "Could not find empty sound source slot for '%s'", filename);
			return source;
		}
	}

	long size = 0L;
	unsigned char* memory = io_file_read(DIRT_INSTALL, filename, "rb", &size);
	if(!memory)
	{
		log_error("sound:source_create", "Failed to read file");
		return source;
	}

	switch(type)
	{
	case ST_WAV:
	{
		Wav* wave = Wav_create();
		int rc = Wav_loadMem(wave, memory, (uint)size);
		if(rc != 0)
		{
			log_error("sound:source_create", "Failed to load %s, Soloud: %s", filename, Soloud_getErrorString(sound->soloud_context, rc));
			free(memory);
			return 0;
		}
		source->type = ST_WAV;
		source->wav = wave;
	}
	break;
	case ST_WAV_STREAM:
	{
		WavStream* wave_stream = WavStream_create();
		int rc = WavStream_loadMem(wave_stream, memory, (uint)size);
		if(rc != 0)
		{
			log_error("sound:source_create", "Failed to load %s, Soloud: %s", filename, Soloud_getErrorString(sound->soloud_context, rc));
			free(memory);
			return 0;
		}
		source->type = ST_WAV_STREAM;
		source->wavstream = wave_stream;
	}
	break;
	default: log_error("sound:source_create", "Invalid source type %d", type); break;
	}

	strncpy(source->filename, filename, MAX_FILENAME_LEN);
	return source;
}

struct Sound_Source_Buffer* sound_source_get(struct Sound* sound, const char* name)
{
	struct Sound_Source_Buffer* source = NULL;
	for(int i = 0; i < MAX_SOUND_BUFFERS; i++)
	{
		if(sound->sound_buffers[i].type == ST_NONE)
			continue;

		if(strncmp(name, sound->sound_buffers[i].filename, MAX_FILENAME_LEN) == 0)
		{
			source = &sound->sound_buffers[i];
			break;
		}
	}
	return source;
}

void sound_source_destroy(struct Sound* sound, struct Sound_Source_Buffer* source)
{
	if(source)
	{
		sound_source_stop_all(sound, source);
		switch(source->type)
		{
		case ST_WAV: Wav_destroy(source->wav); source->wav = NULL;  break;
		case ST_WAV_STREAM: WavStream_destroy(source->wavstream); source->wavstream = NULL; break;
		}
		source->type = ST_NONE;
		memset(source->filename, '\0', MAX_FILENAME_LEN);
	}
}

void sound_source_volume_set(struct Sound* sound, struct Sound_Source_Buffer* source, float volume)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_setVolume(source->wav, volume); break;
	case ST_WAV_STREAM: WavStream_setVolume(source->wavstream, volume); break;
	}
}

void sound_source_loop_set(struct Sound* sound, struct Sound_Source_Buffer* source, bool loop)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_setLooping(source->wav, loop); break;
	case ST_WAV_STREAM: WavStream_setLooping(source->wavstream, loop); break;
	}
}

void sound_source_stop_all(struct Sound* sound, struct Sound_Source_Buffer* source)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Soloud_stopAudioSource(sound->soloud_context, source->wav); break;
	case ST_WAV_STREAM: Soloud_stopAudioSource(sound->soloud_context, source->wavstream); break;
	}
}

void sound_source_min_max_distance_set(struct Sound* sound, struct Sound_Source_Buffer* source, float min_distance, float max_distance)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_set3dMinMaxDistance(source->wav, min_distance, max_distance); break;
	case ST_WAV_STREAM: WavStream_set3dMinMaxDistance(source->wavstream, min_distance, max_distance); break;
	}
}
