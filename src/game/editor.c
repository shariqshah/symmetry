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


static struct Debug_Variable* debug_vars_list = NULL;
static int*                   empty_indices   = NULL;
static int                    window_flags    = NK_WINDOW_BORDER |
		                                        NK_WINDOW_CLOSABLE |
		                                        NK_WINDOW_MOVABLE |
		                                        NK_WINDOW_SCROLL_AUTO_HIDE |
		                                        NK_WINDOW_SCALABLE;

static void editor_on_mousebutton(const struct Event* event);
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

void editor_init(struct Editor* editor)
{
    editor->renderer_settings_window  = 0;
	editor->window_debug_variables    = 0;
	editor->window_property_inspector = 0;
	editor->window_scene_heirarchy    = 0;
	editor->camera_looking_around     = 0;
    editor->selected_entity           = NULL;
    editor->top_panel_height          = 30;
    editor->camera_turn_speed         = 50.f;
    editor->camera_move_speed         = 20.f;
    editor->camera_sprint_multiplier  = 2.f;
	vec4_fill(&editor->selected_entity_colour, 0.f, 1.f, 0.f, 1.f);
    debug_vars_list                   = array_new(struct Debug_Variable);
    empty_indices                     = array_new(int);
	
	event_manager_subscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_PRESSED, &editor_on_mousebutton);
	event_manager_subscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_RELEASED, &editor_on_mousebutton);
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
	//Disabling this for now until better handling of bounding box and scale of the entity is implemented
	//Get the selected entity if any, see if it has a mesh and render it in the selected entity colour
	// if(editor->selected_entity)
	// {
	// 	if(editor->selected_entity->type == ET_STATIC_MESH)
	// 	{
	// 		struct Static_Mesh* mesh = (struct Static_Mesh*)editor->selected_entity;
	// 		struct Geometry* geom = geom_get(mesh->model.geometry_index);
	// 		vec3 abs_pos;
	// 		vec3 abs_scale;
	// 		quat abs_rot;
	// 		transform_get_absolute_position(mesh, &abs_pos);
	// 		transform_get_absolute_scale(mesh, &abs_scale);
	// 		transform_get_absolute_rot(mesh, &abs_rot);
	// 		im_box(geom->bounding_box.max.x, geom->bounding_box.max.y, geom->bounding_box.max.z, abs_pos, abs_rot, editor->selected_entity_colour, GDM_TRIANGLES);

	// 	}
	// }
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
		static float top_panel_ratios[] = { 0.1f, 0.1f, 0.7f, 0.1f };
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

		nk_menubar_begin(context);

		nk_layout_row_begin(context, NK_DYNAMIC, editor->top_panel_height, 5);
		nk_layout_row_push(context, 0.1f);
		if(nk_menu_begin_label(context, "File", NK_TEXT_CENTERED | NK_TEXT_ALIGN_MIDDLE, nk_vec2(150, 100)))
		{
			nk_layout_row_dynamic(context, 25, 1);
			nk_menu_item_label(context, "Open", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			nk_menu_item_label(context, "Save", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
			if(nk_menu_item_label(context, "Back to Game", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE))
			{
				game_state->game_mode = GAME_MODE_GAME;
				game_state->scene->active_camera_index = CAM_GAME;
			}
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.1f);
		if(nk_menu_begin_label(context, "Settings", NK_TEXT_CENTERED | NK_TEXT_ALIGN_MIDDLE, nk_vec2(150, 100)))
		{
			nk_layout_row_dynamic(context, 25, 1);
			if(nk_menu_item_label(context, "Render Settings", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE)) editor->renderer_settings_window = !editor->renderer_settings_window;
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.1f);
		if(nk_menu_begin_label(context, "Windows", NK_TEXT_CENTERED | NK_TEXT_ALIGN_MIDDLE, nk_vec2(180, 100)))
		{
			nk_layout_row_dynamic(context, 25, 1);
			nk_checkbox_label(context, "Scene Heirarchy", &editor->window_scene_heirarchy);
			nk_checkbox_label(context, "Property Inspector", &editor->window_property_inspector);
			nk_checkbox_label(context, "Debug Variables", &editor->window_debug_variables);
			nk_menu_end(context);
		}

		nk_layout_row_push(context, 0.6f);
		nk_spacing(context, 1);

		nk_layout_row_push(context, 0.1f);
		nk_labelf(context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE, "FPS : %.d", fps);
		
		nk_menubar_end(context);
	}
	nk_end(context);

	if(editor->window_scene_heirarchy) editor_window_scene_heirarchy(context, editor, game_state);
	if(editor->window_debug_variables) editor_window_debug_variables(context, editor);
	if(editor->window_property_inspector) editor_window_property_inspector(context, editor, game_state);

    /* Render Settings Window */
	if(editor->renderer_settings_window)
	{
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
			editor->renderer_settings_window = 0;
		}
		nk_end(context);
	}
}

