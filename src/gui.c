#define NK_IMPLEMENTATION
#include "gui.h"
#include "platform.h"
#include "linmath.h"
#include "log.h"
#include "shader.h"
#include "texture.h"
#include "game.h"
#include "input.h"
#include "renderer.h"
#include "file_io.h"

#include <string.h>
#include <stdlib.h>

#define FONT_SIZE 14.f

struct Gui_Vertex
{
    vec2 pos, uv;
    nk_byte col[4];
};

static void gui_handle_clipbard_copy(nk_handle usr, const char *text, int len);
static void gui_handle_clipbard_paste(nk_handle usr, struct nk_text_edit *edit);
static void gui_handle_textinput_event(const char* text);
static void gui_upload_atlas(const void *image, int width, int height);
static void gui_font_stash_begin(struct nk_font_atlas **atlas);
static void gui_font_stash_end(void);

static struct Gui_State* gui_state = NULL;

int gui_init(void)
{
	int success = 0;
	gui_state = malloc(sizeof(*gui_state));
	if(!gui_state)
	{
		log_error("gui:init", "Malloc failed, out of memory");
		return success;
	}

	nk_init_default(&gui_state->context, 0);
    gui_state->context.clip.copy = gui_handle_clipbard_copy;
    gui_state->context.clip.paste = gui_handle_clipbard_paste;
    gui_state->context.clip.userdata = nk_handle_ptr(0);
	
    nk_buffer_init_default(&gui_state->cmds);
	gui_state->shader = shader_create("gui.vert", "gui.frag");

	if(gui_state->shader < 0)
	{
		log_error("gui:init", "Failed to create shader for gui");
		free(gui_state);
		return success;
	}

    gui_state->uniform_tex  = shader_get_uniform_location(gui_state->shader, "sampler");
    gui_state->uniform_proj = shader_get_uniform_location(gui_state->shader, "proj_mat");
    gui_state->attrib_pos   = shader_get_attribute_location(gui_state->shader, "vPosition");
    gui_state->attrib_uv    = shader_get_attribute_location(gui_state->shader, "vUV");
    gui_state->attrib_col   = shader_get_attribute_location(gui_state->shader, "vColor");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct Gui_Vertex);
        size_t vp = offsetof(struct Gui_Vertex, pos);
        size_t vt = offsetof(struct Gui_Vertex, uv);
        size_t vc = offsetof(struct Gui_Vertex, col);

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

	platform_textinput_callback_set(&gui_handle_textinput_event);

	renderer_check_glerror("Before font upload check");
	/* Load default font and roboto */
	struct nk_font_atlas* atlas = NULL;
	gui_font_stash_begin(&atlas);
	gui_font_stash_end();
	long size = 0;
	char* font_data = io_file_read("fonts/roboto.ttf", "rb", &size);
	if(!font_data)
	{
		log_error("gui:init", "Could not load font %s", "roboto.ttf");
	}
	else
	{
		gui_font_stash_begin(&atlas);
		struct nk_font *roboto = nk_font_atlas_add_from_memory(atlas, font_data, size, FONT_SIZE, NULL);
		gui_font_stash_end();
		if(roboto)
			nk_style_set_font(&gui_state->context, &roboto->handle);
		else
			log_error("gui:init", "Could not add font %s", "roboto.ttf");
		free(font_data);
	}
	//nk_font_atlas_cleanup(atlas);
	success = 1;
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
	nk_font_atlas_clear(&gui_state->atlas);
    nk_free(&gui_state->context);
	shader_remove(gui_state->shader);
	texture_remove(gui_state->font_tex);
    glDeleteBuffers(1, &gui_state->vbo);
    glDeleteBuffers(1, &gui_state->ebo);
    nk_buffer_free(&gui_state->cmds);
    free(gui_state);
}

void gui_render(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    int width, height;
    int display_width, display_height;
    struct nk_vec2 scale;
	mat4 gui_mat;
	
	mat4_identity(&gui_mat);
	struct Game_State* game_state = game_state_get();
	window_get_size(game_state->window, &width, &height);
    window_get_drawable_size(game_state->window, &display_width, &display_height);
	mat4_ortho(&gui_mat, 0, width, height, 0, -100, 100);

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

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

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
            nk_buffer_init_fixed(&vbuf, vertices, (nk_size)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (nk_size)max_element_buffer);
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

void gui_handle_clipbard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    char *text = platform_clipboard_text_get();
    if(text)
	{
		nk_textedit_paste(edit, text, nk_strlen(text));
		free(text);
	}
    (void)usr;
}

void gui_handle_clipbard_copy(nk_handle usr, const char *text, int len)
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

void gui_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&gui_state->atlas);
    nk_font_atlas_begin(&gui_state->atlas);
    *atlas = &gui_state->atlas;
}

void gui_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&gui_state->atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    gui_upload_atlas(image, w, h);
    nk_font_atlas_end(&gui_state->atlas, nk_handle_id((int)gui_state->font_tex), &gui_state->null);
    if (gui_state->atlas.default_font)
        nk_style_set_font(&gui_state->context, &gui_state->atlas.default_font->handle);
}

void gui_handle_keyboard_event(int key, int state, int mod_ctrl, int mod_shift)
{
    struct nk_context *ctx = &gui_state->context;
	int down               = (state == KS_PRESSED);
	
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

void gui_handle_mousebutton_event(int button, int state, int x, int y)
{
	int down = state == KS_PRESSED;
	struct nk_context *ctx = &gui_state->context;
	if(button == MB_LEFT)   nk_input_button(ctx, NK_BUTTON_LEFT,   x, y, down);
	if(button == MB_MIDDLE)	nk_input_button(ctx, NK_BUTTON_MIDDLE, x, y, down);
	if(button == MB_RIGHT)	nk_input_button(ctx, NK_BUTTON_RIGHT,  x, y, down);
}

void gui_handle_mousemotion_event(int x, int y, int xrel, int yrel)
{
	struct nk_context *ctx = &gui_state->context;
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

void gui_handle_textinput_event(const char* text)
{
	struct nk_context *ctx = &gui_state->context;
	nk_glyph glyph;
	memcpy(glyph, text, NK_UTF_SIZE);
	nk_input_glyph(ctx, glyph);
}

void gui_handle_mousewheel_event(int x, int y)
{
	struct nk_context *ctx = &gui_state->context;
	nk_input_scroll(ctx,(float)y);
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
