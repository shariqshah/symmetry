#ifndef SOUND_H
#define SOUND_H

#include "num_types.h"

#ifdef AL_DEBUG
    #define al_check(expr) {expr; sound_error_check(__FILE__, __LINE__, #expr);}
#else
    #define al_check(expr) expr;
#endif

bool sound_init(void);
void sound_cleanup(void);
void sound_volume_set(float volume);
void sound_error_check(const char* file, unsigned int line, const char* expression);

void sound_listener_update(float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z);

void sound_source_update(uint source_handle,
                         float apos_x, float apos_y, float apos_z,
                         float afwd_x, float afwd_y, float afwd_z,
                         float aup_x,  float aup_y,  float aup_z);
void sound_source_create(bool relative, uint num_buffers, uint* out_handle, uint* out_buffer_handles);
void sound_source_destroy(uint source_handle, uint* attached_buffers, uint num_buffers);
void sound_source_volume_set(uint source_handle, float volume);
void sound_source_pitch_set(uint source_handle, float pitch);
void sound_source_load_wav(uint source_handle, uint buffer_handle, const char* file_name);
void sound_source_loop_set(uint source_handle, bool loop);
void sound_source_relative_set(uint source_handle, bool relative);
void sound_source_play(uint source_handle);
void sound_source_pause(uint source_handle);
void sound_source_rewind(uint source_handle);
void sound_source_stop(uint source_handle);

#endif
