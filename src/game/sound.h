#ifndef SOUND_H
#define SOUND_H

#include "../common/num_types.h"

#ifdef AL_DEBUG
    #define al_check(expr) expr;
#else
    #define al_check(expr) expr;
#endif

bool sound_init(void);
void sound_cleanup(void);
void sound_volume_set(float volume);

void sound_listener_update(float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z);

void sound_source_update(uint source_handle, float apos_x, float apos_y, float apos_z);
uint sound_source_create(bool relative, const char* filename, int type);
void sound_source_destroy(uint handle);
void sound_source_volume_set(uint handle, float volume);
void sound_source_loop_set(uint handle, bool loop);
void sound_source_play(uint handle);
void sound_source_pause(uint handle);
void sound_source_rewind(uint handle);
void sound_source_stop(uint handle);

#endif
