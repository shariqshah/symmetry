#ifndef SOUND_H
#define SOUND_H

#include "../common/num_types.h"
#include "../common/linmath.h"
#include "file_io.h"

#define MAX_SOUND_BUFFERS 128

typedef void* Wav;
typedef void* WavStream;
typedef void* Soloud;

enum Sound_Source_Type
{
	ST_NONE = 0,
	ST_WAV,
	ST_WAV_STREAM
};

enum Sound_Attenuation_Type
{
	SA_NONE = 0,   // No attenuation
	SA_INVERSE,    // Inverse distance attenuation model
	SA_LINEAR,     // Linear distance attenuation model
	SA_EXPONENTIAL // Exponential distance attenuation model
};

struct Sound_Source_Buffer
{
	int type;
	char filename[MAX_FILENAME_LEN];
	union
	{
		Wav* wav;
		WavStream* wavstream;
	};
};

struct Sound
{
	Soloud*                    soloud_context;
	struct Entity*             listener;
	float                      master_volume;
	struct Sound_Source_Buffer sound_buffers[MAX_SOUND_BUFFERS];
};

bool sound_init(struct Sound* sound);
void sound_cleanup(struct Sound* sound);
void sound_master_volume_set(struct Sound* sound, float volume);
void sound_listener_set(struct Sound* sound, struct Entity* listener);
void sound_update_3d(struct Sound* sound);
void sound_listener_update(struct Sound* sound);
void sound_pause_all(struct Sound* sound, bool pause);

void sound_source_instance_update_position(struct Sound* sound, uint source_instance, vec3 abs_pos);
uint sound_source_instance_create(struct Sound* sound, struct Sound_Source_Buffer* source, bool is3d);
void sound_source_instance_destroy(struct Sound* sound, uint source_instance);
void sound_source_instance_volume_set(struct Sound* sound, uint source_instance, float volume);
void sound_source_instance_loop_set(struct Sound* sound, uint source_instance, bool loop);
void sound_source_instance_play(struct Sound* sound, uint source_instance);
void sound_source_instance_pause(struct Sound* sound, uint source_instance);
void sound_source_instance_rewind(struct Sound* sound, uint source_instance);
void sound_source_instance_stop(struct Sound* sound, uint source_instance);
void sound_source_instance_min_max_distance_set(struct Sound* sound, uint source_instance, float min_distance, float max_distance);
void sound_source_instance_attenuation_set(struct Sound* sound, uint source_instance, int attenuation_type, float rolloff_factor);
bool sound_source_instance_is_valid(struct Sound* sound, uint source_instance);

float sound_source_instance_volume_get(struct Sound* sound, uint source_instance);
bool  sound_source_instance_loop_get(struct Sound* sound, uint source_instance);
bool  sound_source_instance_is_paused(struct Sound* sound, uint source_instance);

struct Sound_Source_Buffer* sound_source_buffer_create(struct Sound* sound, const char* filename, int type);
struct Sound_Source_Buffer* sound_source_buffer_get(struct Sound* sound, const char* name);
int                         sound_source_buffer_play_3d(struct Sound* sound, struct Sound_Source_Buffer* source, vec3 position);
int                         sound_source_buffer_play_clocked_3d(struct Sound* sound, struct Sound_Source_Buffer* source, float delay, vec3 position);
void                        sound_source_buffer_destroy(struct Sound* sound, struct Sound_Source_Buffer* source);
void                        sound_source_buffer_volume_set(struct Sound* sound, struct Sound_Source_Buffer* source, float volume);
void                        sound_source_buffer_loop_set(struct Sound* sound, struct Sound_Source_Buffer* source, bool loop);
void                        sound_source_buffer_stop_all(struct Sound* sound, struct Sound_Source_Buffer* source);
void                        sound_source_buffer_min_max_distance_set(struct Sound* sound, struct Sound_Source_Buffer* source, float min_distance, float max_distance);

#endif
