#include "sound.h"
#include "log.h"
#include "transform.h"
#include "entity.h"
#include "array.h"
#include "string_utils.h"
#include "file_io.h"

#include <assert.h>
#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <SDL2/SDL.h>

struct Sound_State
{
	int         listener_entity;
	ALCdevice*  device;
	ALCcontext* context;
	float       volume;
};

static struct Sound_State sound_state =
{
	.listener_entity = -1,
	.device          = NULL,
	.context         = NULL,
	.volume          = 0.f
};

bool sound_init(void)
{
	bool success = false;
	
	sound_state.device = alcOpenDevice(NULL);
	if(!sound_state.device)
	{
		log_error("sound:init", "Failed to open a sound sound_state.device");
		return success;
	}

	sound_state.context = alcCreateContext(sound_state.device, NULL);
	if(sound_state.context == NULL || alcMakeContextCurrent(sound_state.context) == ALC_FALSE)
	{
		if(!sound_state.context) alcDestroyContext(sound_state.context);
		alcCloseDevice(sound_state.device);
		log_error("sound:init", "Could not set sound context");
		return success;
			
	}
	log_message("Opened %s", alcGetString(sound_state.device, ALC_DEVICE_SPECIFIER));
	
	/* Set default listener values */
	sound_state.listener_entity = -1;
	sound_state.volume          = 1.f;
	float orientation[] =
	{
		0.f, 0.f, -1.f,			/* Direction, towards negative z axis */
		0.f, 1.f,  0.f			/* Up vector */
	};
	al_check(alListenerf(AL_GAIN, sound_state.volume))
    al_check(alListener3f(AL_POSITION, 0.f, 0.f, 0.f))
	al_check(alListenerfv(AL_ORIENTATION, orientation))
	
	success = true;
	return success;
}

void sound_listener_set(int listener_entity)
{
	assert(listener_entity > -1);
	if(sound_state.listener_entity != -1)
	{
		struct Entity* current_listener = entity_get(sound_state.listener_entity);
		current_listener->is_listener   = false;
	}
	
	sound_state.listener_entity = listener_entity;
	struct Entity* entity = entity_get(listener_entity);
	entity->is_listener = true;
	sound_listener_update();
}

void sound_listener_update(void)
{
	if(sound_state.listener_entity == -1) return;
	struct Entity* entity = entity_get(sound_state.listener_entity);
	vec3 abs_pos     = {0.f, 0.f,  0.f};
	vec3 abs_up      = {0.f, 1.f,  0.f};
	vec3 abs_forward = {0.f, 0.f, -1.f};
	
	transform_get_absolute_pos(entity, &abs_pos);
	transform_get_absolute_up(entity, &abs_up);
	transform_get_absolute_forward(entity, &abs_forward);
	float orientation[] =
	{
		abs_forward.x, abs_forward.y, abs_forward.z,
		abs_up.x,      abs_up.y,      abs_up.z
	};
	al_check(alListener3f(AL_POSITION, abs_pos.x, abs_pos.y, abs_pos.z))
	al_check(alListenerfv(AL_ORIENTATION, orientation))
}

int sound_listener_get(void)
{
	return sound_state.listener_entity;
}

void sound_volume_set(float volume)
{
	if(volume < 0.f) volume = 0.f;
	al_check(alListenerf(AL_GAIN, volume))
}

void sound_source_destroy(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	struct Sound_Source* source = &entity->sound_source;
	if(!source->active) return;
	if(alIsBuffer(source->al_buffer_handle) == AL_TRUE)
	{
		sound_source_stop(entity);
		al_check(alSourcei(source->al_source_handle, AL_BUFFER, 0))
		al_check(alDeleteBuffers(1, &source->al_buffer_handle))
	}
	if(alIsSource(source->al_source_handle) == AL_TRUE) al_check(alDeleteSources(1, &source->al_source_handle))
	source->al_buffer_handle =  0;
	source->al_source_handle =  0;
	source->active           =  false;
}

void sound_cleanup(void)
{	
	alcMakeContextCurrent(NULL);
	alcDestroyContext(sound_state.context);
	alcCloseDevice(sound_state.device);
	sound_state.context         = NULL;
	sound_state.device          = NULL;
	sound_state.volume          = 0.f;
	sound_state.listener_entity = -1;
}

void sound_error_check(const char* file, unsigned int line, const char* expression)
{
	ALenum error_code = alGetError();
	const char* error_string = "No Error String";
	if(error_code != AL_NO_ERROR)
	{
		switch(error_code)
		{
		case AL_INVALID_NAME:      error_string = "AL_INVALID_NAME, Invalid name(ID) specified";          break;
		case AL_INVALID_ENUM:      error_string = "AL_INVALID_ENUM, Invalid enum specified";              break;
		case AL_INVALID_VALUE:     error_string = "AL_INVALID_VALUE, A numeric argument is out of range"; break;
		case AL_INVALID_OPERATION: error_string = "AL_INVALID_OPERATION, The specified operation is not allowed in current state"; break;
		case AL_OUT_OF_MEMORY:     error_string = "AL_OUT_OF_MEMORY, Not enough memory to execute command"; break;
		}
		log_error("OpenAL", "%s\n(%s::%d): %s", file, expression, line, error_string);
	}
}

void sound_source_create(struct Entity* entity, bool relative)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	struct Sound_Source* source = &entity->sound_source;
	source->active = true;
	al_check(alGenSources(1, &source->al_source_handle))
	al_check(alGenBuffers(1, &source->al_buffer_handle))
	sound_source_volume_set(entity, 1.f);
	if(relative)
		sound_source_relative_set(entity, true);
	else
		sound_source_update(entity);
}

