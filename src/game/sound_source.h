#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H

struct Sound_Source;
struct Sound;

void sound_source_play(struct Sound* sound, struct Sound_Source* entity);
void sound_source_pause(struct Sound* sound, struct Sound_Source* entity);
void sound_source_stop(struct Sound* sound, struct Sound_Source* entity);
void sound_source_update_position(struct Sound* sound, struct Sound_Source* entity);
void sound_source_buffer_set(struct Sound* sound, struct Sound_Source* entity);

#endif