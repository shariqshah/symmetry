#ifndef EDITOR_H
#define EDITOR_H

#include <stdbool.h>
#include "../common/linmath.h"
#include "../common/limits.h"

struct Camera;
struct Entity;
struct Hashmap;
struct Static_Mesh;

struct Editor
{
    int                 window_settings_renderer;
    int                 window_settings_editor;
    int                 window_settings_scene;
	int                 window_scene_heirarchy;
	int                 window_property_inspector;
	int                 window_scene_dialog;
	int                 window_entity_dialog;
	int                 camera_looking_around;
    struct Entity*      selected_entity;
	struct Static_Mesh* cursor_entity;
	struct Entity*      hovered_entity;
	vec4                hovered_entity_color;
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
	bool                tool_translate_allowed;
	float               axis_line_length;
	vec4                axis_color_x;
	vec4                axis_color_y;
	vec4                axis_color_z;
	bool                picking_enabled;
	bool                scene_operation_save;
	bool                entity_operation_save;
	float               notification_timer;
	float               notification_timer_speed;
	float               notification_stay_time;
	char                notification_message[MAX_EDITOR_NOTIFICATION_MESSAGE_LEN];
};

void editor_init(struct Editor* editor_state);
void editor_camera_init(struct Editor* editor_state, struct Hashmap* cvars);
void editor_init_entities(struct Editor* editor);
void editor_render(struct Editor* editor_state, struct Camera* active_camera);
void editor_update(struct Editor* editor_state, float dt);
void editor_post_update(struct Editor* editor);
void editor_cleanup(struct Editor* editor_state);
void editor_set_notification(struct Editor* editor, const char* message, ...);

#endif
