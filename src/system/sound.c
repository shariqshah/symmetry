#include "sound.h"
#include "../common/log.h"
#include "../common/array.h"
#include "../common/hashmap.h"
#include "../common/variant.h"
#include "../common/string_utils.h"
#include "file_io.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <soloud_c.h>

struct Sound_Source_Buffer
{
	int type;
	union
	{
		Wav* wav;
		WavStream* wavstream;
	};
};

struct Sound_State
{
	Soloud* soloud_context;
	float   volume;
};

static struct Sound_State sound_state =
{
	.soloud_context  = NULL,
	.volume          = 1.f
};

static struct Hashmap* sound_sources = NULL;

bool sound_init(void)
{
	sound_sources = hashmap_new();

	sound_state.soloud_context = Soloud_create();

	if(!sound_state.soloud_context)
	{
		log_error("sound:init", "Failed to create sound context");
		return false;
	}

    Soloud_initEx(sound_state.soloud_context, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_SDL2, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);
	Soloud_setGlobalVolume(sound_state.soloud_context, 4);

	Soloud_set3dListenerParameters(sound_state.soloud_context,
								   0.f, 0.f,  0.f,  // Position
								   0.f, 0.f, -1.f,  // At
								   0.f, 1.f,  0.f); // Up

	log_message("Sound initialized with %s", Soloud_getBackendString(sound_state.soloud_context));
	return true;
}

void sound_listener_update(float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z)
{	
	Soloud_set3dListenerParameters(sound_state.soloud_context,
								   apos_x, apos_y, apos_z,  // Position
								   afwd_x, afwd_y, afwd_z,  // At
								   aup_x,  aup_y,  aup_z); // Up
}

void sound_volume_set(float volume)
{
	if(volume < 0.f) volume = 0.f;
	sound_state.volume = volume;
	Soloud_setGlobalVolume(sound_state.soloud_context, sound_state.volume);
}

void sound_update_3d(void)
{
	Soloud_update3dAudio(sound_state.soloud_context);
}

void sound_cleanup(void)
{
	char* key = NULL;
	struct Variant* value = NULL;
	HASHMAP_FOREACH(sound_sources, key, value)
	{
		struct Sound_Source_Buffer* source = value->val_voidptr;
		sound_source_stop_all(source);
		switch(source->type)
		{
		case ST_WAV: Wav_destroy(source->wav); break;
		case ST_WAV_STREAM: WavStream_destroy(source->wavstream); break;
		}
		free(source);
	}

	hashmap_free(sound_sources);
	Soloud_deinit(sound_state.soloud_context);
	Soloud_destroy(sound_state.soloud_context);

	sound_state.volume          = 0.f;
	sound_state.soloud_context  = NULL;
}

void sound_source_instance_destroy(uint source_instance)
{
	Soloud_stop(sound_state.soloud_context, source_instance);
}

void sound_source_instance_update_position(uint source_instance, float apos_x, float apos_y, float apos_z)
{
	Soloud_set3dSourceParameters(sound_state.soloud_context, source_instance,	apos_x, apos_y, apos_z);
}

uint sound_source_instance_create(struct Sound_Source_Buffer* source, bool is3d)
{
	assert(source);
	uint source_instance = 0;
	if(is3d)
	{
		source_instance = Soloud_play3dEx(sound_state.soloud_context,
								 source->type == ST_WAV ? source->wav : source->wavstream,
								 0.f, 0.f, 0.f,
								 0.f, 0.f, 0.f,
								 1.f,
								 true,
								 0);
	}
	else
	{
		source_instance = Soloud_playEx(sound_state.soloud_context, source->type == ST_WAV ? source->wav : source->wavstream, 1.f, 0.0f, true, 0);
	}
	return source_instance;
}

void sound_source_instance_volume_set(uint source_instance, float volume)
{
	if(volume < 0.f) volume = 0.f;
	Soloud_setVolume(sound_state.soloud_context, source_instance, volume);
}

void sound_source_instance_loop_set(uint source_instance, bool loop)
{
	Soloud_setLooping(sound_state.soloud_context, source_instance, loop);
}

void sound_source_instance_play(uint source_instance)
{
	Soloud_setPause(sound_state.soloud_context, source_instance, false);
}

void sound_source_instance_pause(uint source_instance)
{
	Soloud_setPause(sound_state.soloud_context, source_instance, true);
}

void sound_source_instance_rewind(uint source_instance)
{
	Soloud_seek(sound_state.soloud_context, source_instance, 0.0);
}

