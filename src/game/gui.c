#define NK_IMPLEMENTATION
#include "gui.h"
#include "../common/linmath.h"
#include "../common/log.h"
#include "shader.h"
#include "texture.h"
#include "game.h"
#include "input.h"
#include "renderer.h"
#include "../common/string_utils.h"
#include "../system/platform.h"
#include "../system/file_io.h"
#include "event.h"

#include <string.h>
#include <stdlib.h>

struct Gui_Vertex
{
    vec2 pos, uv;
    nk_byte col[4];
};

#define MAX_GUI_VERTEX_MEMORY  512 * 1024
#define MAX_GUI_ELEMENT_MEMORY 128 * 1024

static void gui_on_clipbard_copy(nk_handle usr, const char *text, int len);
static void gui_on_clipbard_paste(nk_handle usr, struct nk_text_edit *edit);
static void gui_on_textinput(const struct Event* event);
static void gui_on_mousewheel(const struct Event* event);
static void gui_on_mousemotion(const struct Event* event);
static void gui_on_mousebutton(const struct Event* event);
static void gui_on_key(const struct Event* event);

static void gui_upload_atlas(const void *image, int width, int height);
static void gui_font_set_default(void);

static struct Gui_State* gui_state = NULL;

bool gui_init(void)
{
	bool success = false;
	gui_state = malloc(sizeof(*gui_state));
	if(!gui_state)
	{
		log_error("gui:init", "Malloc failed, out of memory");
		return success;
	}
	
	nk_init_default(&gui_state->context, 0);
	nk_buffer_init_default(&gui_state->cmds);
    gui_state->context.clip.copy     = gui_on_clipbard_copy;
    gui_state->context.clip.paste    = gui_on_clipbard_paste;
    gui_state->context.clip.userdata = nk_handle_ptr(0);
	gui_state->current_font          = NULL;
	gui_state->shader                = shader_create("gui.vert", "gui.frag");
	if(gui_state->shader < 0)
	{
		log_error("gui:init", "Failed to create shader for gui");
		free(gui_state);
		return success;
	}
    gui_state->uniform_tex  = shader_get_uniform_location(gui_state->shader,   "sampler");
    gui_state->uniform_proj = shader_get_uniform_location(gui_state->shader,   "proj_mat");
    gui_state->attrib_pos   = shader_get_attribute_location(gui_state->shader, "vPosition");
    gui_state->attrib_uv    = shader_get_attribute_location(gui_state->shader, "vUV");
    gui_state->attrib_col   = shader_get_attribute_location(gui_state->shader, "vColor");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct Gui_Vertex);
        size_t  vp = offsetof(struct Gui_Vertex, pos);
        size_t  vt = offsetof(struct Gui_Vertex, uv);
        size_t  vc = offsetof(struct Gui_Vertex, col);

        glGenBuffers(1, &gui_state->vbo);
        glGenBuffers(1, &gui_state->ebo);
        glGenVertexArrays(1, &gui_state->vao);

        glBindVertexArray(gui_state->vao);
        glBindBuffer(GL_ARRAY_BUFFER, gui_state->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui_state->ebo);

        glEnableVertexAttribArray((GLuint)gui_state->attrib_pos);
        glEnableVertexAttribArray((GLuint)gui_state->attrib_uv);
        glEnableVertexAttribArray((GLuint)gui_state->attrib_col);

        glVertexAttribPointer((GLuint)gui_state->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)gui_state->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)gui_state->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	//gui_font_set("Ubuntu-R.ttf", 14);
	//gui_font_set("FiraSans-Regular.ttf", 14);
	gui_font_set("roboto_condensed.ttf", 18);
