#ifndef SOUND_H
#define SOUND_H

#include "num_types.h"

#ifdef AL_DEBUG
    #define al_check(expr) {expr; sound_error_check(__FILE__, __LINE__, #expr);}
#else
    #define al_check(expr);
#endif

struct Sound_Source
{
	int  entity;
	uint al_source_handle;
	uint al_buffer_handle;
	int  active;
};

int  sound_init(void);
void sound_cleanup(void);
void sound_volume_set(float volume);
void sound_error_check(const char* file, unsigned int line, const char* expression);

void sound_listener_set(int entity);
int  sound_listener_get(void);
void sound_listener_update(void);

struct Sound_Source* sound_source_get(int index);
int  				 sound_source_create(int entity);
void 				 sound_source_remove(int index);
void 				 sound_source_update(struct Sound_Source* source);
void 				 sound_source_volume_set(struct Sound_Source* source, float volume);
void 				 sound_source_pitch_set(struct Sound_Source* source, float pitch);
void 				 sound_source_load_wav(struct Sound_Source* source, const char* file_name);
void 				 sound_source_loop_set(struct Sound_Source* source, int loop);
void 				 sound_source_relative_set(struct Sound_Source* source, int relative);
void 				 sound_source_play(struct Sound_Source* source);
void 				 sound_source_pause(struct Sound_Source* source);
void 				 sound_source_rewind(struct Sound_Source* source);
void 				 sound_source_stop(struct Sound_Source* source);

#endif
