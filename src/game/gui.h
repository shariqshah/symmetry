#ifndef GUI_H
#define GUI_H

#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_DEFAULT_FONT

#include <nuklear.h>
#include "gl_load.h"
#include "../common/num_types.h"

struct Gui
{
    struct nk_buffer            commands;
    struct nk_draw_null_texture null;
	struct nk_context           context;
	struct nk_font_atlas        atlas;
	struct nk_font*             current_font;
    GLuint                      vbo, vao, ebo;
	int   					    shader;
    GLuint					    vertex_shader;
    GLuint					    fragment_shader;
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

bool gui_init(struct Gui* gui);
void gui_cleanup(struct Gui* gui);
void gui_render(struct Gui* gui, enum nk_anti_aliasing AA);
void gui_input_begin(struct Gui* gui);
void gui_input_end(struct Gui* gui);
void gui_font_set(struct Gui* gui, const char* font_name, float font_height);
void gui_theme_set(struct Gui* gui, enum Gui_Theme theme);
	
#endif
