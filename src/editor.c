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

struct Editor_State
{
	int enabled;
	int debug_window_enabled;
	int top_panel_height;
};

static struct Editor_State editor_state;

void editor_init(void)
{
	editor_state.enabled              = 1;
	editor_state.debug_window_enabled = 0;
	editor_state.top_panel_height     = 50;
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
	static int debug_window = 1;


	/* Top Panel */
	if(nk_begin(context, "Top_Panel", nk_recti(0, 0, win_width, win_height - (win_height - editor_state.top_panel_height)),
				NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND))
	{
		nk_layout_row_static(context, 40, 100, 2);
		if(nk_button_label(context, "Debug")) editor_state.debug_window_enabled = !editor_state.debug_window_enabled; 
	}
	nk_end(context);

	/* Debug Window */
	if(debug_window)
	{
		if(nk_begin_titled(context, "Debug_Window", "Debug", nk_recti(0, 0, 200, 200),
						   NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE |
						   NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_DYNAMIC))
		{
			debug_window = 1;
			nk_layout_row_static(context, 40, 50, 1);
			nk_checkbox_label(context, "Debug Draw", &render_settings->debug_draw_enabled);
		}
		else
		{
			debug_window = 0;
		}
		nk_end(context);
	}
}

void editor_toggle(void)
{
	editor_state.enabled = !editor_state.enabled;
}

