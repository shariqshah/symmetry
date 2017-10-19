#include "sound.h"
#include "../common/log.h"
#include "../common/array.h"
#include "../common/hashmap.h"
#include "../common/variant.h"
#include "../common/string_utils.h"
#include "file_io.h"

#include <assert.h>
#include <stdio.h>

#include <soloud_c.h>

struct Sound_Buffer
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

static struct Hashmap* sound_buffers = NULL;

bool sound_init(void)
{
	sound_buffers = hashmap_new();

	sound_state.soloud_context = Soloud_create();

	if(!sound_state.soloud_context)
	{
		log_error("sound:init", "Failed to create sound context");
		return false;
	}

	Soloud_initEx(sound_state.soloud_context, SOLOUD_CLIP_ROUNDOFF | SOLOUD_ENABLE_VISUALIZATION, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO, SOLOUD_AUTO);
	Soloud_setGlobalVolume(sound_state.soloud_context, 4);

	Soloud_set3dListenerParameters(sound_state.soloud_context,
								   0.f, 0.f,  0.f,  // Position
								   0.f, 0.f, -1.f,  // At
								   0.f, 1.f,  0.f); // Up

	Soloud_update3dAudio(sound_state.soloud_context);

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
	Soloud_update3dAudio(sound_state.soloud_context);
}

void sound_volume_set(float volume)
{
	if(volume < 0.f) volume = 0.f;
	sound_state.volume = volume;
	Soloud_setGlobalVolume(sound_state.soloud_context, sound_state.volume);
}

void sound_cleanup(void)
{
	char* key = NULL;
	struct Variant* value = NULL;
	HASHMAP_FOREACH(sound_buffers, key, value)
	{
		struct Sound_Buffer* buffer = value->val_voidptr;
		switch(buffer->type)
		{
		case ST_WAV: Wav_destroy(buffer->wav); break;
		case ST_WAV_STREAM: WavStream_destroy(buffer->wavstream); break;
		}
	}

	hashmap_free(sound_buffers);
	Soloud_deinit(sound_state.soloud_context);
	Soloud_destroy(sound_state.soloud_context);

	sound_state.volume          = 0.f;
	sound_state.soloud_context  = NULL;
}

uint sound_source_create(bool relative, const char* filename, int type)
{
    uint handle = 0;
	struct Sound_Buffer* buffer = NULL;

	long size = 0L;
	char* memory = io_file_read(DIRT_INSTALL, filename, "rb", &size);

	//See if we've already loaded this file
	if(hashmap_value_exists(sound_buffers, filename))
	{
		buffer = (struct Sound_Buffer*)hashmap_ptr_get(sound_buffers, filename);
		if(relative)
		{
			handle = Soloud_play3dEx(sound_state.soloud_context,
									 type == ST_WAV ? buffer->wav : buffer->wavstream,
									 0.f, 0.f, 0.f,
									 0.f, 0.f, 0.f,
									 1.f,
									 true,
									 0);
			return handle;
		}
	}

	buffer = malloc(sizeof(*buffer));
	if(!buffer)
	{
		log_error("sound:source_create", "Out of memory!");
		return 0;
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
			free(buffer);
			return 0;
		}
		buffer->type = ST_WAV;
		buffer->wav = wave;
		Wav_set3dAttenuation(wave, 1, 0.9f);
	}
	break;
	case ST_WAV_STREAM:
	{
		WavStream* wave_stream = WavStream_create();
		int rc = WavStream_loadMem(wave_stream, memory, (uint)size);
		if(rc != 0)
		{
			log_error("sound:source_create", "Failed to load %s, Soloud: %s", filename, Soloud_getErrorString(sound_state.soloud_context, rc));
			free(buffer);
			return 0;
		}
		buffer->type = ST_WAV_STREAM;
		buffer->wavstream = wave_stream;
	}
	break;
	default: log_error("sound:source_create", "Invalid source type %d", type); return 0;
	}

	hashmap_ptr_set(sound_buffers, filename, (void*)buffer);
	handle = Soloud_play3dEx(sound_state.soloud_context,
							 type == ST_WAV ? buffer->wav : buffer->wavstream,
							 0.f, 0.f, 0.f,
							 0.f, 0.f, 0.f,
							 1.f,
							 true,
							 0);

	return handle;
}

void sound_source_destroy(uint source_handle)
{
	Soloud_stop(sound_state.soloud_context, source_handle);
}


void sound_source_update(uint source_handle, float apos_x, float apos_y, float apos_z)
{
	Soloud_set3dSourceParameters(sound_state.soloud_context, source_handle,	apos_x, apos_y, apos_z);
	Soloud_update3dAudio(sound_state.soloud_context);
}

void sound_source_volume_set(uint source_handle, float volume)
{
	if(volume < 0.f) volume = 0.f;
	Soloud_setVolume(sound_state.soloud_context, source_handle, volume);
}

void sound_source_loop_set(uint source_handle, bool loop)
{
	Soloud_setLooping(sound_state.soloud_context, source_handle, loop);
}

void sound_source_play(uint source_handle)
{
	Soloud_setPause(sound_state.soloud_context, source_handle, false);
}

void sound_source_pause(uint source_handle)
{
	Soloud_setPause(sound_state.soloud_context, source_handle, true);
}

void sound_source_rewind(uint source_handle)
{
	Soloud_seek(sound_state.soloud_context, source_handle, 0.0);
}

void sound_source_stop(uint source_handle)
{
	Soloud_stop(sound_state.soloud_context, source_handle);
}
