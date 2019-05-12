#include "editor.h"
#include "renderer.h"
#include "gl_load.h"
#include "../common/log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "../common/array.h"
#include "../common/hashmap.h"
#include "shader.h"
#include "../common/num_types.h"
#include "light.h"
#include "entity.h"
#include "transform.h"
#include "game.h"
#include "gui.h"
#include "../common/array.h"
#include "../common/variant.h"
#include "../common/num_types.h"
#include "../common/string_utils.h"
#include "bounding_volumes.h"
#include "input.h"
#include "scene.h"
#include "../system/file_io.h"
#include "../system/config_vars.h"
#include "../system/platform.h"
#include "event.h"
#include "im_render.h"
#include "geometry.h"
#include "gui.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <math.h>

struct Debug_Variable
{
    struct Variant data;
    char*          name;
};

enum Editor_Tool
{
	EDITOR_TOOL_NORMAL = 0,
	EDITOR_TOOL_TRANSLATE,
	EDITOR_TOOL_ROTATE,
	EDITOR_TOOL_SCALE,
	EDITOR_TOOL_MAX
};

enum Editor_Axis
{
	EDITOR_AXIS_NONE = 0,
	EDITOR_AXIS_X,
	EDITOR_AXIS_Y,
	EDITOR_AXIS_Z,
	EDITOR_AXIS_XZ,
	EDITOR_AXIS_XY,
	EDITOR_AXIS_YZ,
};

static struct Debug_Variable* debug_vars_list = NULL;
static int*                   empty_indices   = NULL;
static int                    window_flags    = NK_WINDOW_BORDER |
		                                        NK_WINDOW_CLOSABLE |
		                                        NK_WINDOW_MOVABLE |
		                                        NK_WINDOW_SCROLL_AUTO_HIDE |
		                                        NK_WINDOW_SCALABLE;

static void editor_on_mousebutton_press(const struct Event* event);
static void editor_on_mousebutton_release(const struct Event* event);
static void editor_on_mousemotion(const struct Event* event);
static void editor_on_key_release(const struct Event* event);
static void editor_on_key_press(const struct Event* event);
static void editor_camera_update(struct Editor* editor, float dt);
static void editor_show_entity_in_list(struct Editor* editor, struct nk_context* context, struct Scene* scene, struct Entity* entity);
static void editor_widget_color_combov3(struct nk_context* context, vec3* color, int width, int height);
static void editor_widget_color_combov4(struct nk_context* context, vec4* color, int width, int height);
static bool editor_widget_v3(struct nk_context* context,
			                 vec3*              value,
			                 const char*        name_x,
			                 const char*        name_y,
			                 const char*        name_z,
			                 float              min,
			                 float              max,
			                 float              step,
			                 float              inc_per_pixel,
			                 int                row_height);

static void editor_window_scene_heirarchy(struct nk_context* context, struct Editor* editor, struct Game_State* game_state);
static void editor_window_debug_variables(struct nk_context* context, struct Editor* editor);
static void editor_window_property_inspector(struct nk_context* context, struct Editor* editor, struct Game_State* game_state);
static void editor_window_renderer_settings(struct nk_context* context, struct Editor* editor, struct Game_State* game_state);
static void editor_window_settings_editor(struct nk_context* context, struct Editor* editor, struct Game_State* game_state);
static void editor_axis_set(struct Editor* editor, int axis);
static void editor_entity_select(struct Editor* editor, struct Entity* entity);
static void editor_tool_set(struct Editor* editor, int mode);
static void editor_tool_reset(struct Editor* editor);

void editor_init(struct Editor* editor)
{
    editor->window_settings_renderer           = 0;
    editor->window_settings_editor             = 0;
	editor->window_debug_variables             = 0;
	editor->window_property_inspector          = 0;
	editor->window_scene_heirarchy             = 0;
	editor->camera_looking_around              = 0;
    editor->selected_entity                    = NULL;
    editor->hovered_entity                     = NULL;
    editor->top_panel_height                   = 30;
    editor->camera_turn_speed                  = 50.f;
    editor->camera_move_speed                  = 20.f;
    editor->camera_sprint_multiplier           = 2.f;
	editor->current_tool                       = EDITOR_TOOL_NORMAL;
	editor->current_axis                       = EDITOR_AXIS_NONE;
	editor->previous_axis                      = EDITOR_AXIS_NONE;
	editor->grid_enabled                       = 1;
	editor->grid_relative                      = 1;
	editor->grid_num_lines                     = 100;
	editor->grid_scale                         = 1.f;
	editor->tool_mesh_draw_enabled             = 1;
	editor->tool_snap_enabled                  = 1;
	editor->tool_rotate_amount                 = 0.f;
	editor->tool_rotate_increment              = 5.f;
	editor->tool_rotate_arc_radius             = 5.f;
	editor->tool_rotate_arc_segments           = 20.f;
	editor->tool_rotate_starting_rotation      = 0.f;
	editor->tool_rotate_rotation_started       = false;
	editor->tool_rotate_allowed                = false;
	editor->axis_line_length                   = 500.f;
	editor->picking_enabled                    = true;
	editor->draw_cursor_entity                 = false;
	editor->tool_scale_started                 = false;

	vec4_fill(&editor->cursor_entity_color, 0.f, 1.f, 1.f, 1.f);
	vec4_fill(&editor->hovered_entity_color, 0.53, 0.87, 0.28, 1.f);
	vec4_fill(&editor->selected_entity_color, 0.96, 0.61, 0.17, 1.f);
	vec4_fill(&editor->grid_color, 0.3f, 0.3f, 0.3f, 0.7f);
	vec4_fill(&editor->axis_color_x, 0.87, 0.32, 0.40, 1.f);
	vec4_fill(&editor->axis_color_y, 0.53, 0.67, 0.28, 1.f);
	vec4_fill(&editor->axis_color_z, 0.47, 0.67, 0.89, 1.f);
	vec3_fill(&editor->tool_scale_amount, 1.f, 1.f, 1.f);

    debug_vars_list = array_new(struct Debug_Variable);
    empty_indices   = array_new(int);
	
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_subscribe(event_manager, EVT_MOUSEBUTTON_PRESSED, &editor_on_mousebutton_press);
	event_manager_subscribe(event_manager, EVT_MOUSEBUTTON_RELEASED, &editor_on_mousebutton_release);
	event_manager_subscribe(event_manager, EVT_MOUSEMOTION, &editor_on_mousemotion);
	event_manager_subscribe(event_manager, EVT_KEY_PRESSED, &editor_on_key_press);
	event_manager_subscribe(event_manager, EVT_KEY_RELEASED, &editor_on_key_release);

	editor->cursor_entity = scene_static_mesh_create(game_state_get()->scene, "EDITOR_SELECTED_ENTITY_WIREFRAME", NULL, "sphere.symbres", MAT_UNSHADED);
}

void editor_init_camera(struct Editor* editor, struct Hashmap* cvars)
{
    struct Camera* editor_camera = &game_state_get()->scene->cameras[CAM_EDITOR];
    entity_rename(editor_camera, "Editor_Camera");
    editor_camera->base.active = true;
    editor_camera->clear_color.x = 0.3f;
    editor_camera->clear_color.y = 0.6f;
    editor_camera->clear_color.z = 0.9f;
    editor_camera->clear_color.w = 1.f;

    int render_width  = hashmap_int_get(cvars, "render_width");
    int render_height = hashmap_int_get(cvars, "render_height");
    camera_attach_fbo(editor_camera, render_width, render_height, true, true, true);

    vec3 cam_pos = {5.f, 20.f, 50.f};
    transform_translate(editor_camera, &cam_pos, TS_WORLD);
}

