#include "sound.h"
#include "log.h"
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
	ALCdevice*  device;
	ALCcontext* context;
	float       volume;
};

static struct Sound_State sound_state =
{
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

void sound_listener_update(float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z)
{	
	float orientation[] =
	{
        afwd_x, afwd_y, afwd_z,
        aup_x,  aup_y,  aup_z
	};
    al_check(alListener3f(AL_POSITION, apos_x, apos_y, apos_z))
	al_check(alListenerfv(AL_ORIENTATION, orientation))
}

void sound_volume_set(float volume)
{
	if(volume < 0.f) volume = 0.f;
	al_check(alListenerf(AL_GAIN, volume))
}

void sound_source_destroy(uint source_handle, uint* attached_buffers, uint num_buffers)
{
    for(int i = 0; i < (int)num_buffers; i++)
    {
        if(alIsBuffer(attached_buffers[i]) == AL_TRUE)
        {
            al_check(alSourcei(source_handle, AL_BUFFER, i))
            al_check(alDeleteBuffers(1, &attached_buffers[i]))
        }
    }
    if(alIsSource(source_handle) == AL_TRUE) al_check(alDeleteSources(1, &source_handle))
}

void sound_cleanup(void)
{	
	alcMakeContextCurrent(NULL);
	alcDestroyContext(sound_state.context);
	alcCloseDevice(sound_state.device);
	sound_state.context         = NULL;
	sound_state.device          = NULL;
	sound_state.volume          = 0.f;
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

void sound_source_create(bool relative, uint num_buffers, uint* out_handle, uint* out_buffer_handles)
{
    al_check(alGenSources(1, out_handle))
    al_check(alGenBuffers((int)num_buffers, out_buffer_handles))
    sound_source_volume_set(*out_handle, 1.f);
	if(relative)
        sound_source_relative_set(*out_handle, true);
}

void sound_source_update(uint source_handle,
                           float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z)
{
	float orientation[] =
	{
        afwd_x, afwd_y, afwd_z,
        aup_x,  aup_y,  aup_z
	};
    al_check(alSource3f(source_handle, AL_POSITION, apos_x, apos_y, apos_z))
    al_check(alSourcefv(source_handle, AL_ORIENTATION, orientation))
}

void sound_source_volume_set(uint source_handle, float volume)
{
	if(volume < 0.f) volume = 0.f;
    al_check(alSourcef(source_handle, AL_GAIN, volume))
}

void sound_source_pitch_set(uint source_handle, float pitch)
{
	if(pitch < 0.f) pitch = 0.f;
    al_check(alSourcef(source_handle, AL_PITCH, pitch))
}

void sound_source_loop_set(uint source_handle, bool loop)
{
	loop = loop ? AL_TRUE : AL_FALSE;
    al_check(alSourcei(source_handle, AL_LOOPING, loop))
}

void sound_source_relative_set(uint source_handle, bool relative)
{
	relative = relative ? AL_TRUE : AL_FALSE;
    al_check(alSourcei(source_handle, AL_SOURCE_RELATIVE, relative));
}

void sound_source_load_wav(uint source_handle, uint buffer_handle, const char* file_name)
{
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

    al_check(alBufferData(buffer_handle, format, wav_data, wav_data_len, wav_spec.freq))
    al_check(alSourcei(source_handle, AL_BUFFER, buffer_handle))
	SDL_FreeWAV(wav_data);
}

void sound_source_play(uint source_handle)
{
    al_check(alSourcePlay(source_handle))
}

void sound_source_pause(uint source_handle)
{
    al_check(alSourcePause(source_handle))
}

void sound_source_rewind(uint source_handle)
{
    al_check(alSourceRewind(source_handle))
}

void sound_source_stop(uint source_handle)
{
    al_check(alSourceStop(source_handle))
}