void editor_on_mousebutton(const struct Event* event)
{
	assert(event->type == EVT_MOUSEBUTTON_PRESSED || event->type == EVT_MOUSEBUTTON_RELEASED);

	struct Game_State* game_state = game_state_get();
	struct Editor*     editor     = game_state->editor;
	struct Gui*        gui        = game_state->gui;
	if(game_state->game_mode != GAME_MODE_EDITOR)
		return;

	if(event->mousebutton.button == MSB_LEFT &&
	   event->type == EVT_MOUSEBUTTON_RELEASED && 
	   !editor->camera_looking_around && 
	   nk_item_is_any_active(&gui->context) == 0)
	{
		log_message("Editor Picking");
		struct Camera* editor_camera = &game_state_get()->scene->cameras[CAM_EDITOR];
		int mouse_x = 0, mouse_y = 0;
		platform_mouse_position_get(&mouse_x, &mouse_y);
		struct Ray ray = camera_screen_coord_to_ray(editor_camera, mouse_x, mouse_y);
		//log_message("Ray: %.3f, %.3f, %.3f", ray.direction.x, ray.direction.y, ray.direction.z); 

		struct Scene* scene = game_state_get()->scene;
		struct Raycast_Result ray_result;
		scene_ray_intersect(scene, &ray, &ray_result);

		if(ray_result.num_entities_intersected > 0)
		{
			//For now, just select the first entity that is intersected 
			struct Entity* intersected_entity = ray_result.entities_intersected[0];

			if(editor->selected_entity && editor->selected_entity != intersected_entity)
			{
				editor->selected_entity->editor_selected = false;
				editor->selected_entity = NULL;
			}

			intersected_entity->editor_selected = true;
			editor->selected_entity = intersected_entity;
		}
		else
		{
			//Deselect the currently selected entity if nothing was found
			if(editor->selected_entity)
			{
				editor->selected_entity->editor_selected = false;
				editor->selected_entity = NULL;
			}
		}
	}
}

void editor_camera_update(struct Editor* editor, float dt)
{
    struct Camera* editor_camera = &game_state_get()->scene->cameras[CAM_EDITOR];
    static float total_up_down_rot = 0.f;
    float move_speed          = editor->camera_move_speed, turn_speed = editor->camera_turn_speed;
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
		editor->camera_looking_around = true;
		const float scale = 0.1f;
		int cursor_lr, cursor_ud;
		input_mouse_delta_get(&cursor_lr, &cursor_ud);
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
		input_mouse_mode_set(MM_NORMAL);
		turn_up_down *= dt;
		turn_left_right *= dt;
		if(editor->camera_looking_around)
		{
			int width = 0, height = 0;
			window_get_drawable_size(game_state_get()->window, &width, &height);
			platform_mouse_position_set(game_state_get()->window, width / 2, height / 2);
			editor->camera_looking_around = false;
		}
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
    if(input_map_state_get("Sprint",        KS_PRESSED)) move_speed *= editor->camera_sprint_multiplier;
    if(input_map_state_get("Move_Forward",  KS_PRESSED)) offset.z   -= move_speed;
    if(input_map_state_get("Move_Backward", KS_PRESSED)) offset.z   += move_speed;
    if(input_map_state_get("Move_Left",     KS_PRESSED)) offset.x   -= move_speed;
    if(input_map_state_get("Move_Right",    KS_PRESSED)) offset.x   += move_speed;
    if(input_map_state_get("Move_Up",       KS_PRESSED)) offset.y   += move_speed;
    if(input_map_state_get("Move_Down",     KS_PRESSED)) offset.y   -= move_speed;

    vec3_scale(&offset, &offset, dt);
    if(offset.x != 0 || offset.y != 0 || offset.z != 0)
    {
		transform_translate(editor_camera, &offset, TS_LOCAL);
		//log_message("Position : %s", tostr_vec3(&transform->position));
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
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_PRESSED, &editor_on_mousebutton);
	event_manager_unsubscribe(game_state_get()->event_manager, EVT_MOUSEBUTTON_RELEASED, &editor_on_mousebutton);
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
	if(nk_selectable_label(context, entity->name, NK_TEXT_ALIGN_LEFT, &entity->editor_selected))
	{
		if(editor->selected_entity && editor->selected_entity != entity)
		{
			editor->selected_entity->editor_selected = false;
			editor->selected_entity = NULL;
		}
		else if(editor->selected_entity && editor->selected_entity == entity && !entity->editor_selected)
		{
			editor->selected_entity = NULL;
		}

		if(entity->editor_selected)
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
			nk_layout_row_dynamic(context, row_height, 2); nk_label(context, "Selected", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%s", entity->editor_selected ? "True" : "False");
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
				rot_angles.x = TO_DEGREES(quat_get_pitch(&abs_rot));
				rot_angles.y = TO_DEGREES(quat_get_yaw(&abs_rot));
				rot_angles.z = TO_DEGREES(quat_get_roll(&abs_rot));
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
							light->outer_angle = TO_RADIANS(nk_propertyf(context, "Outer Angle", TO_DEGREES(light->inner_angle), TO_DEGREES(light->outer_angle), 360, 1.f, 0.5f));

							nk_layout_row_dynamic(context, row_height, 1);
							light->inner_angle = TO_RADIANS(nk_propertyf(context, "Inner Angle", 1.f, TO_DEGREES(light->inner_angle), TO_DEGREES(light->outer_angle), 1.f, 0.5f));

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
