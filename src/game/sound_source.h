#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H

#include <stdbool.h>

struct Sound_Source;
struct Sound;

void sound_source_play(struct Sound* sound, struct Sound_Source* entity);
void sound_source_pause(struct Sound* sound, struct Sound_Source* entity);
void sound_source_stop(struct Sound* sound, struct Sound_Source* entity);
bool sound_source_is_paused(struct Sound* sound, struct Sound_Source* entity);
void sound_source_update_position(struct Sound* sound, struct Sound_Source* entity);
bool sound_source_buffer_set(struct Sound* sound, struct Sound_Source* entity, const char* filename, int type);
void sound_source_validate_instance(struct Sound* sound, struct Sound_Source* entity);
void sound_source_apply_params_to_instance(struct Sound* sound, struct Sound_Source* entity);

#endif