void editor_render(struct Editor* editor, struct Camera * active_camera)
{
	struct Game_State* game_state = game_state_get();
	struct Renderer* renderer = game_state->renderer;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if(game_state->editor->selected_entity)
	{
		/* Draw selected entity */
		if(editor->selected_entity->type == ET_STATIC_MESH)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shader_bind(renderer->debug_shader);
			{
				static mat4 mvp;
				shader_set_uniform_vec4(renderer->debug_shader, "debug_color", &editor->selected_entity_color);
				struct Static_Mesh* mesh = (struct Static_Mesh*)editor->selected_entity;
				struct Model*       model = &mesh->model;
				struct Transform*   transform = &mesh->base.transform;
				int                 geometry = model->geometry_index;
				mat4_identity(&mvp);
				mat4_mul(&mvp, &active_camera->view_proj_mat, &transform->trans_mat);
				shader_set_uniform_mat4(renderer->debug_shader, "mvp", &mvp);
				geom_render(geometry, GDM_TRIANGLES);
			}
			shader_unbind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		}
		else
		{
			//For now draw a placeholder sphere just to visually denote that the entity is selected
			vec3 abs_pos;
			quat abs_rot;
			transform_get_absolute_position(editor->selected_entity, &abs_pos);
			transform_get_absolute_rot(editor->selected_entity, &abs_rot);
			im_sphere(1.f, abs_pos, abs_rot, editor->selected_entity_color, GDM_TRIANGLES, 1);
			switch(editor->selected_entity->type)
			{
			case ET_LIGHT:
			{
				struct Light* light = (struct Light*)editor->selected_entity;
				if(light->type != LT_POINT)
				{
					struct Ray light_ray;
					vec3_assign(&light_ray.origin, &abs_pos);
					transform_get_absolute_forward(light, &light_ray.direction);
					im_ray(&light_ray, 5.f, editor->cursor_entity_color, 3);
				}
			}
			break;
			}
		}

		/* Draw selected entity with projected transformation applied  */
		if(editor->draw_cursor_entity)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shader_bind(renderer->debug_shader);
			{
				static mat4 mvp;
				shader_set_uniform_vec4(renderer->debug_shader, "debug_color", &editor->cursor_entity_color);
				struct Static_Mesh* mesh      = editor->cursor_entity;
				struct Model*       model     = editor->selected_entity->type == ET_STATIC_MESH ? &((struct Static_Mesh*)editor->selected_entity)->model : &mesh->model;
				struct Transform*   transform = &mesh->base.transform;
				int                 geometry  = model->geometry_index;
				mat4_identity(&mvp);
				mat4_mul(&mvp, &active_camera->view_proj_mat, &transform->trans_mat);
				shader_set_uniform_mat4(renderer->debug_shader, "mvp", &mvp);
				geom_render(geometry, GDM_TRIANGLES);
			}
			shader_unbind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	/* If cursor is hovering over an entity, draw it*/
	if(editor->hovered_entity)
	{
		if(editor->hovered_entity->type == ET_STATIC_MESH)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			shader_bind(renderer->debug_shader);
			{
				static mat4 mvp;
				shader_set_uniform_vec4(renderer->debug_shader, "debug_color", &editor->hovered_entity_color);
				struct Static_Mesh* mesh = editor->hovered_entity;
				struct Model*       model = &mesh->model;
				struct Transform*   transform = &mesh->base.transform;
				int                 geometry = model->geometry_index;
				mat4_identity(&mvp);
				mat4_mul(&mvp, &active_camera->view_proj_mat, &transform->trans_mat);
				shader_set_uniform_mat4(renderer->debug_shader, "mvp", &mvp);
				geom_render(geometry, GDM_TRIANGLES);
			}
			shader_unbind();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			vec3 abs_pos;
			quat abs_rot;
			transform_get_absolute_position(editor->hovered_entity, &abs_pos);
			transform_get_absolute_rot(editor->hovered_entity, &abs_rot);
			im_sphere(1.f, abs_pos, abs_rot, editor->hovered_entity_color, GDM_TRIANGLES, 4);
		}
	}

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);


	vec3 position = { 0.f, 0.f, 0.f };
	quat rotation = { 0.f, 0.f, 0.f, 1.f };
	vec3 scale = { 1.f, 1.f, 1.f };

	float half_axis_line_length = editor->axis_line_length / 2.f;

	im_line((vec3) { -half_axis_line_length, 0.f, 0.f }, (vec3) { half_axis_line_length, 0.f, 0.f }, position, rotation, editor->axis_color_x, 1); // X Axis
	im_line((vec3) { 0.f, -half_axis_line_length, 0.f }, (vec3) { 0.f, half_axis_line_length, 0.f }, position, rotation, editor->axis_color_y, 1); // Y Axis
	im_line((vec3) { 0.f, 0.f, -half_axis_line_length }, (vec3) { 0.f, 0.f, half_axis_line_length }, position, rotation, editor->axis_color_z, 1); // Z Axis

	//Draw Grid
	if(editor->grid_enabled)
	{
		if(editor->grid_relative && editor->selected_entity)
		{
			transform_get_absolute_position(editor->selected_entity, &position);
		}

		im_begin(position, rotation, scale, editor->grid_color, GDM_LINES, 0);

		float half_grid = editor->grid_num_lines * editor->grid_scale / 2.f;
		for(float i = 0; i <= editor->grid_num_lines * editor->grid_scale; i += editor->grid_scale)
		{
			im_pos(-half_grid,     0.f, -half_grid + i); im_pos(half_grid,      0.f, -half_grid + i); // X
			im_pos(-half_grid + i, 0.f, -half_grid);     im_pos(-half_grid + i, 0.f,  half_grid);     // Z
		}

		im_end();
	}
}

int editor_debugvar_slot_create(const char* name, int value_type)
{
	int index = -1;
	struct Debug_Variable* debug_var = NULL;
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		array_pop(empty_indices);
		debug_var = &debug_vars_list[index];
	}
	else
	{
		debug_var = array_grow(debug_vars_list, struct Debug_Variable);
		index = array_len(debug_vars_list) - 1;
	}
	debug_var->name = str_new(name);
	debug_var->data.type = value_type;

	return index;
}

void editor_debugvar_slot_remove(int index)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    struct Debug_Variable* debug_var = &debug_vars_list[index];
    variant_free(&debug_var->data);
    if(debug_var->name) free(debug_var->name);
    debug_var->name      = NULL;
    debug_var->data.type = VT_NONE;
}

