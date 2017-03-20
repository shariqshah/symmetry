#include "editor.h"
#include "renderer.h"
#include "gl_load.h"
#include "log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "array.h"
#include "shader.h"
#include "num_types.h"
#include "light.h"
#include "entity.h"
#include "transform.h"
#include "game.h"
#include "gui.h"

#include <stdio.h>

struct Editor_State
{
	int enabled;
	int renderer_settings_window;
	int top_panel_height;
};

static struct Editor_State editor_state;

static void editor_color_combo(struct nk_context* context, vec4* color, int width, int height);

void editor_init(void)
{
	editor_state.enabled                  = 1;
	editor_state.renderer_settings_window = 0;
	editor_state.top_panel_height         = 30;
}

void editor_update(float dt)
{
	if(!editor_state.enabled) return;
	
	struct Game_State* 		game_state 		= game_state_get();
	struct Gui_State*  		gui_state  		= gui_state_get();
	struct nk_context* 		context    		= &gui_state->context;
	struct Render_Settings* render_settings = renderer_settings_get();
	int win_width = 0, win_height = 0;
	window_get_drawable_size(game_state->window, &win_width, &win_height);
	int half_width = win_width / 2, half_height = win_height / 2;


	/* Top Panel */
	if(nk_begin(context, "Top_Panel", nk_recti(0, 0, win_width, win_height - (win_height - editor_state.top_panel_height)),
				NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR))
	{
		float ratios[] = {0.1f, 0.8f, 0.1f};
		static int   frames  = 0;
		static int   fps     = 0;
		static float seconds = 0.f;
		seconds += dt;
		frames++;
		if(seconds >= 1.f)
		{
			fps = frames;
			seconds = 0.f;
			frames = 0;
		}
		nk_layout_row(context, NK_DYNAMIC, 22, 3, ratios);
		if(nk_button_label(context, "Render Settings"))
			editor_state.renderer_settings_window = !editor_state.renderer_settings_window;
		nk_spacing(context, 1);
		nk_labelf(context, NK_TEXT_ALIGN_RIGHT | NK_TEXT_ALIGN_MIDDLE, "FPS : %.d", fps);
	}
	nk_end(context);

	/* Debug Window */
	if(editor_state.renderer_settings_window)
	{
		if(nk_begin_titled(context, "Renderer_Settings_Window", "Renderer Settings", nk_rect(half_width, half_height, 300, 350),
						   NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE |
						   NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_SCALABLE | NK_WINDOW_MINIMIZABLE))
		{
			if(nk_tree_push(context, NK_TREE_TAB, "Debug", NK_MAXIMIZED))
			{
				static const char* draw_modes[] = {"Triangles", "Lines", "Points"};
				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Debug Draw", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				nk_checkbox_label(context, "", &render_settings->debug_draw_enabled);
				
				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Debug Draw Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				render_settings->debug_draw_mode = nk_combo(context, draw_modes, 3, render_settings->debug_draw_mode, 20, nk_vec2(180, 100));
				
				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Debug Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				editor_color_combo(context, &render_settings->debug_draw_color, 200, 400);
				nk_tree_pop(context);
			}

			if(nk_tree_push(context, NK_TREE_TAB, "Fog", NK_MAXIMIZED))
			{
				static const char* fog_modes[] = {"None", "Linear", "Exponential", "Exponential Squared"};
				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Color", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				static vec4 fog_color;
				vec4_fill_vec3(&fog_color, &render_settings->fog.color, 1.f);
				editor_color_combo(context, &fog_color, 200, 400);
				vec3_fill(&render_settings->fog.color, fog_color.x, fog_color.y, fog_color.z);
				
				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Fog Mode", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				render_settings->fog.mode = nk_combo(context,
													 fog_modes,
													 4,
													 render_settings->fog.mode,
													 20,
													 nk_vec2(180, 100));

				nk_layout_row_dynamic(context, 25, 2);
				nk_label(context, "Density", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
				struct nk_rect bounds = nk_widget_bounds(context);
				nk_slider_float(context, 0.f, &render_settings->fog.density, 1.f, 0.005);
				if(nk_input_is_mouse_hovering_rect(&context->input, bounds))
				{
					static char float_str[6];
					snprintf(float_str, 6, "%.4f", render_settings->fog.density);
					float_str[5] = '\0';
					nk_tooltip(context, float_str);
				}

				nk_layout_row_dynamic(context, 25, 1);
				nk_property_float(context,
								  "Start Distance",
								  0.f,
								  &render_settings->fog.start_dist,
								  render_settings->fog.max_dist,
								  5.f, 10.f);

				nk_layout_row_dynamic(context, 25, 1);
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
			editor_state.renderer_settings_window = 0;
		}
		nk_end(context);
	}
}

void editor_toggle(void)
{
	editor_state.enabled = !editor_state.enabled;
}

void editor_color_combo(struct nk_context* context, vec4* color, int width, int height)
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
