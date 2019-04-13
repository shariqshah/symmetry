#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include "../common/linmath.h"

struct Camera;
struct Entity;

struct Editor
{
    bool           enabled;
    bool           renderer_settings_window;
	bool           camera_looking_around;
    struct Entity* selected_entity;
    int            top_panel_height;
    float          camera_turn_speed;
    float          camera_move_speed;
    float          camera_sprint_multiplier;
	vec4           selected_entity_colour;
};

void editor_init(struct Editor* editor_state);
void editor_init_camera(struct Editor* editor_state);
void editor_render(struct Editor* editor_state, struct Camera* active_camera);
void editor_update(struct Editor* editor_state, float dt);
void editor_cleanup(struct Editor* editor_state);
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
