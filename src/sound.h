#ifndef SOUND_H
#define SOUND_H

#include "num_types.h"

#ifdef AL_DEBUG
    #define al_check(expr) {expr; sound_error_check(__FILE__, __LINE__, #expr);}
#else
    #define al_check(expr) expr;
#endif

struct Sound_Source;
struct Transform;

bool sound_init(void);
void sound_cleanup(void);
void sound_volume_set(float volume);
void sound_error_check(const char* file, unsigned int line, const char* expression);

void sound_listener_set(int entity);
int  sound_listener_get(void);
void sound_listener_update(void);

void sound_source_create(struct Sound_Source* source, struct Transform* transform);
void sound_source_destroy(struct Sound_Source* source);
void sound_source_update(struct Sound_Source* source, struct Transform* transform);
void sound_source_volume_set(struct Sound_Source* source, float volume);
void sound_source_pitch_set(struct Sound_Source* source, float pitch);
void sound_source_load_wav(struct Sound_Source* source, const char* file_name);
void sound_source_loop_set(struct Sound_Source* source, bool loop);
void sound_source_relative_set(struct Sound_Source* source, bool relative);
void sound_source_play(struct Sound_Source* source);
void sound_source_pause(struct Sound_Source* source);
void sound_source_rewind(struct Sound_Source* source);
void sound_source_stop(struct Sound_Source* source);

#endif