void editor_debugvar_slot_set_float(int index, float value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_float(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_int(int index, int value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_int(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_double(int index, double value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_double(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_vec2(int index, vec2* value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_vec2(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_vec3(int index, vec3* value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_vec3(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_vec4(int index, vec4* value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_vec4(&debug_vars_list[index].data, value);
}

void editor_debugvar_slot_set_quat(int index, quat* value)
{
    assert(index > -1 && index < array_len(debug_vars_list));
    variant_assign_quat(&debug_vars_list[index].data, value);
}

void editor_update(struct Editor* editor, float dt)
{
	editor_camera_update(editor, dt);

	struct Game_State* game_state = game_state_get();
	struct nk_context* context    = &game_state->gui->context;
	int win_width = 0, win_height = 0;
	window_get_drawable_size(game_state->window, &win_width, &win_height);
	int half_width = win_width / 2, half_height = win_height / 2;

	/* Top Panel */
	if(nk_begin(context, "Top Panel", nk_recti(0, 0, win_width, editor->top_panel_height), NK_WINDOW_NO_SCROLLBAR))
	{
		const int row_height = 25.f;
		nk_menubar_begin(context);

		nk_layout_row_begin(context, NK_DYNAMIC, editor->top_panel_height - 5, 8);
		nk_layout_row_push(context, 0.03f);
		if(nk_menu_begin_label(context, "File", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, nk_vec2(150, 100)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			nk_menu_item_label(context, "Open", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			nk_menu_item_label(context, "Save", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			if(nk_menu_item_label(context, "Back to Game", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE))
			{
				game_state->game_mode = GAME_MODE_GAME;
				game_state->scene->active_camera_index = CAM_GAME;
			}
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.05f);
		if(nk_menu_begin_label(context, "Settings", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, nk_vec2(150, 100)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			if(nk_menu_item_label(context, "Editor Settings", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE)) editor->window_settings_editor = !editor->window_settings_editor;
			if(nk_menu_item_label(context, "Render Settings", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE)) editor->window_settings_renderer = !editor->window_settings_renderer;
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.05f);
		if(nk_menu_begin_label(context, "Windows", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, nk_vec2(180, 100)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			nk_checkbox_label(context, "Scene Heirarchy", &editor->window_scene_heirarchy);
			nk_checkbox_label(context, "Property Inspector", &editor->window_property_inspector);
			nk_checkbox_label(context, "Debug Variables", &editor->window_debug_variables);
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.45f);
		nk_spacing(context, 1);

		nk_layout_row_push(context, 0.15f);
		static const char* editor_transformation_tools[] = { "Tool: Normal", "Tool: Translate", "Tool: Rotate", "Tool: Scale" };
		if(nk_combo_begin_label(context, editor_transformation_tools[editor->current_tool], nk_vec2(160, 125)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			int tool = editor->current_tool;
			tool = nk_option_label(context, "Normal", tool == EDITOR_TOOL_NORMAL) ? EDITOR_TOOL_NORMAL : tool;
			tool = nk_option_label(context, "Translate", tool == EDITOR_TOOL_TRANSLATE) ? EDITOR_TOOL_TRANSLATE : tool;
			tool = nk_option_label(context, "Rotate", tool == EDITOR_TOOL_ROTATE) ? EDITOR_TOOL_ROTATE : tool;
			tool = nk_option_label(context, "Scale", tool == EDITOR_TOOL_SCALE) ? EDITOR_TOOL_SCALE : tool;
			editor_tool_set(editor, tool);
			nk_combo_end(context);
		}

		nk_layout_row_push(context, 0.1f);
		static const char* editor_axes[] = { "Axis: None", "Axis: X", "Axis: Y", "Axis: Z", "Axis: XZ", "Axis: XY", "Axis: YZ" };
		if(nk_combo_begin_label(context, editor_axes[editor->current_axis], nk_vec2(160, 125)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			int axis = editor->current_axis;
			axis = nk_option_label(context, "X",    axis == EDITOR_AXIS_X)    ? EDITOR_AXIS_X    : axis;
			axis = nk_option_label(context, "Y",    axis == EDITOR_AXIS_Y)    ? EDITOR_AXIS_Y    : axis;
			axis = nk_option_label(context, "Z",    axis == EDITOR_AXIS_Z)    ? EDITOR_AXIS_Z    : axis;
			if(editor->current_tool != EDITOR_TOOL_ROTATE)
			{
				axis = nk_option_label(context, "XZ", axis == EDITOR_AXIS_XZ) ? EDITOR_AXIS_XZ : axis;
				axis = nk_option_label(context, "XY", axis == EDITOR_AXIS_XY) ? EDITOR_AXIS_XY : axis;
				axis = nk_option_label(context, "YZ", axis == EDITOR_AXIS_YZ) ? EDITOR_AXIS_YZ : axis;
			}
			axis = nk_option_label(context, "None", axis == EDITOR_AXIS_NONE) ? EDITOR_AXIS_NONE : axis;
			if(axis != editor->current_axis) editor_axis_set(editor, axis);
			nk_combo_end(context);
		}
		
		nk_layout_row_push(context, 0.17f);
		vec3 camera_position = { 0.f, 0.f, 0.f };
		transform_get_absolute_position(&game_state->scene->cameras[CAM_EDITOR], &camera_position);
		static char position_text[32];
		snprintf(position_text, 32, "Camera: %.1f  %.1f  %.1f", camera_position.x, camera_position.y, camera_position.z);
		if(nk_combo_begin_label(context, position_text, nk_vec2(200, 125)))
		{
			nk_layout_row_dynamic(context, row_height, 1);
			editor->camera_move_speed = nk_propertyf(context, "Move Speed", 1.f, editor->camera_move_speed, 100.f, 1.f, 0.5f);
			editor->camera_turn_speed = nk_propertyf(context, "Turn Speed", 5.f, editor->camera_turn_speed, 200.f, 1.f, 0.5f);

			nk_combo_end(context);
		}
		
		nk_menubar_end(context);

		/* Tooltip for current action or entity being hovered */
		if(!nk_window_is_any_hovered(context))
		{
			nk_byte previous_opacity = context->style.window.background.a;
			context->style.window.background.a = 150;
			nk_flags alignment_flags_left = NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_LEFT;
			nk_flags alignment_flags_right = NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_RIGHT;
			nk_flags alignment_flags_center = NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED;
			float tooltip_width = 250.f;

			if(editor->hovered_entity)
			{
				if(nk_tooltip_begin(context, tooltip_width))
				{
					nk_layout_row_dynamic(context, 20, 2);
					nk_label(context, "Hovered Entity: ", alignment_flags_left); nk_label_colored(context, editor->hovered_entity->name, alignment_flags_right, nk_rgba_fv(&editor->hovered_entity_color));
					nk_label(context, "Hovered Entity Type: ", alignment_flags_left);   nk_label_colored(context, entity_type_name_get(editor->hovered_entity), alignment_flags_right, nk_rgba_fv(&editor->hovered_entity_color));
					nk_tooltip_end(context);
				}
			}

			if(editor->selected_entity && editor->current_axis != EDITOR_AXIS_NONE)
			{
				switch(editor->current_tool)
				{
				case EDITOR_TOOL_TRANSLATE:
				{
					if(nk_tooltip_begin(context, tooltip_width))
					{
						vec3 abs_pos_selected = { 0.f, 0.f, 0.f };
						vec3 abs_pos_cursor = { 0.f, 0.f, 0.f };
						transform_get_absolute_position(editor->selected_entity, &abs_pos_selected);
						transform_get_absolute_position(editor->cursor_entity, &abs_pos_cursor);
						nk_layout_row_dynamic(context, 20, 2);
						nk_label(context, "Current Position: ", alignment_flags_left); nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->selected_entity_color), "%.1f %.1f %.1f", abs_pos_selected.x, abs_pos_selected.y, abs_pos_selected.z);
						nk_label(context, "New Position: ", alignment_flags_left);     nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->cursor_entity_color), "%.1f %.1f %.1f", abs_pos_cursor.x, abs_pos_cursor.y, abs_pos_cursor.z);
						nk_tooltip_end(context);
					}
				}
				break;
				case EDITOR_TOOL_ROTATE:
				{
					if(editor->tool_rotate_rotation_started)
					{
						if(nk_tooltip_begin(context, tooltip_width))
						{
							nk_layout_row_dynamic(context, 20, 2);
							nk_label(context, "Axis: ", alignment_flags_left);
							switch(editor->current_axis)
							{
							case EDITOR_AXIS_X: nk_label_colored(context, "X", alignment_flags_right, nk_rgba_fv(&editor->axis_color_x)); break;
							case EDITOR_AXIS_Y: nk_label_colored(context, "Y", alignment_flags_right, nk_rgba_fv(&editor->axis_color_y)); break;
							case EDITOR_AXIS_Z: nk_label_colored(context, "Z", alignment_flags_right, nk_rgba_fv(&editor->axis_color_z)); break;
							}
							nk_label(context, "Current Rotation: ", alignment_flags_left); nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->selected_entity_color), "%.3f", editor->tool_rotate_starting_rotation);
							nk_label(context, "New Rotation: ", alignment_flags_left);     nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->cursor_entity_color), "%.3f", editor->tool_rotate_total_rotation);
							nk_tooltip_end(context);
						}
					}
				}
				break;
				case EDITOR_TOOL_SCALE:
				{
					if(editor->tool_scale_started)
					{
						if(nk_tooltip_begin(context, tooltip_width))
						{
							nk_layout_row_dynamic(context, 20, 4);
							nk_label(context, "Axis: ", alignment_flags_left);
							bool x_enabled = editor->current_axis == EDITOR_AXIS_X || editor->current_axis == EDITOR_AXIS_XZ || editor->current_axis == EDITOR_AXIS_XY ? true : false;
							bool y_enabled = editor->current_axis == EDITOR_AXIS_Y || editor->current_axis == EDITOR_AXIS_XY || editor->current_axis == EDITOR_AXIS_YZ ? true : false;
							bool z_enabled = editor->current_axis == EDITOR_AXIS_Z || editor->current_axis == EDITOR_AXIS_XZ || editor->current_axis == EDITOR_AXIS_YZ ? true : false;
							nk_label_colored(context, "X", alignment_flags_right, nk_rgba_f(editor->axis_color_x.x, editor->axis_color_x.y, editor->axis_color_x.z, x_enabled ? 1.f : 0.2f));
							nk_label_colored(context, "Y", alignment_flags_right, nk_rgba_f(editor->axis_color_y.x, editor->axis_color_y.y, editor->axis_color_y.z, y_enabled ? 1.f : 0.2f));
							nk_label_colored(context, "Z", alignment_flags_right, nk_rgba_f(editor->axis_color_z.x, editor->axis_color_z.y, editor->axis_color_z.z, z_enabled ? 1.f : 0.2f));

							nk_layout_row_dynamic(context, 20, 2);
							vec3 current_scale = { 1.f, 1.f, 1.f };
							vec3 cursor_entity_scale = { 1.f, 1.f, 1.f };
							vec3_assign(&current_scale, &editor->selected_entity->transform.scale);
							vec3_assign(&cursor_entity_scale, &editor->cursor_entity->base.transform.scale);
							nk_label(context, "Current Scale: ", alignment_flags_left); nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->selected_entity_color), "%.1f %.1f %.1f", current_scale.x, current_scale.y, current_scale.z);
							nk_label(context, "New Scale: ", alignment_flags_left);     nk_labelf_colored(context, alignment_flags_right, nk_rgba_fv(&editor->cursor_entity_color), "%.1f %.1f %.1f", cursor_entity_scale.x, cursor_entity_scale.y, cursor_entity_scale.z);
							nk_tooltip_end(context);
						}
					}
				}
				break;
				}
			}
			context->style.window.background.a = previous_opacity;
		}
	}
	nk_end(context);

	/* Status Bar */
	if(nk_begin(context, "Status Bar", nk_recti(0, win_height - editor->top_panel_height, win_width, editor->top_panel_height), NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_begin(context, NK_DYNAMIC, editor->top_panel_height - 5, 8);

		nk_layout_row_push(context, 0.06f);
		nk_label(context, "Selected: ", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);

		nk_layout_row_push(context, 0.06f);
		nk_label_colored(context, editor->selected_entity ? editor->selected_entity->name : "None", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, nk_rgba_fv(&editor->selected_entity_color));

		nk_layout_row_push(context, 0.1f);
		nk_checkbox_label(context, "Snap to grid ", &editor->tool_snap_enabled);

		nk_layout_row_push(context, 0.1f);
		nk_checkbox_label(context, "Relative grid ", &editor->grid_relative);

		nk_layout_row_push(context, 0.1f);
		nk_labelf(context, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, "Grid Scale: %.1f", editor->grid_scale);

		nk_layout_row_push(context, 0.1f);
		nk_labelf(context, NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE, "Grid Length: %d", editor->grid_num_lines);

		nk_layout_row_push(context, 0.38f);
		nk_spacing(context, 1);
		
		nk_layout_row_push(context, 0.1f);
		static int   frames = 0;
		static int   fps = 0;
		static float seconds = 0.f;
		seconds += dt;
		frames++;
		if(seconds >= 1.f)
		{
			fps = frames;
			seconds = 0.f;
			frames = 0;
		}
		nk_labelf(context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE, "FPS : %.d", fps);
		
	}
	nk_end(context);

	if(editor->window_scene_heirarchy) editor_window_scene_heirarchy(context, editor, game_state);
	if(editor->window_debug_variables) editor_window_debug_variables(context, editor);
	if(editor->window_property_inspector) editor_window_property_inspector(context, editor, game_state);
	if(editor->window_settings_renderer) editor_window_renderer_settings(context, editor, game_state);
	if(editor->window_settings_editor) editor_window_settings_editor(context, editor, game_state);
	
	if(editor->tool_mesh_draw_enabled)
	{
		switch(editor->current_tool)
		{
		case EDITOR_TOOL_SCALE:
		case EDITOR_TOOL_TRANSLATE:
		{
			quat rotation = { 0.f, 0.f, 0.f, 1.f };
			vec3 scale = { 1.f, 1.f, 1.f };
			vec3 position = { editor->cursor_entity->base.transform.position.x, editor->cursor_entity->base.transform.position.y, editor->cursor_entity->base.transform.position.z };

			//Draw Axes
			switch(editor->current_axis)
			{
			case EDITOR_AXIS_Y:
				im_line((vec3) { 0.f, -editor->axis_line_length, 0.f }, (vec3) { 0.f, editor->axis_line_length, 0.f }, position, rotation, editor->axis_color_y, 3);
				break;
			case EDITOR_AXIS_X:
				im_line((vec3) { -editor->axis_line_length, 0.f, 0.f }, (vec3) { editor->axis_line_length, 0.f, 0.f }, position, rotation, editor->axis_color_x, 3);
				break;
			case EDITOR_AXIS_Z:
				im_line((vec3) { 0.f, 0.f, -editor->axis_line_length }, (vec3) { 0.f, 0.f, editor->axis_line_length }, position, rotation, editor->axis_color_z, 3);
				break;
			case EDITOR_AXIS_XZ:
				im_line((vec3) { -editor->axis_line_length, 0.f, 0.f }, (vec3) { editor->axis_line_length, 0.f, 0.f }, position, rotation, editor->axis_color_x, 3);
				im_line((vec3) { 0.f, 0.f, -editor->axis_line_length }, (vec3) { 0.f, 0.f, editor->axis_line_length }, position, rotation, editor->axis_color_z, 3);
				break;
			case EDITOR_AXIS_XY:
				im_line((vec3) { -editor->axis_line_length, 0.f, 0.f }, (vec3) { editor->axis_line_length, 0.f, 0.f }, position, rotation, editor->axis_color_x, 3);
				im_line((vec3) { 0.f, -editor->axis_line_length, 0.f }, (vec3) { 0.f, editor->axis_line_length, 0.f }, position, rotation, editor->axis_color_y, 3);
				break;
			case EDITOR_AXIS_YZ:
				im_line((vec3) { 0.f, -editor->axis_line_length, 0.f }, (vec3) { 0.f, editor->axis_line_length, 0.f }, position, rotation, editor->axis_color_y, 3);
				im_line((vec3) { 0.f, 0.f, -editor->axis_line_length }, (vec3) { 0.f, 0.f, editor->axis_line_length }, position, rotation, editor->axis_color_z, 3);
				break;
			}
		}
		break;
		case EDITOR_TOOL_ROTATE:
		{
			quat rotation = { 0.f, 0.f, 0.f, 1.f };
			vec3 scale = { 1.f, 1.f, 1.f };
			vec3 position = { editor->cursor_entity->base.transform.position.x, editor->cursor_entity->base.transform.position.y, editor->cursor_entity->base.transform.position.z };
			switch(editor->current_axis)
			{
			case EDITOR_AXIS_X:
				quat_axis_angle(&rotation, &UNIT_Y, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_x, 3);
				break;
			case EDITOR_AXIS_Y:
				quat_axis_angle(&rotation, &UNIT_X, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_y, 3);
				break;
			case EDITOR_AXIS_Z:
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_z, 3);
				break;
			case EDITOR_AXIS_XZ:
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_z, 3);
				quat_axis_angle(&rotation, &UNIT_Y, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_x, 3);
				break;
			case EDITOR_AXIS_XY:
				quat_axis_angle(&rotation, &UNIT_Y, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_x, 3);
				quat_identity(&rotation);
				quat_axis_angle(&rotation, &UNIT_X, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_y, 3);
				break;
			case EDITOR_AXIS_YZ:
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_z, 3);
				quat_axis_angle(&rotation, &UNIT_X, -90.f);
				im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, false, position, rotation, editor->axis_color_y, 3);
				break;
			}

			if(editor->current_axis != EDITOR_AXIS_NONE)
			{
				quat_identity(&rotation);
				vec4 arc_color = { 1.f, 1.f, 1.f, 1.f };
				switch(editor->current_axis)
				{
				case EDITOR_AXIS_X: quat_axis_angle(&rotation, &UNIT_Y, -90.f); vec4_assign(&arc_color, &editor->axis_color_x); break;
				case EDITOR_AXIS_Y: quat_axis_angle(&rotation, &UNIT_X, 90.f);  vec4_assign(&arc_color, &editor->axis_color_y); break;
				case EDITOR_AXIS_Z: quat_axis_angle(&rotation, &UNIT_Y, 180.f); vec4_assign(&arc_color, &editor->axis_color_z); break;
				}

				if(editor->tool_rotate_allowed)
				{
					arc_color.w = 0.1f;
					im_circle(editor->tool_rotate_arc_radius, editor->tool_rotate_arc_segments, true, position, rotation, arc_color, 2);
				}

				if(editor->tool_rotate_total_rotation != 0.f)
				{
					arc_color.w = 0.5f;
					im_arc(editor->tool_rotate_arc_radius / 2.f, editor->tool_rotate_starting_rotation, editor->tool_rotate_total_rotation, editor->tool_rotate_arc_segments, true, position, rotation, arc_color, 4);
				}
			}
		}
		break;

		}
	}

}

void editor_on_mousebutton_release(const struct Event* event)
{
	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;

	if(game_state->game_mode != GAME_MODE_EDITOR || nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

	if(editor->camera_looking_around)
	{
		input_mouse_mode_set(MM_NORMAL);
		int width = 0, height = 0;
		window_get_drawable_size(game_state_get()->window, &width, &height);
		platform_mouse_position_set(game_state_get()->window, width / 2, height / 2);
		editor->camera_looking_around = false;

		if(editor->selected_entity && editor->current_tool == EDITOR_TOOL_ROTATE && editor->previous_axis != EDITOR_AXIS_NONE)
			editor_axis_set(editor, editor->previous_axis);
	}

	if(event->mousebutton.button == MSB_LEFT &&
		!editor->camera_looking_around &&
		nk_item_is_any_active(&gui->context) == 0)
	{
		if(editor->picking_enabled)
		{
			log_message("Editor Picking");
			struct Camera* editor_camera = &game_state_get()->scene->cameras[CAM_EDITOR];
			int mouse_x = 0, mouse_y = 0;
			platform_mouse_position_get(&mouse_x, &mouse_y);
			struct Ray ray = camera_screen_coord_to_ray(editor_camera, mouse_x, mouse_y);

			struct Scene* scene = game_state_get()->scene;
			struct Raycast_Result ray_result;
			scene_ray_intersect(scene, &ray, &ray_result);

			if(ray_result.num_entities_intersected > 0)
			{
				//For now, just select the first entity that is intersected 
				struct Entity* intersected_entity = ray_result.entities_intersected[0];
				if(intersected_entity == editor->cursor_entity)
				{
					if(ray_result.num_entities_intersected > 1)
						intersected_entity = ray_result.entities_intersected[1];
					else
						intersected_entity = NULL;
				}
				if(intersected_entity) editor_entity_select(editor, intersected_entity);
			}
			else
			{
				//Deselect the currently selected entity if nothing was found
				if(editor->current_axis == EDITOR_AXIS_NONE)
					editor_entity_select(editor, NULL);
			}
		}

	}

	if(editor->selected_entity && event->mousebutton.button == MSB_LEFT && nk_item_is_any_active(&gui->context) == 0)
	{
		switch(editor->current_tool)
		{
		case EDITOR_TOOL_TRANSLATE:
			if(editor->current_axis != EDITOR_AXIS_NONE)
				transform_copy(editor->selected_entity, editor->cursor_entity, false);
			break;
		case EDITOR_TOOL_ROTATE:
			if(editor->tool_rotate_rotation_started)
			{
				editor->picking_enabled = true;
				editor->tool_rotate_rotation_started = false;
				transform_copy(editor->selected_entity, editor->cursor_entity, false);
				editor->tool_rotate_total_rotation = 0.f;
				editor->tool_rotate_starting_rotation = 0.f;
				editor->draw_cursor_entity = false;
			}
			break;
		case EDITOR_TOOL_SCALE:
			if(editor->tool_scale_started)
			{
				editor->picking_enabled = true;
				editor->tool_scale_started = false;
				transform_copy(editor->selected_entity, editor->cursor_entity, false);
				vec3_fill(&editor->tool_scale_amount, 1.f, 1.f, 1.f);
				editor->draw_cursor_entity = false;
			}
			else
			{
				if(editor->current_axis != EDITOR_AXIS_NONE)
				{
					editor->tool_scale_started = true;
					editor->picking_enabled = false;
					editor->draw_cursor_entity = true;
					vec3_assign(&editor->tool_scale_amount, &editor->cursor_entity->base.transform.scale);
				}
			}
			break;
		}
	}
}

void editor_on_mousebutton_press(const struct Event* event)
{
	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;
	if(game_state->game_mode != GAME_MODE_EDITOR || nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

	if(event->mousebutton.button == MSB_LEFT && editor->selected_entity)
	{
		if(editor->current_tool == EDITOR_TOOL_ROTATE && editor->tool_rotate_allowed)
		{
			/* Check if there's an entity under cursor, if there is then select it,
			   otherwise disable picking and start rotating */
			struct Scene* scene = game_state_get()->scene;
			struct Camera* editor_camera = &scene->cameras[CAM_EDITOR];
			int mouse_x = 0, mouse_y = 0;
			platform_mouse_position_get(&mouse_x, &mouse_y);
			struct Ray ray = camera_screen_coord_to_ray(editor_camera, mouse_x, mouse_y);

			struct Raycast_Result ray_result;
			scene_ray_intersect(scene, &ray, &ray_result);

			if(ray_result.num_entities_intersected > 0)
			{
				//For now, just select the first entity that is intersected 
				struct Entity* intersected_entity = ray_result.entities_intersected[0];
				if(intersected_entity == editor->cursor_entity)
				{
					if(ray_result.num_entities_intersected > 1)
					{
						intersected_entity = ray_result.entities_intersected[1];
						if(intersected_entity) 
							editor_entity_select(editor, intersected_entity);
					}
				}
			}
			else
			{
				editor->picking_enabled = false;
				editor->tool_rotate_rotation_started = true;
				editor->tool_rotate_total_rotation = 0.f;
				editor->draw_cursor_entity = true;
				switch(editor->current_axis)
				{
				case EDITOR_AXIS_X: editor->tool_rotate_starting_rotation = roundf(quat_get_pitch(&editor->cursor_entity->base.transform.rotation)); break;
				case EDITOR_AXIS_Y: editor->tool_rotate_starting_rotation = roundf(quat_get_yaw(&editor->cursor_entity->base.transform.rotation));   break;
				case EDITOR_AXIS_Z: editor->tool_rotate_starting_rotation = roundf(quat_get_roll(&editor->cursor_entity->base.transform.rotation));  break;
				}
			}
		}
	}

	/* Cancel rotation on right mouse press */
	if(event->mousebutton.button == MSB_RIGHT && editor->selected_entity && editor->current_tool == EDITOR_TOOL_ROTATE)
		editor_tool_reset(editor);
}

void editor_on_mousemotion(const struct Event* event)
{
	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;
	if(game_state->game_mode != GAME_MODE_EDITOR || nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

	switch(editor->current_tool)
	{
	case EDITOR_TOOL_NORMAL:
	{

	}
	break;
	case EDITOR_TOOL_TRANSLATE:
	{
		if(editor->selected_entity)
		{
			struct Camera* editor_camera = &game_state->scene->cameras[CAM_EDITOR];
			vec3 current_position = { 0.f, 0.f, 0.f };
			vec3 cursor_entity_position;
			vec3_assign(&cursor_entity_position, &editor->cursor_entity->base.transform.position);
			transform_get_absolute_position(editor->selected_entity, &current_position);
			struct Ray cam_ray;
			cam_ray = camera_screen_coord_to_ray(editor_camera, event->mousemotion.x, event->mousemotion.y);

			switch(editor->current_axis)
			{
			case EDITOR_AXIS_X: cursor_entity_position.x +=  event->mousemotion.xrel / 2; break;
			case EDITOR_AXIS_Y: cursor_entity_position.y += -event->mousemotion.xrel / 2; break;
			case EDITOR_AXIS_Z: cursor_entity_position.z += -event->mousemotion.xrel / 2; break;
			case EDITOR_AXIS_XZ:
			{
				Plane ground_plane;
				plane_init(&ground_plane, &(vec3){0.f, 1.f, 0.f}, &current_position);

				float distance = bv_distance_ray_plane(&cam_ray, &ground_plane);
				if(distance < INFINITY && distance > -INFINITY)
				{
					vec3 abs_cam_pos, projected_point, cam_forward;
					transform_get_absolute_position(editor_camera, &abs_cam_pos);
					transform_get_absolute_forward(editor_camera, &cam_forward);
					vec3_scale(&projected_point, &cam_ray.direction, distance);
					vec3_add(&current_position, &projected_point, &abs_cam_pos);
					vec3_assign(&cursor_entity_position, &current_position);
				}
			}
			break;
			}

			if(editor->tool_snap_enabled)
			{
				cursor_entity_position.x = roundf(cursor_entity_position.x / editor->grid_scale) * editor->grid_scale;
				cursor_entity_position.y = roundf(cursor_entity_position.y / editor->grid_scale) * editor->grid_scale;
				cursor_entity_position.z = roundf(cursor_entity_position.z / editor->grid_scale) * editor->grid_scale;
				transform_set_position(editor->cursor_entity, &cursor_entity_position);
			}
		}
	}
	break;
	case EDITOR_TOOL_ROTATE:
	{
		if(editor->selected_entity && editor->current_axis < EDITOR_AXIS_XZ)
		{
			struct Camera* editor_camera = &game_state->scene->cameras[CAM_EDITOR];
			vec3 position = { 0.f };
			transform_get_absolute_position(editor->selected_entity, &position);
			struct Ray cam_ray;
			cam_ray = camera_screen_coord_to_ray(editor_camera, event->mousemotion.x, event->mousemotion.y);


			/* Instead of using a spehre intersection, get the point where the ray intersects the plane 
			   then check if the distance of that point from the selected entity is less than or equal to
			   the radius of the circle/disc, if it is, then we are inside the circle and can rotate */



			struct Bounding_Sphere gizmo_sphere;
			//vec3_assign(&gizmo_sphere.center, &position);
			gizmo_sphere.center = (vec3) { 0.f, 0.f, 0.f };
			gizmo_sphere.radius = editor->tool_rotate_arc_radius;
			if(bv_intersect_sphere_ray(&gizmo_sphere, &position, &cam_ray))
			{
				//Plane ground_plane;
				//plane_init(&ground_plane, &UNIT_X, &position);
				//float distance_x = bv_distance_ray_plane(&cam_ray, &ground_plane);

				//plane_init(&ground_plane, &UNIT_Y, &position);
				//float distance_y = bv_distance_ray_plane(&cam_ray, &ground_plane);

				//plane_init(&ground_plane, &UNIT_Z, &position);
				//float distance_z = bv_distance_ray_plane(&cam_ray, &ground_plane);

				////Determine the closest plane
				////log_message("X: %.3f  Y: %.3f  Z: %.3f", distance_x, distance_y, distance_z);

				//float shortest_distance = distance_x < distance_y ? distance_x : distance_y;
				//shortest_distance = shortest_distance < distance_z ? shortest_distance : distance_z;

				//if(shortest_distance == distance_x) editor->tool_rotate_axis = EDITOR_AXIS_X;
				//if(shortest_distance == distance_y) editor->tool_rotate_axis = EDITOR_AXIS_Y;
				//if(shortest_distance == distance_z) editor->tool_rotate_axis = EDITOR_AXIS_Z;
				editor->tool_rotate_allowed = true;
			}
			else
			{
				editor->tool_rotate_allowed = false;
			}

			if(editor->current_axis != EDITOR_AXIS_NONE && editor->tool_rotate_rotation_started)
			{
				if(editor->tool_snap_enabled)
					editor->tool_rotate_amount += editor->grid_scale * editor->tool_rotate_increment * ((float)event->mousemotion.xrel / 2.f);
				else
					editor->tool_rotate_amount += event->mousemotion.xrel / 2;

				if(editor->tool_rotate_amount > 360.f)
					editor->tool_rotate_amount = editor->tool_rotate_amount - 360.f;
				else if(editor->tool_rotate_amount < -360.f)
					editor->tool_rotate_amount = editor->tool_rotate_amount + 360.f;

				if(editor->tool_rotate_amount != 0.f)
				{
					switch(editor->current_axis)
					{
					case EDITOR_AXIS_X:  
						transform_rotate(editor->cursor_entity, &UNIT_X, editor->tool_rotate_amount, TS_WORLD);
						editor->tool_rotate_total_rotation = roundf(quat_get_pitch(&editor->cursor_entity->base.transform.rotation)); 
						break;
					case EDITOR_AXIS_Y:  
						transform_rotate(editor->cursor_entity, &UNIT_Y, editor->tool_rotate_amount, TS_WORLD);
						editor->tool_rotate_total_rotation = roundf(quat_get_yaw(&editor->cursor_entity->base.transform.rotation)); 
						break;
					case EDITOR_AXIS_Z: 
						transform_rotate(editor->cursor_entity, &UNIT_Z, editor->tool_rotate_amount, TS_WORLD);
						editor->tool_rotate_total_rotation = roundf(quat_get_roll(&editor->cursor_entity->base.transform.rotation)); 
						break;
					}
					editor->tool_rotate_amount = 0.f;
				}
			}
		}
		
	}
	break;
	case EDITOR_TOOL_SCALE:
	{
		if(editor->current_axis != EDITOR_AXIS_NONE && editor->tool_scale_started)
		{
			switch(editor->current_axis)
			{
			case EDITOR_AXIS_X:  editor->tool_scale_amount.x += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			case EDITOR_AXIS_Y:  editor->tool_scale_amount.y += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			case EDITOR_AXIS_Z:  editor->tool_scale_amount.z += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			case EDITOR_AXIS_XZ: editor->tool_scale_amount.x += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; editor->tool_scale_amount.z += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			case EDITOR_AXIS_XY: editor->tool_scale_amount.x += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; editor->tool_scale_amount.y += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			case EDITOR_AXIS_YZ: editor->tool_scale_amount.y += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; editor->tool_scale_amount.z += (float)(event->mousemotion.xrel / 2) * editor->grid_scale; break;
			}
			transform_scale(editor->cursor_entity, &editor->tool_scale_amount);
		}
	}
	break;
	default: break;
	}

	/* Check if we're hovering over an entity */
	struct Scene* scene = game_state->scene;
	struct Camera* editor_camera = &scene->cameras[CAM_EDITOR];
	struct Ray ray = camera_screen_coord_to_ray(editor_camera, event->mousemotion.x, event->mousemotion.y);
	struct Raycast_Result ray_result;
	scene_ray_intersect(scene, &ray, &ray_result);

	if(ray_result.num_entities_intersected > 0)
	{
		struct Entity* intersected_entity = ray_result.entities_intersected[0];
		if(intersected_entity == editor->cursor_entity)
		{
			if(ray_result.num_entities_intersected > 1)
				intersected_entity = ray_result.entities_intersected[1];
			else
				intersected_entity = NULL;
		}
		if(intersected_entity && intersected_entity != editor->hovered_entity)
			editor->hovered_entity = intersected_entity;
		else if(editor->hovered_entity)
			editor->hovered_entity = NULL;
	}
	else
	{
		if(editor->hovered_entity)
			editor->hovered_entity = NULL;
	}
}

void editor_on_key_release(const struct Event* event)
{
	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;
	if(game_state->game_mode != GAME_MODE_EDITOR || nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

	/* Tool Cycle */
	if(event->key.key == KEY_TAB)
	{
		int tool = editor->current_tool;
		if(++tool == EDITOR_TOOL_MAX)
			tool = EDITOR_TOOL_NORMAL;
		editor_tool_set(editor, tool);
	}

	/* Tool Select */
	if(!editor->camera_looking_around)
	{
		if(event->key.key == KEY_Q) editor_tool_set(editor, EDITOR_TOOL_NORMAL);
		if(event->key.key == KEY_W) editor_tool_set(editor, EDITOR_TOOL_TRANSLATE);
		if(event->key.key == KEY_E) editor_tool_set(editor, EDITOR_TOOL_ROTATE);
		if(event->key.key == KEY_R) editor_tool_set(editor, EDITOR_TOOL_SCALE);
	}

	/* Axis select */
	int selected_axis = editor->current_axis;
	if(event->key.key == KEY_X) editor_axis_set(editor, EDITOR_AXIS_X);
	if(event->key.key == KEY_Y) editor_axis_set(editor, EDITOR_AXIS_Y);
	if(event->key.key == KEY_Z) editor_axis_set(editor, EDITOR_AXIS_Z);
	if(event->key.key == KEY_X && input_is_key_pressed(KEY_LSHIFT)) editor_axis_set(editor, EDITOR_AXIS_YZ);
	if(event->key.key == KEY_Y && input_is_key_pressed(KEY_LSHIFT)) editor_axis_set(editor, EDITOR_AXIS_XZ);
	if(event->key.key == KEY_Z && input_is_key_pressed(KEY_LSHIFT)) editor_axis_set(editor, EDITOR_AXIS_XY);
	if(event->key.key == KEY_ALT && editor->current_tool == EDITOR_TOOL_TRANSLATE && editor->current_axis == EDITOR_AXIS_Y)
	{
		if(editor->previous_axis != EDITOR_AXIS_Y)
			editor_axis_set(editor, editor->previous_axis);
		else
			editor_axis_set(editor, EDITOR_AXIS_NONE);
		editor->previous_axis = EDITOR_AXIS_NONE;
	}

	/* Grid Scale select */
	if(event->key.key == KEY_1) editor->grid_scale = 1.f;
	if(event->key.key == KEY_2) editor->grid_scale = 2.f;
	if(event->key.key == KEY_3) editor->grid_scale = 3.f;
	if(event->key.key == KEY_4) editor->grid_scale = 4.f;
	if(event->key.key == KEY_5) editor->grid_scale = 5.f;
	if(event->key.key == KEY_6) editor->grid_scale = 6.f;
	if(event->key.key == KEY_7) editor->grid_scale = 7.f;
	if(event->key.key == KEY_8) editor->grid_scale = 8.f;
	if(event->key.key == KEY_9) editor->grid_scale = 9.f;
	if(event->key.key == KEY_0) editor->grid_scale = 0.5f;

	if(event->key.key == KEY_G) editor->grid_enabled = !editor->grid_enabled;
	if(event->key.key == KEY_ESCAPE) editor_entity_select(editor, NULL);

	if(event->key.key == KEY_DELETE && editor->selected_entity)
	{
		editor->selected_entity->marked_for_deletion = true;
		editor_entity_select(editor, NULL);
	}
}

void editor_tool_set(struct Editor* editor, int tool)
{
	if(editor->current_tool != tool)
	{
		editor->current_tool = tool;
		if(editor->current_tool == EDITOR_TOOL_TRANSLATE)
			editor->draw_cursor_entity = true;
		else
			editor->draw_cursor_entity = false;
		editor->previous_axis = editor->current_axis;
		editor_tool_reset(editor);
	}
}

void editor_entity_select(struct Editor* editor, struct Entity* entity)
{
	if(!entity && editor->selected_entity) // Deselect
	{
		editor->selected_entity->selected_in_editor = false;
		editor->selected_entity = NULL;
		editor_tool_reset(editor);
	}
	else if(entity) // Select
	{
		if(editor->selected_entity && editor->selected_entity != entity)
		{
			editor->selected_entity->selected_in_editor = false;
			editor->selected_entity = NULL;
		}

		if(editor->current_tool == EDITOR_TOOL_TRANSLATE)
		{
			if(editor->current_axis == EDITOR_AXIS_NONE)
				editor_axis_set(editor, EDITOR_AXIS_XZ);
			editor->draw_cursor_entity = true;
		}
		entity->selected_in_editor = true;
		editor->selected_entity = entity;
		transform_copy(editor->cursor_entity, editor->selected_entity, false);
	}
}

void editor_tool_reset(struct Editor* editor)
{
	if(editor->selected_entity)
		transform_copy(editor->cursor_entity, editor->selected_entity, false);
	else
		transform_reset(editor->cursor_entity);

	switch(editor->current_tool)
	{
	case EDITOR_TOOL_NORMAL:
		editor->draw_cursor_entity = false;
		break;
	case EDITOR_TOOL_TRANSLATE:
		if(editor->current_axis != EDITOR_AXIS_XZ)
			editor_axis_set(editor, EDITOR_AXIS_XZ);
		editor->draw_cursor_entity = true;
		break;
	case EDITOR_TOOL_ROTATE:
		editor->tool_rotate_amount = 0.f;
		editor->tool_rotate_total_rotation = 0.f;
		editor->tool_rotate_allowed = false;
		editor->tool_rotate_rotation_started = false;
		editor->draw_cursor_entity = false;
		editor_axis_set(editor, EDITOR_AXIS_NONE);
		break;
	case EDITOR_TOOL_SCALE:
		vec3_fill(&editor->tool_scale_amount, 1.f, 1.f, 1.f);
		editor->tool_scale_started = false;
		editor->draw_cursor_entity = false;
		editor_axis_set(editor, EDITOR_AXIS_NONE);
		break;
	}

	editor->picking_enabled = true;
}

void editor_on_key_press(const struct Event* event)
{
	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;
	if(game_state->game_mode != GAME_MODE_EDITOR || nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

	if(!nk_window_is_any_hovered(&game_state_get()->gui->context))
	{
		if(event->key.key == KEY_ALT && editor->current_tool == EDITOR_TOOL_TRANSLATE && editor->current_axis != EDITOR_AXIS_Y) editor_axis_set(editor, EDITOR_AXIS_Y);
	}
}

void editor_axis_set(struct Editor* editor, int axis)
{
	if(editor->current_axis != axis)
	{
		editor->previous_axis = editor->current_axis;
		editor->current_axis = axis;

		/* Reset tool position after axis has changed */
		if(editor->selected_entity)
			transform_copy(editor->cursor_entity, editor->selected_entity, false);

		if(editor->current_tool == EDITOR_TOOL_ROTATE)
		{
			// Assign rotation axis only if it is a single axis since we don't want to rotate on multiple axes at the same time
			if(axis >= EDITOR_AXIS_XZ)
			{
				editor->previous_axis = EDITOR_AXIS_NONE;
				editor->current_axis = EDITOR_AXIS_NONE;
				editor->tool_rotate_allowed = false;
				editor->tool_rotate_amount = 0.f;
			}
		}

		if(editor->current_tool == EDITOR_TOOL_SCALE && axis != EDITOR_AXIS_NONE)
		{
			editor->tool_scale_started = true;
			editor->picking_enabled = false;
			editor->draw_cursor_entity = true;
			vec3_assign(&editor->tool_scale_amount, &editor->cursor_entity->base.transform.scale);
		}
	}
	else
	{
		if(axis == EDITOR_AXIS_NONE)
			return;
		/* De-select axis */
		editor->previous_axis = editor->current_axis;
		editor->current_axis = EDITOR_AXIS_NONE;

		if(editor->selected_entity)
			transform_copy(editor->cursor_entity, editor->selected_entity, false);
		
		if(editor->current_tool == EDITOR_TOOL_SCALE)
		{
			editor->picking_enabled = true;
			editor->tool_scale_started = false;
			vec3_fill(&editor->tool_scale_amount, 1.f, 1.f, 1.f);
			editor->draw_cursor_entity = false;
		}
	}
}

void editor_camera_update(struct Editor* editor, float dt)
{
	struct Game_State* game_state = game_state_get();
	struct Gui*        gui        = game_state->gui;
	if(nk_window_is_any_hovered(&gui->context) || game_state->console->visible)
		return;

    struct Camera* editor_camera = &game_state_get()->scene->cameras[CAM_EDITOR];
    static float total_up_down_rot = 0.f;
    float move_speed = editor->camera_move_speed, turn_speed = editor->camera_turn_speed;
    float turn_up_down        = 0.f;
    float turn_left_right     = 0.f;
    float max_up_down         = 60.f;
	vec3  offset              = { 0, 0, 0 };
	vec3  rot_axis_up_down    = { 1, 0, 0 };
	vec3  rot_axis_left_right = { 0, 1, 0 };

    /* Look around */
    if(input_map_state_get("Turn_Up",    KS_PRESSED)) turn_up_down    += turn_speed;
    if(input_map_state_get("Turn_Down",  KS_PRESSED)) turn_up_down    -= turn_speed;
    if(input_map_state_get("Turn_Right", KS_PRESSED)) turn_left_right += turn_speed;
    if(input_map_state_get("Turn_Left",  KS_PRESSED)) turn_left_right -= turn_speed;

    if(input_mousebutton_state_get(MSB_RIGHT, KS_PRESSED))
    {
		const float scale = 0.1f;
		int cursor_lr, cursor_ud;
		input_mouse_delta_get(&cursor_lr, &cursor_ud);
		editor->camera_looking_around = true;
		if(input_mouse_mode_get() != MM_RELATIVE)
		{
			input_mouse_mode_set(MM_RELATIVE);
			cursor_lr = cursor_ud = 0;
		}

		turn_up_down = -cursor_ud * turn_speed * dt * scale;
		turn_left_right = cursor_lr * turn_speed * dt * scale;
    }
    else
    {
		turn_up_down *= dt;
		turn_left_right *= dt;
    }

    total_up_down_rot += turn_up_down;
    if(total_up_down_rot >= max_up_down)
    {
		total_up_down_rot = max_up_down;
		turn_up_down = 0.f;
    }
    else if(total_up_down_rot <= -max_up_down)
    {
		total_up_down_rot = -max_up_down;
		turn_up_down = 0.f;
    }

    if(turn_left_right != 0.f)
    {
		transform_rotate(editor_camera, &rot_axis_left_right, -turn_left_right, TS_WORLD);
    }

    if(turn_up_down != 0.f)
    {
		//transform_rotate(editor_camera, &rot_axis_up_down, turn_up_down, TS_LOCAL);
		transform_rotate(editor_camera, &rot_axis_up_down, turn_up_down, TS_LOCAL);
    }

    /* Movement */
	if(editor->camera_looking_around)
	{
		if(input_map_state_get("Sprint", KS_PRESSED)) move_speed *= editor->camera_sprint_multiplier;
		if(input_map_state_get("Move_Forward", KS_PRESSED)) offset.z -= move_speed;
		if(input_map_state_get("Move_Backward", KS_PRESSED)) offset.z += move_speed;
		if(input_map_state_get("Move_Left", KS_PRESSED)) offset.x -= move_speed;
		if(input_map_state_get("Move_Right", KS_PRESSED)) offset.x += move_speed;
		if(input_map_state_get("Move_Up", KS_PRESSED)) offset.y += move_speed;
		if(input_map_state_get("Move_Down", KS_PRESSED)) offset.y -= move_speed;

		vec3_scale(&offset, &offset, dt);
		if(offset.x != 0 || offset.y != 0 || offset.z != 0)
		{
			transform_translate(editor_camera, &offset, TS_LOCAL);
			//log_message("Position : %s", tostr_vec3(&transform->position));
		}
	}
}

void editor_widget_color_combov3(struct nk_context* context, vec3* color, int width, int height)
{
	struct nk_color temp_color = nk_rgba_f(color->x, color->y, color->z, 1.f);
	if(nk_combo_begin_color(context, temp_color, nk_vec2(width, height)))
	{
		enum color_mode { COL_RGB, COL_HSV };
		static int col_mode = COL_RGB;
		nk_layout_row_dynamic(context, 25, 2);
		col_mode = nk_option_label(context, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
		col_mode = nk_option_label(context, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;
		nk_layout_row_dynamic(context, 120, 1);
		struct nk_colorf temp_colorf = nk_color_cf(temp_color);
		temp_colorf = nk_color_picker(context, temp_colorf, NK_RGB);
		temp_color = nk_rgba_cf(temp_colorf);

		nk_layout_row_dynamic(context, 25, 1);
		if(col_mode == COL_RGB)
		{
			temp_color.r = (nk_byte)nk_propertyi(context, "#R:", 0, temp_color.r, 255, 1, 1);
			temp_color.g = (nk_byte)nk_propertyi(context, "#G:", 0, temp_color.g, 255, 1, 1);
			temp_color.b = (nk_byte)nk_propertyi(context, "#B:", 0, temp_color.b, 255, 1, 1);
		}
		else
		{
			nk_byte tmp[4];
			nk_color_hsva_bv(tmp, temp_color);
			tmp[0] = (nk_byte)nk_propertyi(context, "#H:", 0, tmp[0], 255, 1, 1);
			tmp[1] = (nk_byte)nk_propertyi(context, "#S:", 0, tmp[1], 255, 1, 1);
			tmp[2] = (nk_byte)nk_propertyi(context, "#V:", 0, tmp[2], 255, 1, 1);
			temp_color = nk_hsva_bv(tmp);
		}
		float empty = 1.f;
		nk_color_f(&color->x, &color->y, &color->z, &empty, temp_color);
		nk_combo_end(context);
	}
}

void editor_widget_color_combov4(struct nk_context* context, vec4* color, int width, int height)
{
	struct nk_color temp_color = nk_rgba_f(color->x, color->y, color->z, color->w);
	if(nk_combo_begin_color(context, temp_color, nk_vec2(width, height)))
	{
		enum color_mode { COL_RGB, COL_HSV };
		static int col_mode = COL_RGB;
		nk_layout_row_dynamic(context, 25, 2);
		col_mode = nk_option_label(context, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
		col_mode = nk_option_label(context, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;
		nk_layout_row_dynamic(context, 120, 1);
		struct nk_colorf temp_colorf = nk_color_cf(temp_color);
		temp_colorf = nk_color_picker(context, temp_colorf, NK_RGBA);
		temp_color = nk_rgba_cf(temp_colorf);

		nk_layout_row_dynamic(context, 25, 1);
		if(col_mode == COL_RGB)
		{
			temp_color.r = (nk_byte)nk_propertyi(context, "#R:", 0, temp_color.r, 255, 1, 1);
			temp_color.g = (nk_byte)nk_propertyi(context, "#G:", 0, temp_color.g, 255, 1, 1);
			temp_color.b = (nk_byte)nk_propertyi(context, "#B:", 0, temp_color.b, 255, 1, 1);
			temp_color.a = (nk_byte)nk_propertyi(context, "#A:", 0, temp_color.a, 255, 1, 1);
		}
		else
		{
			nk_byte tmp[4];
			nk_color_hsva_bv(tmp, temp_color);
			tmp[0] = (nk_byte)nk_propertyi(context, "#H:", 0, tmp[0], 255, 1, 1);
			tmp[1] = (nk_byte)nk_propertyi(context, "#S:", 0, tmp[1], 255, 1, 1);
			tmp[2] = (nk_byte)nk_propertyi(context, "#V:", 0, tmp[2], 255, 1, 1);
			tmp[3] = (nk_byte)nk_propertyi(context, "#A:", 0, tmp[3], 255, 1, 1);
			temp_color = nk_hsva_bv(tmp);
		}
		nk_color_f(&color->x, &color->y, &color->z, &color->w, temp_color);
		nk_combo_end(context);
	}
}

void editor_cleanup(struct Editor* editor)
{
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_PRESSED, &editor_on_mousebutton_press);
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_RELEASED, &editor_on_mousebutton_release);
    for(int i = 0; i < array_len(debug_vars_list); i++)
	editor_debugvar_slot_remove(i);
    array_free(debug_vars_list);
    array_free(empty_indices);
}



bool editor_widget_v3(struct nk_context* context, vec3* value, const char* name_x, const char* name_y, const char* name_z, float min, float max, float step, float inc_per_pixel, int row_height)
{
    bool changed = false;
    vec3 val_copy = {0.f, 0.f, 0.f};
    vec3_assign(&val_copy, value);
    nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, name_x, min, &value->x, max, step, inc_per_pixel);
    nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, name_y, min, &value->y, max, step, inc_per_pixel);
    nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, name_z, min, &value->z, max, step, inc_per_pixel);
    if(!vec3_equals(&val_copy, value)) changed = true;
    return changed;
}


void editor_show_entity_in_list(struct Editor* editor, struct nk_context* context, struct Scene* scene, struct Entity* entity)
{
	if(!entity->active) return;

	nk_layout_row_dynamic(context, 20, 1);
	if(nk_selectable_label(context, entity->name, NK_TEXT_ALIGN_LEFT, &entity->selected_in_editor))
	{
		if(editor->selected_entity && editor->selected_entity != entity)
			editor_entity_select(editor, entity);
		else if(editor->selected_entity && editor->selected_entity == entity && !entity->selected_in_editor)
			editor_entity_select(editor, NULL);

		if(entity->selected_in_editor)
		{
			editor->selected_entity = entity;
			if(!editor->window_property_inspector) editor->window_property_inspector = true;
		}
	}
}

void editor_window_scene_heirarchy(struct nk_context* context, struct Editor* editor, struct Game_State* game_state)
{
	if(nk_begin(context, "Scene Heirarchy", nk_recti(0, editor->top_panel_height, 300, 400), window_flags))
	{
		struct Scene* scene = game_state_get()->scene;
		nk_layout_row_dynamic(context, 350, 1);
		if(nk_group_begin(context, "Entity Name", NK_WINDOW_SCROLL_AUTO_HIDE))
		{

			for(int i = 0; i < MAX_ENTITIES; i++)      editor_show_entity_in_list(editor, context, scene, &scene->entities[i]);
			for(int i = 0; i < MAX_CAMERAS; i++)       editor_show_entity_in_list(editor, context, scene, &scene->cameras[i]);
			for(int i = 0; i < MAX_LIGHTS; i++)        editor_show_entity_in_list(editor, context, scene, &scene->lights[i]);
			for(int i = 0; i < MAX_STATIC_MESHES; i++) editor_show_entity_in_list(editor, context, scene, &scene->static_meshes[i]);

			nk_group_end(context);
		}
	}
	else
	{
		editor->window_scene_heirarchy = false;
	}
	nk_end(context);
}

void editor_window_debug_variables(struct nk_context* context, struct Editor* editor)
{
	if(nk_begin(context, "Debug Variables", nk_recti(0, 500, 300, 400), window_flags))
	{
		static char variant_str[MAX_VARIANT_STR_LEN] = { '\0' };
		nk_layout_row_dynamic(context, 250, 1);
		if(nk_group_begin(context, "Name", NK_WINDOW_SCROLL_AUTO_HIDE))
		{
			for(int i = 0; i < array_len(debug_vars_list); i++)
			{
				struct Debug_Variable* debug_var = &debug_vars_list[i];
				if(debug_var->data.type == VT_NONE) continue;
				nk_layout_row_dynamic(context, 20, 2);
				nk_label(context, debug_var->name, NK_TEXT_ALIGN_LEFT);
				variant_to_str(&debug_var->data, variant_str, MAX_VARIANT_STR_LEN);
				nk_label(context, variant_str, NK_TEXT_ALIGN_RIGHT);
				memset(variant_str, '\0', MAX_VARIANT_STR_LEN);
			}
			nk_group_end(context);
		}
	}
	else
	{
		editor->window_debug_variables = false;
	}
	nk_end(context);
}

void editor_window_property_inspector(struct nk_context* context, struct Editor* editor, struct Game_State* game_state)
{
	int win_width = 0, win_height = 0;
	window_get_drawable_size(game_state->window, &win_width, &win_height);
	if(nk_begin(context, "Properties", nk_recti(win_width - 300, editor->top_panel_height, 300, 600), window_flags))
	{
		const int row_height = 18;
		if(editor->selected_entity)
		{
			struct Scene* scene = game_state_get()->scene;
			struct Entity* entity = editor->selected_entity;

			struct Entity* parent_ent = entity->transform.parent;
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "Name", NK_TEXT_ALIGN_LEFT); nk_label(context, entity->name, NK_TEXT_ALIGN_RIGHT);
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "ID", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%d", entity->id);
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "Selected", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%s", entity->selected_in_editor ? "True" : "False");
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "Entity Type", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%s", entity_type_name_get(entity));
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "Parent Name", NK_TEXT_ALIGN_LEFT); nk_label(context, parent_ent ? parent_ent->name : "NONE", NK_TEXT_ALIGN_RIGHT);

			/* Transform */
			{
				nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Position", NK_TEXT_ALIGN_CENTERED);
				vec3 abs_pos = { 0.f, 0.f, 0.f };
				transform_get_absolute_position(entity, &abs_pos);
				if(editor_widget_v3(context, &abs_pos, "#X", "#Y", "#Z", -FLT_MAX, FLT_MAX, 5.f, 1.f, row_height)) transform_set_position(entity, &abs_pos);

				nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Rotation", NK_TEXT_ALIGN_CENTERED);
				quat abs_rot = { 0.f, 0.f, 0.f, 1.f };
				transform_get_absolute_rot(entity, &abs_rot);
				vec3 rot_angles = { 0.f, 0.f, 0.f };
				rot_angles.x = quat_get_pitch(&abs_rot);
				rot_angles.y = quat_get_yaw(&abs_rot);
				rot_angles.z = quat_get_roll(&abs_rot);
				vec3 curr_rot = { rot_angles.x, rot_angles.y, rot_angles.z };

				nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "#X", -FLT_MAX, &curr_rot.x, FLT_MAX, 5.f, 1.f);
				nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "#Y", -FLT_MAX, &curr_rot.y, FLT_MAX, 5.f, 1.f);
				nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "#Z", -FLT_MAX, &curr_rot.z, FLT_MAX, 5.f, 1.f);

				vec3 delta = { 0.f, 0.f, 0.f };
				vec3_sub(&delta, &rot_angles, &curr_rot);

				vec3 AXIS_X = { 1.f, 0.f, 0.f };
				vec3 AXIS_Y = { 0.f, 1.f, 0.f };
				vec3 AXIS_Z = { 0.f, 0.f, 1.f };

				const float epsilon = 0.0001f;
				if(fabsf(delta.x) > epsilon) transform_rotate(entity, &AXIS_X, delta.x, TS_WORLD);
				if(fabsf(delta.y) > epsilon) transform_rotate(entity, &AXIS_Y, delta.y, TS_WORLD);
				if(fabsf(delta.z) > epsilon) transform_rotate(entity, &AXIS_Z, delta.z, TS_WORLD);

				nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Scale", NK_TEXT_ALIGN_CENTERED);
				vec3 abs_scale = { 0.f, 0.f, 0.f };
				transform_get_absolute_scale(entity, &abs_scale);
				if(editor_widget_v3(context, &abs_scale, "#X", "#Y", "#Z", 0.1f, FLT_MAX, 1.f, 0.1f, row_height))
				{
					entity->transform.scale = abs_scale;
					transform_update_transmat(entity);
				}
			}

			/* Light */
			if(entity->type == ET_LIGHT)
			{
				if(nk_tree_push(context, NK_TREE_TAB, "Light", NK_MAXIMIZED))
				{
					struct Light* light = (struct Light*)entity;
					if(light->type > LT_POINT)
					{
						nk_layout_row_dynamic(context, row_height, 1);
						nk_label(context, "Invalid light type!", NK_TEXT_ALIGN_CENTERED);
					}
					else
					{
						static const char* light_types[] = { "Spot", "Directional", "Point" };
						float combo_width = nk_widget_width(context), combo_height = row_height * (LT_MAX);

						nk_layout_row_dynamic(context, row_height, 2);
						nk_label(context, "Light Type", NK_TEXT_ALIGN_LEFT);
						nk_combobox(context, light_types, LT_MAX - 1, &light->type, row_height, nk_vec2(combo_width, combo_height));

						nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Light Color", NK_TEXT_ALIGN_CENTERED);
						nk_layout_row_dynamic(context, row_height, 1);
						editor_widget_color_combov3(context, &light->color, 200, 300);

						nk_layout_row_dynamic(context, row_height, 1);
						nk_property_float(context, "Intensity", 0.f, &light->intensity, 100.f, 0.1f, 0.05f);

						if(light->type != LT_DIR)
						{
							nk_layout_row_dynamic(context, row_height, 1);
							light->outer_angle = nk_propertyf(context, "Outer Angle", light->inner_angle, light->outer_angle, 360, 1.f, 0.5f);

							nk_layout_row_dynamic(context, row_height, 1);
							light->inner_angle = nk_propertyf(context, "Inner Angle", 1.f, light->inner_angle, light->outer_angle, 1.f, 0.5f);

							nk_layout_row_dynamic(context, row_height, 1);
							nk_property_int(context, "Radius", 1, &light->radius, INT_MAX, 1, 1);

							nk_layout_row_dynamic(context, row_height, 1);
							nk_property_float(context, "Falloff", 0.f, &light->falloff, 100.f, 0.1f, 0.05f);
						}
					}
					nk_tree_pop(context);
				}
			}

			/* Camera */
			if(entity->type == ET_CAMERA)
			{
				if(nk_tree_push(context, NK_TREE_TAB, "Camera", NK_MAXIMIZED))
				{
					bool update = false;
					struct Camera* camera = (struct Camera*)entity;

					nk_layout_row_dynamic(context, row_height, 2);
					nk_label(context, "Orthographic", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
					bool ortho = nk_check_label(context, "", camera->ortho);
					if(ortho != camera->ortho)
					{
						camera->ortho = ortho;
						update = true;
					}

					if(!camera->ortho)
					{
						nk_layout_row_dynamic(context, row_height, 1);
						float new_fov = nk_propertyf(context, "Fov", 30.f, camera->fov, 90.f, 0.1f, 1.f);
						if(new_fov != camera->fov)
						{
							camera->fov = new_fov;
							update = true;
						}

						nk_layout_row_dynamic(context, row_height, 2);
						nk_label(context, "Aspect Ratio", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%.5f", camera->aspect_ratio);
					}

					nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Clear Color", NK_TEXT_ALIGN_CENTERED);
					nk_layout_row_dynamic(context, row_height, 1);
					editor_widget_color_combov4(context, &camera->clear_color, 200, 300);

					nk_layout_row_dynamic(context, row_height, 1);
					float new_zoom = nk_propertyf(context, "Zoom", 1.f, camera->zoom, FLT_MAX, 0.1f, 1.f);
					if(new_zoom != camera->zoom)
					{
						camera->zoom = new_zoom;
						update = true;
					}

					nk_layout_row_dynamic(context, row_height, 1);
					float new_near_z = nk_propertyf(context, "NearZ", -FLT_MAX, camera->nearz, camera->farz, 0.1f, 1.f);
					if(new_near_z != camera->nearz)
					{
						camera->nearz = new_near_z;
						update = true;
					}

					nk_layout_row_dynamic(context, row_height, 1);
					float new_far_z = nk_propertyf(context, "FarZ", camera->nearz, camera->farz, FLT_MAX, 0.1f, 2.f);
					if(new_far_z != camera->farz)
					{
						camera->farz = new_far_z;
						update = true;
					}

					if(update)
					{
						camera_update_view(entity);
						camera_update_proj(entity);
					}
					nk_tree_pop(context);
				}
			}
		}
		else
		{
			nk_label(context, "No Entity Selected", NK_TEXT_ALIGN_CENTERED);
		}
	}
	else
	{
	     editor->window_property_inspector = false;
	}
	nk_end(context);
}

void editor_window_renderer_settings(struct nk_context* context, struct Editor* editor, struct Game_State* game_state)
{
	int win_width = 0, win_height = 0;
	window_get_drawable_size(game_state->window, &win_width, &win_height);
	int half_width = win_width / 2, half_height = win_height / 2;

	const int row_height = 25;
	if(nk_begin_titled(context, "Renderer_Settings_Window", "Renderer Settings", nk_rect(half_width, half_height, 300, 350), window_flags))
	{
		struct Render_Settings* render_settings = &game_state->renderer->settings;
		if(nk_tree_push(context, NK_TREE_TAB, "Debug", NK_MAXIMIZED))
		{
			static const char* draw_modes[] = { "Triangles", "Lines", "Points" };
			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Debug Draw", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			render_settings->debug_draw_enabled = nk_check_label(context, "", render_settings->debug_draw_enabled);

			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Debug Draw Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			render_settings->debug_draw_mode = nk_combo(context, draw_modes, 3, render_settings->debug_draw_mode, 20, nk_vec2(180, 100));

			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Debug Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			editor_widget_color_combov4(context, &render_settings->debug_draw_color, 200, 400);
			nk_tree_pop(context);
		}

		if(nk_tree_push(context, NK_TREE_TAB, "Fog", NK_MAXIMIZED))
		{
			static const char* fog_modes[] = { "None", "Linear", "Exponential", "Exponential Squared" };
			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			editor_widget_color_combov3(context, &render_settings->fog.color, 200, 400);

			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Fog Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			render_settings->fog.mode = nk_combo(context,
				fog_modes,
				4,
				render_settings->fog.mode,
				20,
				nk_vec2(180, 100));

			nk_layout_row_dynamic(context, row_height, 2);
			nk_label(context, "Density", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			struct nk_rect bounds = nk_widget_bounds(context);
			nk_slider_float(context, 0.f, &render_settings->fog.density, 1.f, 0.005);
			if(nk_input_is_mouse_hovering_rect(&context->input, bounds))
			{
				if(nk_tooltip_begin(context, 100))
				{
					nk_layout_row_dynamic(context, row_height, 1);
					nk_labelf(context, NK_TEXT_ALIGN_CENTERED, "%.3f", render_settings->fog.density);
					nk_tooltip_end(context);
				}
			}

			nk_layout_row_dynamic(context, row_height, 1);
			nk_property_float(context,
				"Start Distance",
				0.f,
				&render_settings->fog.start_dist,
				render_settings->fog.max_dist,
				5.f, 10.f);

			nk_layout_row_dynamic(context, row_height, 1);
			nk_property_float(context,
				"Max Distance",
				render_settings->fog.start_dist,
				&render_settings->fog.max_dist,
				10000.f,
				5.f, 10.f);

			nk_tree_pop(context);
		}
	}
	else
	{
		editor->window_settings_renderer = 0;
	}
	nk_end(context);
}

void editor_window_settings_editor(struct nk_context* context, struct Editor* editor, struct Game_State* game_state)
{
	int win_width = 0, win_height = 0;
	window_get_drawable_size(game_state->window, &win_width, &win_height);
	int half_width = win_width / 2, half_height = win_height / 2;

	const int row_height = 25;
	if(nk_begin_titled(context, "Window_Settings_Editor", "Editor Settings", nk_rect(half_width, half_height, 300, 350), window_flags))
	{
		nk_layout_row_dynamic(context, row_height, 2);
		nk_label(context, "Grid Enabled", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
		nk_checkbox_label(context, "", &editor->grid_enabled);

		nk_layout_row_dynamic(context, row_height, 2);
		nk_label(context, "Grid Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
		editor_widget_color_combov4(context, &editor->grid_color, 200, 400);

		nk_layout_row_dynamic(context, row_height, 1);
		nk_property_int(context, "Grid Lines", 10, &editor->grid_num_lines, 200, 1, 1);

		nk_layout_row_dynamic(context, row_height, 1);
		nk_property_float(context, "Grid Scale", 0.25f, &editor->grid_scale, 10.f, 1, 0.25f);
	}
	else
	{
		editor->window_settings_editor = 0;
	}
	nk_end(context);
}