//	gui_theme_set(GT_RED);
    gui_theme_set(GT_DEFAULT);

	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_subscribe(event_manager, EVT_KEY_PRESSED, &gui_on_key);
	event_manager_subscribe(event_manager, EVT_KEY_RELEASED, &gui_on_key);
	event_manager_subscribe(event_manager, EVT_MOUSEBUTTON_PRESSED, &gui_on_mousebutton);
	event_manager_subscribe(event_manager, EVT_MOUSEBUTTON_RELEASED, &gui_on_mousebutton);
	event_manager_subscribe(event_manager, EVT_MOUSEMOTION, &gui_on_mousemotion);
	event_manager_subscribe(event_manager, EVT_MOUSEWHEEL, &gui_on_mousewheel);
	event_manager_subscribe(event_manager, EVT_TEXT_INPUT, &gui_on_textinput);

	success = true;
	return success;
}

void gui_upload_atlas(const void *image, int width, int height)
{
	gui_state->font_tex = texture_create("Gui_Font_Tex",
										 TU_DIFFUSE,
										 width, height,
										 GL_RGBA,
										 GL_RGBA,
										 GL_UNSIGNED_BYTE,
										 image);
	texture_set_param(gui_state->font_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(gui_state->font_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void gui_cleanup(void)
{
	struct Event_Manager* event_manager = game_state_get()->event_manager;
	event_manager_unsubscribe(event_manager, EVT_KEY_PRESSED, &gui_on_key);
	event_manager_unsubscribe(event_manager, EVT_KEY_RELEASED, &gui_on_key);
	event_manager_unsubscribe(event_manager, EVT_MOUSEBUTTON_PRESSED, &gui_on_mousebutton);
	event_manager_unsubscribe(event_manager, EVT_MOUSEBUTTON_RELEASED, &gui_on_mousebutton);
	event_manager_unsubscribe(event_manager, EVT_MOUSEMOTION, &gui_on_mousemotion);
	event_manager_unsubscribe(event_manager, EVT_MOUSEWHEEL, &gui_on_mousewheel);

    nk_font_atlas_clear(&gui_state->atlas);
    nk_free(&gui_state->context);
    shader_remove(gui_state->shader);
    texture_remove(gui_state->font_tex);
    glDeleteBuffers(1, &gui_state->vbo);
    glDeleteBuffers(1, &gui_state->ebo);
    nk_buffer_free(&gui_state->cmds);
    free(gui_state);
}

void gui_render(enum nk_anti_aliasing AA)
{
    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale;
    mat4 gui_mat;
	
    mat4_identity(&gui_mat);
    struct Game_State* game_state = game_state_get();
    window_get_size(game_state->window, &width, &height);
    window_get_drawable_size(game_state->window, &display_width, &display_height);
    mat4_ortho(&gui_mat, 0.f, display_width, display_height, 0.f, -100.f, 100.f);

    scale.x = (float)display_width/(float)width;
    scale.y = (float)display_height/(float)height;

    /* setup global state */
    glViewport(0,0,display_width,display_height);
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    /* setup program */
    shader_bind(gui_state->shader);
    glUniform1i(gui_state->uniform_tex, 0);
    shader_set_uniform(UT_MAT4, gui_state->uniform_proj, &gui_mat);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;

        /* allocate vertex and element buffer */
        glBindVertexArray(gui_state->vao);
        glBindBuffer(GL_ARRAY_BUFFER, gui_state->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui_state->ebo);

        glBufferData(GL_ARRAY_BUFFER, MAX_GUI_VERTEX_MEMORY, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_GUI_ELEMENT_MEMORY, NULL, GL_STREAM_DRAW);

        /* load vertices/elements directly into vertex/element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] =
	    {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Gui_Vertex, pos)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct Gui_Vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct Gui_Vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct Gui_Vertex);
            config.vertex_alignment = NK_ALIGNOF(struct Gui_Vertex);
            config.null = gui_state->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            struct nk_buffer vbuf, ebuf;
            nk_buffer_init_fixed(&vbuf, vertices, (nk_size)MAX_GUI_VERTEX_MEMORY);
            nk_buffer_init_fixed(&ebuf, elements, (nk_size)MAX_GUI_ELEMENT_MEMORY);
            nk_convert(&gui_state->context, &gui_state->cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &gui_state->context, &gui_state->cmds)
		{
            if (!cmd->elem_count) continue;
			texture_bind(cmd->texture.id);
            glScissor((GLint)(cmd->clip_rect.x * scale.x),
                (GLint)((height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * scale.y),
                (GLint)(cmd->clip_rect.w * scale.x),
                (GLint)(cmd->clip_rect.h * scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&gui_state->context);
    }

	shader_unbind();
	texture_unbind(gui_state->font_tex);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
}

void gui_on_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    char *text = platform_clipboard_text_get();
    if(text)
	{
		nk_textedit_paste(edit, text, nk_strlen(text));
		free(text);
	}
    (void)usr;
}

void gui_on_clipbard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    platform_clipboard_text_set(str);
    free(str);
}

void gui_on_key(const struct Event* event)
{
	assert(event->type == EVT_KEY_PRESSED || event->type == EVT_KEY_RELEASED);

	int                key      = event->key.key;
	bool               mod_ctrl = event->key.mod_ctrl;
    struct nk_context* ctx      = &gui_state->context;

	int down = event->type == EVT_KEY_PRESSED ? 1 : 0;
	
	if (key == KEY_RSHIFT || key == KEY_LSHIFT)
		nk_input_key(ctx, NK_KEY_SHIFT, down);
	else if (key == KEY_DELETE)
		nk_input_key(ctx, NK_KEY_DEL, down);
	else if (key == KEY_RETURN)
		nk_input_key(ctx, NK_KEY_ENTER, down);
	else if (key == KEY_TAB)
		nk_input_key(ctx, NK_KEY_TAB, down);
	else if (key == KEY_BACKSPACE)
		nk_input_key(ctx, NK_KEY_BACKSPACE, down);
	else if (key == KEY_HOME)
	{
		nk_input_key(ctx, NK_KEY_TEXT_START, down);
		nk_input_key(ctx, NK_KEY_SCROLL_START, down);
	}
	else if (key == KEY_END)
	{
		nk_input_key(ctx, NK_KEY_TEXT_END, down);
		nk_input_key(ctx, NK_KEY_SCROLL_END, down);
	}
	else if (key == KEY_PAGEDOWN)
	{
		nk_input_key(ctx, NK_KEY_SCROLL_DOWN, down);
	}
	else if (key == KEY_PAGEUP)
	{
		nk_input_key(ctx, NK_KEY_SCROLL_UP, down);
	}
	else if (key == KEY_Z)
		nk_input_key(ctx, NK_KEY_TEXT_UNDO, down && mod_ctrl);
	else if (key == KEY_R)
		nk_input_key(ctx, NK_KEY_TEXT_REDO, down && mod_ctrl);
	else if (key == KEY_C)
		nk_input_key(ctx, NK_KEY_COPY, down && mod_ctrl);
	else if (key == KEY_V)
		nk_input_key(ctx, NK_KEY_PASTE, down && mod_ctrl);
	else if (key == KEY_X)
		nk_input_key(ctx, NK_KEY_CUT, down && mod_ctrl);
	else if (key == KEY_B)
		nk_input_key(ctx, NK_KEY_TEXT_LINE_START, down && mod_ctrl);
	else if (key == KEY_E)
		nk_input_key(ctx, NK_KEY_TEXT_LINE_END, down && mod_ctrl);
	else if (key == KEY_UP)
		nk_input_key(ctx, NK_KEY_UP, down);
	else if (key == KEY_DOWN)
		nk_input_key(ctx, NK_KEY_DOWN, down);
	else if (key == KEY_LEFT)
	{
		if (mod_ctrl)
			nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, down);
		else nk_input_key(ctx, NK_KEY_LEFT, down);
	}
	else if (key == KEY_RIGHT)
	{
		if (mod_ctrl)
			nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, down);
		else
			nk_input_key(ctx, NK_KEY_RIGHT, down);
	}
}

void gui_on_mousebutton(const struct Event* event)
{
	assert(event->type == EVT_MOUSEBUTTON_PRESSED || event->type == EVT_MOUSEBUTTON_RELEASED);

	int                button = event->mousebutton.button;
	int                x      = event->mousebutton.x;
	int                y      = event->mousebutton.y;
	int                down   = event->type == EVT_MOUSEBUTTON_PRESSED ? 1 : 0;
	struct nk_context* ctx    = &gui_state->context;

	if(button == MSB_LEFT)   nk_input_button(ctx, NK_BUTTON_LEFT,   x, y, down);
	if(button == MSB_MIDDLE) nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
	if(button == MSB_RIGHT)	 nk_input_button(ctx, NK_BUTTON_RIGHT,  x, y, down);
}

void gui_on_mousemotion(const struct Event* event)
{
	int                x    = event->mousemotion.x;
	int                y    = event->mousemotion.y;
	int                xrel = event->mousemotion.xrel;
	int                yrel = event->mousemotion.yrel;
	struct nk_context* ctx  = &gui_state->context;

	if(ctx->input.mouse.grabbed)
	{
		int prev_x = (int)ctx->input.mouse.prev.x, prev_y = (int)ctx->input.mouse.prev.y;
		nk_input_motion(ctx, prev_x + xrel, prev_y + yrel);
	}
	else
	{
		nk_input_motion(ctx, x, y);
	}
}

void gui_on_textinput(const struct Event* event)
{
	struct nk_context *ctx = &gui_state->context;
	nk_glyph glyph;
	memcpy(glyph, event->text_input.text, NK_UTF_SIZE);
	nk_input_glyph(ctx, glyph);
}

void gui_on_mousewheel(const struct Event* event)
{
	int                x   = event->mousewheel.x;
	int                y   = event->mousewheel.y;
	struct nk_context* ctx = &gui_state->context;
	nk_input_scroll(ctx, nk_vec2(x, y));
}

struct Gui_State* gui_state_get(void)
{
	return gui_state;
}

void gui_input_begin(void)
{
	nk_input_begin(&gui_state->context);
}

void gui_input_end(void)
{
	nk_input_end(&gui_state->context);
}

void gui_font_set(const char* font_name, float font_size)
{
	assert(font_name && font_size > 1.f);
	struct nk_font_atlas* atlas = &gui_state->atlas;
	long size = 0;
	char* font_file_name = str_new("fonts/%s", font_name);
    char* font_data = io_file_read(DIRT_INSTALL, font_file_name, "rb", &size);
	free(font_file_name);
	if(!font_data)
	{
		log_error("gui:init", "Could not load font %s, reverting to default", font_name);
		if(!gui_state->current_font) gui_font_set_default();
	}
	else
	{
		if(gui_state->current_font)
		{
			nk_font_atlas_clear(&gui_state->atlas);
			texture_remove(gui_state->font_tex);
			gui_state->font_tex = -1;
			gui_state->current_font = NULL;
		}
		const void *image = NULL;
		int w = 0, h = 0;
		nk_font_atlas_init_default(atlas);
		nk_font_atlas_begin(atlas);		
		struct nk_font *new_font = nk_font_atlas_add_from_memory(atlas, font_data, size, font_size, NULL);
		image = nk_font_atlas_bake(atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
		gui_upload_atlas(image, w, h);
		nk_font_atlas_end(atlas, nk_handle_id((int)gui_state->font_tex), &gui_state->null);
		if(new_font)
		{
			nk_style_set_font(&gui_state->context, &new_font->handle);
			log_message("Set %s as current font", font_name);
			gui_state->current_font = new_font;
		}
		else
		{
			log_error("gui:init", "Could not add font %s, reverting to default", font_name);
			gui_font_set_default();
		}
		free(font_data);
	}
}

void gui_font_set_default(void)
{
	if(gui_state->current_font)
	{
		nk_font_atlas_clear(&gui_state->atlas);
		texture_remove(gui_state->font_tex);
	}
	struct nk_font_atlas* atlas = &gui_state->atlas;
	nk_font_atlas_init_default(atlas);
	const void *image = NULL;
	int w = 0, h = 0;
	image = nk_font_atlas_bake(atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
	gui_upload_atlas(image, w, h);
	nk_style_set_font(&gui_state->context, &atlas->default_font->handle);
	gui_state->current_font = atlas->default_font;
	nk_font_atlas_end(atlas, nk_handle_id((int)gui_state->font_tex), &gui_state->null);
	log_message("Set default font");
}

void gui_theme_set(enum Gui_Theme theme)
{
    struct nk_color table[NK_COLOR_COUNT];
    if(theme == GT_WHITE)
	{
        table[NK_COLOR_TEXT] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_HEADER] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_BORDER] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(185, 185, 185, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(170, 170, 170, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(120, 120, 120, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SELECT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(80, 80, 80, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(70, 70, 70, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(60, 60, 60, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_EDIT] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(0, 0, 0, 255);
        table[NK_COLOR_COMBO] = nk_rgba(175, 175, 175, 255);
        table[NK_COLOR_CHART] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(45, 45, 45, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(180, 180, 180, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(140, 140, 140, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(150, 150, 150, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(160, 160, 160, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(180, 180, 180, 255);
        nk_style_from_table(&gui_state->context, table);
    }
	else if(theme == GT_RED)
	{
        table[NK_COLOR_TEXT] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(30, 33, 40, 215);
        table[NK_COLOR_HEADER] = nk_rgba(181, 45, 69, 220);
        table[NK_COLOR_BORDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(190, 50, 70, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(195, 55, 75, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 60, 60, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SELECT] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(181, 45, 69, 255);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(186, 50, 74, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(191, 55, 79, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_EDIT] = nk_rgba(51, 55, 67, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(190, 190, 190, 255);
        table[NK_COLOR_COMBO] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART] = nk_rgba(51, 55, 67, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(170, 40, 60, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(30, 33, 40, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(181, 45, 69, 220);
        nk_style_from_table(&gui_state->context, table);
    }
	else if(theme == GT_BLUE)
	{
        table[NK_COLOR_TEXT] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(202, 212, 214, 215);
        table[NK_COLOR_HEADER] = nk_rgba(137, 182, 224, 220);
        table[NK_COLOR_BORDER] = nk_rgba(140, 159, 173, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(142, 187, 229, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(147, 192, 234, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(182, 215, 215, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SELECT] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(177, 210, 210, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(137, 182, 224, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(142, 188, 229, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(147, 193, 234, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_EDIT] = nk_rgba(210, 210, 210, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(20, 20, 20, 255);
        table[NK_COLOR_COMBO] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(137, 182, 224, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba( 255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(190, 200, 200, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(64, 84, 95, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(70, 90, 100, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 95, 105, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(156, 193, 220, 255);
        nk_style_from_table(&gui_state->context, table);
    }
	else if(theme == GT_DARK)
	{
        table[NK_COLOR_TEXT] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_WINDOW] = nk_rgba(57, 67, 71, 215);
        table[NK_COLOR_HEADER] = nk_rgba(51, 51, 56, 220);
        table[NK_COLOR_BORDER] = nk_rgba(46, 46, 46, 255);
        table[NK_COLOR_BUTTON] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_BUTTON_HOVER] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(63, 98, 126, 255);
        table[NK_COLOR_TOGGLE] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(45, 53, 56, 255);
        table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SELECT] = nk_rgba(57, 67, 61, 255);
        table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SLIDER] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(48, 83, 111, 245);
        table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_PROPERTY] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_EDIT] = nk_rgba(50, 58, 61, 225);
        table[NK_COLOR_EDIT_CURSOR] = nk_rgba(210, 210, 210, 255);
        table[NK_COLOR_COMBO] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_CHART_COLOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 0, 0, 255);
        table[NK_COLOR_SCROLLBAR] = nk_rgba(50, 58, 61, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(48, 83, 111, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(53, 88, 116, 255);
        table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(58, 93, 121, 255);
        table[NK_COLOR_TAB_HEADER] = nk_rgba(48, 83, 111, 255);
        nk_style_from_table(&gui_state->context, table);
    }
	else if(theme == GT_DEFAULT)
	{
        nk_style_default(&gui_state->context);
    }
	else
	{
		log_error("gui:theme_set", "Unrecognized theme, reverting to default");
		nk_style_default(&gui_state->context);
	}
}
