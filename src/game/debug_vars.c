#include "debug_vars.h"
#include "event.h"
#include "game.h"
#include "gui.h"
#include "../system/platform.h"
#include "texture.h"
#include <string.h>

static void debug_vars_clear(struct Debug_Vars* debug_vars);

void debug_vars_init(struct Debug_Vars* debug_vars)
{
	debug_vars->visible            = false;
	debug_vars->location           = DVL_TOP_RIGHT;
	debug_vars->window_width       = 350;
	debug_vars->window_height      = 500;
	debug_vars->row_height         = 14;
	debug_vars->row_height_color   = 16;
	debug_vars->row_height_texture = 200;

	for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_NUMERIC; i++)
	{
		memset(&debug_vars->numeric_vars[i].name[0], '\0', MAX_DEBUG_VAR_NAME);
		variant_init_empty(&debug_vars->numeric_vars[i]);
	}

	for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_TEXTURES; i++)
	{
		memset(&debug_vars->texture_vars[i].name[0], '\0', MAX_DEBUG_VAR_NAME);
		variant_init_empty(&debug_vars->texture_vars[i]);
	}
}

void debug_vars_cleanup(struct Debug_Vars* debug_vars)
{
	debug_vars->visible = false;
	debug_vars_clear(debug_vars);
}

void debug_vars_location_set(struct Debug_Vars* debug_vars, int location)
{
	if(location < 0 || location >= DVL_MAX)
	{
		log_error("debug_vars:location_set", "Invalid location. Valid values are from 0-4");
		return;
	}
	debug_vars->location = location;
}

void debug_vars_post_update(struct Debug_Vars* debug_vars)
{
	if(!debug_vars->visible)
		return;

	int                window_x       = 0;
	int                window_y       = 0;
	int                display_width  = 0;
	int                display_height = 0;
	struct Game_State* game_state     = game_state_get();
	struct Gui*        gui            = game_state->gui_editor;
	struct nk_context* context        = &gui->context;

	window_get_drawable_size(game_state->window, &display_width, &display_height);
	int window_flags = NK_WINDOW_BACKGROUND | NK_WINDOW_NO_SCROLLBAR;
	switch(debug_vars->location)
	{
	case DVL_TOP_RIGHT:
		window_x = display_width - debug_vars->window_width;
		window_y = 0;
		break;
	case DVL_BOTTOM_RIGHT:
		window_x = display_width - debug_vars->window_width;
		window_y = display_height - debug_vars->window_height;
		break;
	case DVL_BOTTOM_LEFT:
		window_x = 0;
		window_y = display_height - debug_vars->window_height;
		break;
	case DVL_TOP_LEFT:
		window_x = 0;
		window_y = 0;
		break;
	case DVL_FREE:
		window_x = (display_width / 2) - (debug_vars->window_width / 2);
		window_y = (display_height / 2) - (debug_vars->window_height / 2);
		window_flags &= ~(NK_WINDOW_BACKGROUND & NK_WINDOW_NO_INPUT);
		break;
	}

	nk_byte previous_opacity = context->style.window.background.a;
	context->style.window.fixed_background.data.color.a = 100;
	if(nk_begin(context, "Debug Variables", nk_recti(window_x, window_y, debug_vars->window_width, debug_vars->window_height), window_flags))
	{
		nk_layout_row_dynamic(context, debug_vars->row_height, 2);
		int name_flags = NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_CENTERED;
		int value_flags = NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_CENTERED;
		for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_NUMERIC; i++)
		{
			struct Debug_Variable* variable = &debug_vars->numeric_vars[i];
			if(variable->value.type == VT_NONE)
				continue;

			switch(variable->value.type)
			{
			case VT_INT:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%d", variable->value.val_int);
				break;
			case VT_BOOL:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%s", variable->value.val_bool ? "True" : "False");
				break;
			case VT_FLOAT:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%.3f", variable->value.val_float);
				break;
			case VT_VEC2:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%.3f %.3f", variable->value.val_vec2.x, variable->value.val_vec2.y);
				break;
			case VT_VEC3:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%.3f %.3f %.3f", variable->value.val_vec3.x, variable->value.val_vec3.y, variable->value.val_vec3.z);
				break;
			case VT_VEC4:
				nk_label(context, &variable->name[0], name_flags);
				nk_labelf(context, value_flags, "%.3f %.3f %.3f %.3f", variable->value.val_vec4.x, variable->value.val_vec4.y, variable->value.val_vec4.z, variable->value.val_vec4.w);
				break;
			default:
				nk_label(context, &variable->name[0], name_flags);
				nk_label(context, "Unsupported Type", value_flags);
				break;
			}
		}

		for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_TEXTURES; i++)
		{
			struct Debug_Variable* variable = &debug_vars->texture_vars[i];
			if(variable->value.type == VT_NONE)
				continue;

			switch(variable->value.type)
			{
			case VT_INT: // Texture
				nk_layout_row_dynamic(context, debug_vars->row_height_texture, 2);
				nk_label(context, &variable->name[0], name_flags);
				//nk_image_color(context, nk_image_id(texture_get_texture_handle(variable->value.val_int)), nk_rgb_f(1.f, 1.f, 1.f));
				nk_image_color(context, nk_image_id(variable->value.val_int), nk_rgb_f(1.f, 1.f, 1.f));
				break;
			case VT_VEC3: // Color RGB
				nk_layout_row_dynamic(context, debug_vars->row_height_color, 2);
				nk_label(context, &variable->name[0], name_flags);
				nk_button_color(context, nk_rgb_fv(&variable->value.val_vec3));
				break;
			case VT_VEC4: // Color RGBA
				nk_layout_row_dynamic(context, debug_vars->row_height_color, 2);
				nk_label(context, &variable->name[0], name_flags);
				nk_button_color(context, nk_rgba_fv(&variable->value.val_vec4));
				break;
			default:
				nk_layout_row_dynamic(context, debug_vars->row_height, 2);
				nk_label(context, &variable->name[0], name_flags);
				nk_label(context, "Unsupported Type", value_flags);
				break;
			}

		}
		nk_end(context);
	}
	context->style.window.fixed_background.data.color.a = 255;
	debug_vars_clear(debug_vars);
}