void sound_source_instance_stop(uint source_instance)
{
	Soloud_stop(sound_state.soloud_context, source_instance);
}

void sound_source_instance_min_max_distance_set(uint source_instance, float min_distance, float max_distance)
{
	Soloud_set3dSourceMinMaxDistance(sound_state.soloud_context, source_instance, min_distance, max_distance);
}

void sound_source_instance_attenuation_set(uint source_instance, int attenuation_type, float rolloff_factor)
{
	Soloud_set3dSourceAttenuation(sound_state.soloud_context, source_instance, attenuation_type, rolloff_factor);
}

float sound_source_instance_volume_get(uint source_instance)
{
	return Soloud_getVolume(sound_state.soloud_context, source_instance);
}

bool sound_source_instance_loop_get(uint source_instance)
{
	return Soloud_getLooping(sound_state.soloud_context, source_instance);
}

bool sound_source_instance_is_paused(uint source_instance)
{
	return Soloud_getPause(sound_state.soloud_context, source_instance);
}

struct Sound_Source_Buffer* sound_source_create(const char* filename, int type)
{
	if(!filename) return NULL;

	struct Sound_Source_Buffer* source = NULL;

	//See if we've already loaded this file
	if(hashmap_value_exists(sound_sources, filename))
	{
		source = (struct Sound_Source_Buffer*)hashmap_ptr_get(sound_sources, filename);
		return source;
	}

	long size = 0L;
	unsigned char* memory = io_file_read(DIRT_INSTALL, filename, "rb", &size);

	source = malloc(sizeof(*source));
	if(!source)
	{
		log_error("sound:source_create", "Out of memory!");
		return NULL;
	}

	switch(type)
	{
	case ST_WAV:
	{
		Wav* wave = Wav_create();
		int rc = Wav_loadMem(wave, memory, (uint)size);
		if(rc != 0)
		{
			log_error("sound:source_create", "Failed to load %s, Soloud: %s", filename, Soloud_getErrorString(sound_state.soloud_context, rc));
			free(source);
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
			log_error("sound:source_create", "Failed to load %s, Soloud: %s", filename, Soloud_getErrorString(sound_state.soloud_context, rc));
			free(source);
			return 0;
		}
		source->type = ST_WAV_STREAM;
	}
	break;
	default: log_error("sound:source_create", "Invalid source type %d", type); break;
	}

	hashmap_ptr_set(sound_sources, filename, (void*)source);
	if(memory) free(memory);
	return source;
}

struct Sound_Source_Buffer* sound_source_get(const char* name)
{
	struct Sound_Source_Buffer* source = NULL;
	if(hashmap_value_exists(sound_sources, name))
	{
		source = (struct Sound_Source_Buffer*)hashmap_ptr_get(sound_sources, name);
	}
	return source;
}

void sound_source_destroy(const char* name)
{
	struct Sound_Source_Buffer* source = sound_source_get(name);
	if(source)
	{
		sound_source_stop_all(source);
		switch(source->type)
		{
		case ST_WAV: Wav_destroy(source->wav); break;
		case ST_WAV_STREAM: WavStream_destroy(source->wavstream); break;
		}
		free(source);
	}
	hashmap_value_remove(sound_sources, name);
}

void sound_source_volume_set(struct Sound_Source_Buffer* source, float volume)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_setVolume(source->wav, volume); break;
	case ST_WAV_STREAM: WavStream_setVolume(source->wavstream, volume); break;
	}
}

void sound_source_loop_set(struct Sound_Source_Buffer* source, bool loop)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_setLooping(source->wav, loop); break;
	case ST_WAV_STREAM: WavStream_setLooping(source->wavstream, loop); break;
	}
}

void sound_source_stop_all(struct Sound_Source_Buffer* source)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Soloud_stopAudioSource(sound_state.soloud_context, source->wav); break;
	case ST_WAV_STREAM: Soloud_stopAudioSource(sound_state.soloud_context, source->wavstream); break;
	}
}

void sound_source_min_max_distance_set(struct Sound_Source_Buffer* source, float min_distance, float max_distance)
{
	assert(source);
	switch(source->type)
	{
	case ST_WAV:        Wav_set3dMinMaxDistance(source->wav, min_distance, max_distance); break;
	case ST_WAV_STREAM: WavStream_set3dMinMaxDistance(source->wavstream, min_distance, max_distance); break;
	}
}
