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

struct Gui_State
{
    struct nk_buffer            cmds;
    struct nk_draw_null_texture null;
	struct nk_context           context;
	struct nk_font_atlas        atlas;
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
    //GLuint					        font_tex;           
};

int  			  gui_init(void);
void 			  gui_cleanup(void);
void 			  gui_render(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer);
void 			  gui_handle_mousewheel_event(int x, int y);
void 			  gui_handle_mousemotion_event(int x, int y, int xrel, int yrel);
void 			  gui_handle_mousebutton_event(int button, int state, int x, int y);
void 			  gui_handle_keyboard_event(int key, int state, int mod_ctrl, int mod_shift);
void              gui_input_begin(void);
void              gui_input_end(void);
struct Gui_State* gui_state_get(void);
	
#endif
