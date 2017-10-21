#include "editor.h"
#include "renderer.h"
#include "gl_load.h"
#include "../common/log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "../common/array.h"
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
#include "../common/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <math.h>

struct Editor_State
{
	bool enabled;
	bool renderer_settings_window;
	int  selected_entity_id;
	int  top_panel_height;
};

struct Debug_Variable
{
	struct Variant data;
	char*          name;
};

static struct Editor_State    editor_state;
static struct Debug_Variable* debug_vars_list = NULL;
static int*                   empty_indices   = NULL;

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

void editor_init(void)
{
	editor_state.enabled                  = true;
	editor_state.renderer_settings_window = false;
	editor_state.selected_entity_id       = -1;
	editor_state.top_panel_height         = 20;
	debug_vars_list                       = array_new(struct Debug_Variable);
	empty_indices                         = array_new(int);
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
	debug_var->name      = str_new(name);
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

void editor_update(float dt)
{
	if(!editor_state.enabled) return;
	
	struct Game_State* game_state = game_state_get();
	struct Gui_State*  gui_state  = gui_state_get();
	struct nk_context* context    = &gui_state->context;
	int win_width = 0, win_height = 0;
    platform->window.get_drawable_size(game_state->window, &win_width, &win_height);
	int half_width = win_width / 2, half_height = win_height / 2;
	static int window_flags = NK_WINDOW_BORDER |
		NK_WINDOW_CLOSABLE |
		NK_WINDOW_MOVABLE |
		NK_WINDOW_SCROLL_AUTO_HIDE |
		NK_WINDOW_SCALABLE;

	/* Main enacapsulating window */
	struct nk_style_item default_background = context->style.window.fixed_background;
	struct nk_vec2 default_padding = context->style.window.padding;
	context->style.window.padding = nk_vec2(0.f, 0.f);
	context->style.window.fixed_background = nk_style_item_hide();
	if(nk_begin(context, "Editor", nk_recti(0, 0, win_width, win_height),  NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND))
	{
		context->style.window.fixed_background = default_background;
		/* Top Panel */
		nk_layout_row_dynamic(context, editor_state.top_panel_height + 10, 1);
		nk_group_begin(context, "Menubar", NK_WINDOW_NO_SCROLLBAR);
		{
			static float top_panel_ratios[] = {0.1f, 0.1f, 0.7f, 0.1f};
			static int   frames  = 0;
			static int   fps     = 0;
			static float seconds = 0.f;
			seconds += dt;
			frames++;
			if(seconds >= 1.f)
			{
				fps     = frames;
				seconds = 0.f;
				frames  = 0;
			}

			nk_layout_row(context, NK_DYNAMIC, editor_state.top_panel_height, sizeof(top_panel_ratios) / sizeof(float), top_panel_ratios);
			if(nk_button_label(context, "Render Settings"))
				editor_state.renderer_settings_window = !editor_state.renderer_settings_window;
			if(nk_button_label(context, "Save config"))
                platform->config.save("config.cfg", DIRT_USER);
			nk_spacing(context, 1);
			nk_labelf(context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE, "FPS : %.d", fps);
			nk_group_end(context);
		}

		static float main_editor_ratios[] = {0.2f, 0.6f, 0.2f};
        nk_layout_row(context, NK_DYNAMIC, win_height - editor_state.top_panel_height, sizeof(main_editor_ratios) / sizeof(float), main_editor_ratios);
		/* Left */
		if(nk_group_begin(context, "Editor Left", NK_WINDOW_SCROLL_AUTO_HIDE))
		{
			/* Entities List */
			if(nk_tree_push(context, NK_TREE_TAB, "Entities", NK_MAXIMIZED))
			{
				nk_layout_row_dynamic(context, 250, 1);
				if(nk_group_begin(context, "Entity Name", NK_WINDOW_SCROLL_AUTO_HIDE))
				{
					struct Entity* entity_list = entity_get_all();
					for(int i = 0; i < array_len(entity_list); i++)
					{
						struct Entity* entity = &entity_list[i];
						nk_layout_row_dynamic(context, 20, 1);
						if(nk_selectable_label(context, entity->name, NK_TEXT_ALIGN_LEFT, &entity->editor_selected))
						{
							if(editor_state.selected_entity_id != -1)
							{
								struct Entity* currently_selected = entity_get(editor_state.selected_entity_id);
								currently_selected->editor_selected = false;
							}
							
							if(entity->editor_selected)
							{
								editor_state.selected_entity_id = entity->id;
							}
							else
							{
								editor_state.selected_entity_id = -1;
							}
						}
					}
					nk_group_end(context);
				}
				nk_tree_pop(context);
			}

			/* Debug Variables */
			if(nk_tree_push(context, NK_TREE_TAB, "Debug Variables", NK_MAXIMIZED))
			{
				static char variant_str[MAX_VARIANT_STR_LEN] = {'\0'};
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
				nk_tree_pop(context);
			}
			
			nk_group_end(context);
		}

		/* Empty Space in the center */
		nk_spacing(context, 1);

		/* Right */
		if(nk_group_begin(context, "Editor Right", NK_WINDOW_NO_SCROLLBAR))
		{
			/* Entity Inspector */
			if(nk_tree_push(context, NK_TREE_TAB, "Inspector", NK_MAXIMIZED))
			{
				const int row_height = 18;
				if(editor_state.selected_entity_id != -1)
				{
					struct Entity* entity = entity_get(editor_state.selected_entity_id);
					nk_layout_row_dynamic(context, row_height, 2);
					nk_label(context, "Name", NK_TEXT_ALIGN_LEFT); nk_label(context, entity->name, NK_TEXT_ALIGN_RIGHT);
					nk_layout_row_dynamic(context, row_height, 2);
					nk_label(context, "ID", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%d", entity->id);
					nk_layout_row_dynamic(context, row_height, 2);
					nk_label(context, "Entity Type", NK_TEXT_ALIGN_LEFT); nk_labelf(context, NK_TEXT_ALIGN_RIGHT, "%s", entity_type_name_get(entity));
					nk_layout_row_dynamic(context, row_height, 2);
					struct Entity* parent_ent = entity_get(entity->transform.parent);
					nk_label(context, "Parent Name", NK_TEXT_ALIGN_LEFT); nk_label(context, parent_ent ? parent_ent->name : "NONE", NK_TEXT_ALIGN_RIGHT);

					/* Transform */
					{
						nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Position", NK_TEXT_ALIGN_CENTERED);
						vec3 abs_pos = {0.f, 0.f, 0.f};
						transform_get_absolute_pos(entity, &abs_pos);
						if(editor_widget_v3(context, &abs_pos, "Px", "Py", "Pz", -FLT_MAX, FLT_MAX, 5.f, 1.f, row_height)) transform_set_position(entity, &abs_pos);

						nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Rotation", NK_TEXT_ALIGN_CENTERED);
						quat abs_rot = {0.f, 0.f, 0.f, 1.f};
						transform_get_absolute_rot(entity, &abs_rot);
						vec3 rot_angles = {0.f, 0.f, 0.f};
						rot_angles.x = TO_DEGREES(quat_get_pitch(&abs_rot));
						rot_angles.y = TO_DEGREES(quat_get_yaw(&abs_rot));
						rot_angles.z = TO_DEGREES(quat_get_roll(&abs_rot));
						vec3 curr_rot = {rot_angles.x, rot_angles.y, rot_angles.z};

						nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "Rx", -FLT_MAX, &curr_rot.x, FLT_MAX, 5.f, 1.f);
						nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "Ry", -FLT_MAX, &curr_rot.y, FLT_MAX, 5.f, 1.f);
						nk_layout_row_dynamic(context, row_height, 1); nk_property_float(context, "Rz", -FLT_MAX, &curr_rot.z, FLT_MAX, 5.f, 1.f);

						vec3 delta = {0.f, 0.f, 0.f};
						vec3_sub(&delta, &rot_angles, &curr_rot);

						vec3 AXIS_X = {1.f, 0.f, 0.f};
						vec3 AXIS_Y = {0.f, 1.f, 0.f};
						vec3 AXIS_Z = {0.f, 0.f, 1.f};

						const float epsilon = 0.0001f;
						if(fabsf(delta.x) > epsilon) transform_rotate(entity, &AXIS_X, delta.x, TS_WORLD);
						if(fabsf(delta.y) > epsilon) transform_rotate(entity, &AXIS_Y, delta.y, TS_WORLD);
						if(fabsf(delta.z) > epsilon) transform_rotate(entity, &AXIS_Z, delta.z, TS_WORLD);

						nk_layout_row_dynamic(context, row_height, 1); nk_label(context, "Scale", NK_TEXT_ALIGN_CENTERED);
						vec3 abs_scale = {0.f, 0.f, 0.f};
						transform_get_absolute_scale(entity, &abs_scale);
						if(editor_widget_v3(context, &abs_scale, "SX", "SY", "SZ", 0.1f, FLT_MAX, 1.f, 0.1f, row_height))
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
							struct Light* light = &entity->light;
							if(light->type > LT_POINT)
							{
								nk_layout_row_dynamic(context, row_height, 1);
								nk_label(context, "Invalid light type!", NK_TEXT_ALIGN_CENTERED);
							}
							else
							{
								static const char* light_types[] = {"Spot", "Directional", "Point"};
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
							struct Camera* camera = &entity->camera;
							
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
								if(!camera->ortho) camera_update_view(entity);
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
				nk_tree_pop(context);
			}
			
			nk_group_end(context);
		}
	}
	nk_end(context);
	context->style.window.padding = default_padding;

	/* Render Settings Window */
	if(editor_state.renderer_settings_window)
	{
		const int row_height = 25;
		if(nk_begin_titled(context, "Renderer_Settings_Window", "Renderer Settings", nk_rect(half_width, half_height, 300, 350), window_flags))
		{
			static struct Render_Settings render_settings;
			renderer_settings_get(&render_settings);
			if(nk_tree_push(context, NK_TREE_TAB, "Debug", NK_MAXIMIZED))
			{
				static const char* draw_modes[] = {"Triangles", "Lines", "Points"};
				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Debug Draw", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				nk_checkbox_label(context, "", &render_settings.debug_draw_enabled);
				
				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Debug Draw Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				render_settings.debug_draw_mode = nk_combo(context, draw_modes, 3, render_settings.debug_draw_mode, 20, nk_vec2(180, 100));
				
				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Debug Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				editor_widget_color_combov4(context, &render_settings.debug_draw_color, 200, 400);
				nk_tree_pop(context);
			}

			if(nk_tree_push(context, NK_TREE_TAB, "Fog", NK_MAXIMIZED))
			{
				static const char* fog_modes[] = {"None", "Linear", "Exponential", "Exponential Squared"};
				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				editor_widget_color_combov3(context, &render_settings.fog.color, 200, 400);
				
				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Fog Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				render_settings.fog.mode = nk_combo(context,
													fog_modes,
													4,
													render_settings.fog.mode,
													20,
													nk_vec2(180, 100));

				nk_layout_row_dynamic(context, row_height, 2);
				nk_label(context, "Density", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				struct nk_rect bounds = nk_widget_bounds(context);
				nk_slider_float(context, 0.f, &render_settings.fog.density, 1.f, 0.005);
				if(nk_input_is_mouse_hovering_rect(&context->input, bounds))
				{
					if(nk_tooltip_begin(context, 100))
					{
						nk_layout_row_dynamic(context, row_height, 1);
						nk_labelf(context, NK_TEXT_ALIGN_CENTERED, "%.3f", render_settings.fog.density);
						nk_tooltip_end(context);
					}
				}

				nk_layout_row_dynamic(context, row_height, 1);
				nk_property_float(context,
								  "Start Distance",
								  0.f,
								  &render_settings.fog.start_dist,
								  render_settings.fog.max_dist,
								  5.f, 10.f);

				nk_layout_row_dynamic(context, row_height, 1);
				nk_property_float(context,
								  "Max Distance",
								  render_settings.fog.start_dist,
								  &render_settings.fog.max_dist,
								  10000.f,
								  5.f, 10.f);

				nk_tree_pop(context);
			}
			renderer_settings_set(&render_settings);
		}
		else
		{
			editor_state.renderer_settings_window = 0;
		}
		nk_end(context);
	}
}

void editor_toggle(void)
{
	editor_state.enabled = !editor_state.enabled;
}

void editor_widget_color_combov3(struct nk_context* context, vec3* color, int width, int height)
{
	struct nk_color temp_color = nk_rgba_f(color->x, color->y, color->z, 1.f);
	if(nk_combo_begin_color(context, temp_color, nk_vec2(width, height)))
	{
		enum color_mode {COL_RGB, COL_HSV};
		static int col_mode = COL_RGB;
		nk_layout_row_dynamic(context, 25, 2);
		col_mode = nk_option_label(context, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
		col_mode = nk_option_label(context, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;
		nk_layout_row_dynamic(context, 120, 1);
		temp_color = nk_color_picker(context, temp_color, NK_RGB);
		nk_layout_row_dynamic(context, 25, 1);
		if(col_mode == COL_RGB)
		{
			temp_color.r = (nk_byte)nk_propertyi(context, "#R:", 0, temp_color.r, 255, 1,1);
			temp_color.g = (nk_byte)nk_propertyi(context, "#G:", 0, temp_color.g, 255, 1,1);
			temp_color.b = (nk_byte)nk_propertyi(context, "#B:", 0, temp_color.b, 255, 1,1);
		}
		else
		{
			nk_byte tmp[4];
			nk_color_hsva_bv(tmp, temp_color);
			tmp[0] = (nk_byte)nk_propertyi(context, "#H:", 0, tmp[0], 255, 1,1);
			tmp[1] = (nk_byte)nk_propertyi(context, "#S:", 0, tmp[1], 255, 1,1);
			tmp[2] = (nk_byte)nk_propertyi(context, "#V:", 0, tmp[2], 255, 1,1);
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
		enum color_mode {COL_RGB, COL_HSV};
		static int col_mode = COL_RGB;
		nk_layout_row_dynamic(context, 25, 2);
		col_mode = nk_option_label(context, "RGB", col_mode == COL_RGB) ? COL_RGB : col_mode;
		col_mode = nk_option_label(context, "HSV", col_mode == COL_HSV) ? COL_HSV : col_mode;
		nk_layout_row_dynamic(context, 120, 1);
		temp_color = nk_color_picker(context, temp_color, NK_RGBA);
		nk_layout_row_dynamic(context, 25, 1);
		if(col_mode == COL_RGB)
		{
			temp_color.r = (nk_byte)nk_propertyi(context, "#R:", 0, temp_color.r, 255, 1,1);
			temp_color.g = (nk_byte)nk_propertyi(context, "#G:", 0, temp_color.g, 255, 1,1);
			temp_color.b = (nk_byte)nk_propertyi(context, "#B:", 0, temp_color.b, 255, 1,1);
			temp_color.a = (nk_byte)nk_propertyi(context, "#A:", 0, temp_color.a, 255, 1,1);
		}
		else
		{
			nk_byte tmp[4];
			nk_color_hsva_bv(tmp, temp_color);
			tmp[0] = (nk_byte)nk_propertyi(context, "#H:", 0, tmp[0], 255, 1,1);
			tmp[1] = (nk_byte)nk_propertyi(context, "#S:", 0, tmp[1], 255, 1,1);
			tmp[2] = (nk_byte)nk_propertyi(context, "#V:", 0, tmp[2], 255, 1,1);
			tmp[3] = (nk_byte)nk_propertyi(context, "#A:", 0, tmp[3], 255, 1,1);
			temp_color = nk_hsva_bv(tmp);
		}
		nk_color_f(&color->x, &color->y, &color->z, &color->w, temp_color);
		nk_combo_end(context);
	}
}

void editor_cleanup(void)
{
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
