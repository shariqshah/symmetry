#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include "../common/linmath.h"

struct Camera;
struct Entity;
struct Hashmap;
struct Static_Mesh;

struct Editor
{
    int                 window_settings_renderer;
    int                 window_settings_editor;
	int                 window_scene_heirarchy;
	int                 window_property_inspector;
	int                 window_debug_variables;
	int                 camera_looking_around;
    struct Entity*      selected_entity;
	struct Static_Mesh* cursor_entity;
	vec4                cursor_entity_color;
	bool                draw_cursor_entity;
    int                 top_panel_height;
    float               camera_turn_speed;
    float               camera_move_speed;
    float               camera_sprint_multiplier;
	vec4                selected_entity_color;
	int                 current_tool;
	int                 current_axis;
	int                 previous_axis;
	int                 grid_enabled;
	int                 grid_relative;
	vec4                grid_color;
	int                 grid_num_lines;
	float               grid_scale;
	int                 tool_snap_enabled;
	vec4                tool_mesh_color;
	int                 tool_mesh_draw_enabled;
	float               tool_rotate_arc_radius;
	int                 tool_rotate_arc_segments;
	float               tool_rotate_amount;
	bool                tool_rotate_allowed;
	bool                tool_rotate_rotation_started;
	float               tool_rotate_increment;
	float               tool_rotate_total_rotation;
	float               tool_rotate_starting_rotation;
	vec3                tool_scale_amount;
	bool                tool_scale_started;
	float               axis_line_length;
	vec4                axis_color_x;
	vec4                axis_color_y;
	vec4                axis_color_z;
	bool                picking_enabled;
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