void sound_source_update(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	vec3 abs_pos     = {0.f, 0.f,  0.f};
	vec3 abs_up      = {0.f, 1.f,  0.f};
	vec3 abs_forward = {0.f, 0.f, -1.f};
	
	transform_get_absolute_pos(entity, &abs_pos);
	transform_get_absolute_up(entity, &abs_up);
	transform_get_absolute_forward(entity, &abs_forward);
	float orientation[] =
	{
		abs_forward.x, abs_forward.y, abs_forward.z,
		abs_up.x,      abs_up.y,      abs_up.z
	};
	al_check(alSource3f(entity->sound_source.al_source_handle, AL_POSITION, abs_pos.x, abs_pos.y, abs_pos.z))
	al_check(alSourcefv(entity->sound_source.al_source_handle, AL_ORIENTATION, orientation))
}

void sound_source_volume_set(struct Entity* entity, float volume)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	if(volume < 0.f) volume = 0.f;
	al_check(alSourcef(entity->sound_source.al_source_handle, AL_GAIN, volume))
}

void sound_source_pitch_set(struct Entity* entity, float pitch)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	if(pitch < 0.f) pitch = 0.f;
	al_check(alSourcef(entity->sound_source.al_source_handle, AL_PITCH, pitch))
}

void sound_source_loop_set(struct Entity* entity, bool loop)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	loop = loop ? AL_TRUE : AL_FALSE;
	al_check(alSourcei(entity->sound_source.al_source_handle, AL_LOOPING, loop))
}

void sound_source_relative_set(struct Entity* entity, bool relative)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	relative = relative ? AL_TRUE : AL_FALSE;
	al_check(alSourcei(entity->sound_source.al_source_handle, AL_SOURCE_RELATIVE, relative));
	entity->sound_source.relative = relative;
}

void sound_source_load_wav(struct Entity* entity, const char* file_name)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	if(!file_name)
	{
		log_error("sound_source:load_wav", "No file name given");
		return;
	}
	
	char* full_path = str_new("sounds/%s", file_name);
	FILE* wav_file = io_file_open(DT_INSTALL, full_path, "rb");
	free(full_path);
	if(!wav_file)
	{
		log_error("sound_source:load_wav", "Failed to load %s", file_name);
		return;
	}

	SDL_RWops* sdl_rw = SDL_RWFromFP(wav_file, SDL_TRUE);
	Uint8* wav_data = NULL;
	Uint32 wav_data_len;
	SDL_AudioSpec wav_spec;
	if(!SDL_LoadWAV_RW(sdl_rw, 1, &wav_spec, &wav_data, &wav_data_len))
	{
		log_error("sound_source:load_wav", "Failed to load wav %s", SDL_GetError());
		//SDL_FreeRW(sdl_rw);
		return;
	}

	bool mono   = wav_spec.channels == 1 ? true : false;
	int  format = -1;
	if(mono)
	{
		if(wav_spec.format == AUDIO_U8 || wav_spec.format == AUDIO_S8)
			format = AL_FORMAT_MONO8;
		else if(wav_spec.format == AUDIO_S16 || wav_spec.format == AUDIO_S16LSB || wav_spec.format == AUDIO_S16MSB || wav_spec.format == AUDIO_S16SYS)
			format = AL_FORMAT_MONO16;
		else if(wav_spec.format == AUDIO_F32 || wav_spec.format == AUDIO_F32LSB || wav_spec.format == AUDIO_F32MSB || wav_spec.format == AUDIO_F32SYS || wav_spec.format == AUDIO_S32 || wav_spec.format == AUDIO_S32LSB)
			format = AL_FORMAT_MONO_FLOAT32;
	}
	else
	{
		/* TODO: FIX THIS!!!! This should resemble the if condition  */
		if(wav_spec.format == AUDIO_U8 || wav_spec.format == AUDIO_S8)
			format = AL_FORMAT_STEREO8;
		else if(wav_spec.format == AUDIO_S16 || wav_spec.format == AUDIO_S16LSB || wav_spec.format == AUDIO_S16MSB || wav_spec.format == AUDIO_S16SYS)
			format = AL_FORMAT_STEREO16;
		else if(wav_spec.format == AUDIO_F32 || wav_spec.format == AUDIO_F32LSB || wav_spec.format == AUDIO_F32MSB || wav_spec.format == AUDIO_F32SYS || wav_spec.format == AUDIO_S32 || wav_spec.format == AUDIO_S32LSB)
			format = AL_FORMAT_STEREO_FLOAT32;
	}

	if(format == -1)
	{
		log_error("sound_source:load_wav", "Unsupported audio format");
		SDL_FreeWAV(wav_data);
		return;
	}

	struct Sound_Source* source = &entity->sound_source;
	al_check(alBufferData(source->al_buffer_handle, format, wav_data, wav_data_len, wav_spec.freq))
	al_check(alSourcei(source->al_source_handle, AL_BUFFER, source->al_buffer_handle))
	SDL_FreeWAV(wav_data);
}

void sound_source_play(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	al_check(alSourcePlay(entity->sound_source.al_source_handle))
}

void sound_source_pause(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	al_check(alSourcePause(entity->sound_source.al_source_handle))
}

void sound_source_rewind(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	al_check(alSourceRewind(entity->sound_source.al_source_handle))
}

void sound_source_stop(struct Entity* entity)
{
	assert(entity && entity->type == ET_SOUND_SOURCE);
	al_check(alSourceStop(entity->sound_source.al_source_handle))
}