void debug_vars_clear(struct Debug_Vars* debug_vars)
{
	for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_NUMERIC; i++)
	{
		memset(&debug_vars->numeric_vars[i].name[0], '\0', MAX_DEBUG_VAR_NAME);
		variant_free(&debug_vars->numeric_vars[i].value);
	}

	for(int i = 0; i < MAX_DEBUG_VARS_PER_FRAME_TEXTURES; i++)
	{
		memset(&debug_vars->texture_vars[i].name[0], '\0', MAX_DEBUG_VAR_NAME);
		variant_free(&debug_vars->texture_vars[i].value);
	}
}

void debug_vars_show(const char* name, const struct Variant* value, bool is_numeric)
{
	struct Debug_Vars* debug_vars = game_state_get()->debug_vars;
	if(!debug_vars->visible)
		return;
	
	struct Debug_Variable* vars_array = is_numeric ? &debug_vars->numeric_vars[0] : &debug_vars->texture_vars[0];
	int array_length = is_numeric ? MAX_DEBUG_VARS_PER_FRAME_NUMERIC : MAX_DEBUG_VARS_PER_FRAME_TEXTURES;

	for(int i = 0; i < array_length; i++)
	{
		struct Debug_Variable* variable = &vars_array[i];
		if(variable->value.type == VT_NONE)
		{
			strncpy(&variable->name[0], name, MAX_DEBUG_VAR_NAME);
			variant_copy(&variable->value, value);
			return;
		}
	}
	log_warning("%s Debug Vars Full", is_numeric ? "Numeric" : "Texture");
}

void debug_vars_show_int(const char* name, int value)
{
	struct Variant temp_var;
	variant_assign_int(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_bool(const char* name, bool value)
{
	struct Variant temp_var;
	variant_assign_bool(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_float(const char* name, float value)
{
	struct Variant temp_var;
	variant_assign_float(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_texture(const char* name, int texture_index)
{
	struct Variant temp_var;
	variant_assign_int(&temp_var, texture_index);
	debug_vars_show(name, &temp_var, false);
}

void debug_vars_show_vec3(const char* name, const vec3* value)
{
	struct Variant temp_var;
	variant_assign_vec3(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_vec2(const char* name, const vec3* value)
{
	struct Variant temp_var;
	variant_assign_vec2(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_vec4(const char* name, const vec3* value)
{
	struct Variant temp_var;
	variant_assign_vec4(&temp_var, value);
	debug_vars_show(name, &temp_var, true);
}

void debug_vars_show_color_rgb(const char* name, const vec3* value)
{
	struct Variant temp_var;
	variant_assign_vec3(&temp_var, value);
	debug_vars_show(name, &temp_var, false);
}

void debug_vars_show_color_rgba(const char* name, const vec4* value)
{
	struct Variant temp_var;
	variant_assign_vec4(&temp_var, value);
	debug_vars_show(name, &temp_var, false);
}

void debug_vars_cycle_location(struct Debug_Vars* debug_vars)
{
	if(++debug_vars->location >= DVL_MAX)
		debug_vars->location = 0;
}
