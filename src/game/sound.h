#ifndef SOUND_H
#define SOUND_H

#include "../common/num_types.h"

struct Sound_Source_Buffer;

bool sound_init(void);
void sound_cleanup(void);
void sound_volume_set(float volume);
void sound_update_3d(void);

void sound_listener_update(float apos_x, float apos_y, float apos_z,
                           float afwd_x, float afwd_y, float afwd_z,
                           float aup_x,  float aup_y,  float aup_z);


void sound_source_instance_update_position(uint source_instance, float apos_x, float apos_y, float apos_z);
uint sound_source_instance_create(struct Sound_Source_Buffer* source, bool is3d);
void sound_source_instance_destroy(uint source_instance);
void sound_source_instance_volume_set(uint source_instance, float volume);
void sound_source_instance_loop_set(uint source_instance, bool loop);
void sound_source_instance_play(uint source_instance);
void sound_source_instance_pause(uint source_instance);
void sound_source_instance_rewind(uint source_instance);
void sound_source_instance_stop(uint source_instance);
void sound_source_instance_min_max_distance_set(uint source_instance, float min_distance, float max_distance);
void sound_source_instance_attenuation_set(uint source_instance, int attenuation_type, float rolloff_factor);

float sound_source_instance_volume_get(uint source_instance);
bool  sound_source_instance_loop_get(uint source_instance);
bool  sound_source_instance_is_paused(uint source_instance);

struct Sound_Source_Buffer* sound_source_create(const char* filename, int type);
struct Sound_Source_Buffer* sound_source_get(const char* name);
void                        sound_source_destroy(const char* buffer_name);
void                        sound_source_volume_set(struct Sound_Source_Buffer* source, float volume);
void                        sound_source_loop_set(struct Sound_Source_Buffer* source, bool loop);
void                        sound_source_stop_all(struct Sound_Source_Buffer* source);
void                        sound_source_min_max_distance_set(struct Sound_Source_Buffer* source, float min_distance, float max_distance);

#endif
