#ifndef EDITOR_H
#define EDITOR_H

#include "../common/linmath.h"

void editor_init(void);
void editor_init_camera(void);
void editor_update(float dt);
void editor_cleanup(void);
int  editor_debugvar_slot_create(const char* name, int value_type);
void editor_debugvar_slot_remove(int index);
void editor_debugvar_slot_set_float(int index, float value);
void editor_debugvar_slot_set_int(int index, int value);
void editor_debugvar_slot_set_double(int index, double value);
void editor_debugvar_slot_set_vec2(int index, vec2* value);
void editor_debugvar_slot_set_vec3(int index, vec3* value);
void editor_debugvar_slot_set_vec4(int index, vec4* value);
void editor_debugvar_slot_set_quat(int index, quat* value);

#endif
