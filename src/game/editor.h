#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include "../common/linmath.h"

struct Camera;
struct Entity;
struct Hashmap;

struct Editor
{
    int            window_settings_renderer;
    int            window_settings_editor;
	int            window_scene_heirarchy;
	int            window_property_inspector;
	int            window_debug_variables;
	int            camera_looking_around;
    struct Entity* selected_entity;
    int            top_panel_height;
    float          camera_turn_speed;
    float          camera_move_speed;
    float          camera_sprint_multiplier;
	vec4           selected_entity_colour;
	int            current_mode;
	int            current_axis;
	int            current_transform_space;
	int            previous_axis;
	int            grid_enabled;
	vec4           grid_color;
	int            grid_num_lines;
	float          grid_scale;
	int            tool_snap_enabled;
	vec3           tool_mesh_position;
	vec4           tool_mesh_color;
	int            tool_mesh_draw_enabled;
};

void editor_init(struct Editor* editor_state);
void editor_init_camera(struct Editor* editor_state, struct Hashmap* cvars);
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
