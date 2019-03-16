#ifndef GUI_H
#define GUI_H

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>
#include "gl_load.h"
#include "../common/num_types.h"

struct Gui_State
{
    struct nk_buffer            cmds;
    struct nk_draw_null_texture null;
	struct nk_context           context;
	struct nk_font_atlas        atlas;
	struct nk_font*             current_font;
    GLuint                      vbo, vao, ebo;
	int   					    shader;
    GLuint					    vert_shdr;
    GLuint					    frag_shdr;
    GLint 					    attrib_pos;
    GLint 					    attrib_uv;
    GLint 					    attrib_col;
    GLint 					    uniform_tex;
    GLint 					    uniform_proj;
    int					        font_tex;
};

enum Gui_Theme
{
	GT_DEFAULT = 0,
	GT_WHITE,
	GT_RED,
	GT_BLUE,
	GT_DARK
};

bool  			  gui_init(void);
void 			  gui_cleanup(void);
void 			  gui_render(enum nk_anti_aliasing AA);
void 			  gui_handle_mousewheel_event(int x, int y);
void 			  gui_handle_mousemotion_event(int x, int y, int xrel, int yrel);
void 			  gui_handle_mousebutton_event(int button, int state, int x, int y);
void 			  gui_handle_keyboard_event(int key, int state, int mod_ctrl, int mod_shift);
void              gui_input_begin(void);
void              gui_input_end(void);
void              gui_font_set(const char* font_name, float font_height);
void              gui_theme_set(enum Gui_Theme theme);
struct Gui_State* gui_state_get(void);
	
#endif
