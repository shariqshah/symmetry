#ifndef PLATFORM_H
#define PLATFORM_H

#include "num_types.h"

// Function Pointer decls
typedef void (*Keyboard_Event_Func)     (int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
typedef void (*Mousebutton_Event_Func)  (int button, int state, int x, int y, int8 num_clicks);
typedef void (*Mousemotion_Event_Func)  (int x, int y, int xrel, int yrel);
typedef void (*Mousewheel_Event_Func)   (int x, int y);
typedef void (*Windowresize_Event_Func) (int x, int y);
typedef void (*Textinput_Event_Func)    (const char* text);

// Window Related functions
struct Window;

struct Window* window_create(const char* title, int width, int height, int msaa, int msaa_levels);
void           window_destroy(struct Window* window);
void           window_show(struct Window* window);
void           window_hide(struct Window* window);
void           window_raise(struct Window* window);
void           window_make_context_current(struct Window* window);
void           window_set_size(struct Window* window, int width, int height);
void           window_get_size(struct Window* window, int* out_width, int* out_height);
void           window_get_drawable_size(struct Window* window, int* out_width, int* out_height);
void           window_swap_buffers(struct Window* window);
int            window_fullscreen_set(struct Window* window, int fullscreen);

// Platform functions
int         platform_init(void);
void        platform_cleanup(void);
void        platform_poll_events(int* out_quit);
void        platform_keyboard_callback_set(Keyboard_Event_Func func);
void        platform_mousebutton_callback_set(Mousebutton_Event_Func func);
void        platform_mousemotion_callback_set(Mousemotion_Event_Func func);
void        platform_mousewheel_callback_set(Mousewheel_Event_Func func);
void        platform_windowresize_callback_set(Windowresize_Event_Func func);
void        platform_textinput_callback_set(Textinput_Event_Func func);
int         platform_is_key_pressed(int key);
int         platform_mousebutton_state_get(uint button);
void        platform_mouse_position_get(int* x, int* y);
void        platform_mouse_delta_get(int* x, int* y); // Use with relative mouse mode
void        platform_mouse_position_set(struct Window* window, int x, int y);
void        platform_mouse_global_position_set(int x, int y);
void        platform_mouse_relative_mode_set(int relative_mode);
int         platform_mouse_relative_mode_get(void);
uint32      platform_ticks_get(void);
char*       platform_install_directory_get(void);
char*       platform_user_directory_get(const char* organization, const char* application);
void        platform_clipboard_text_set(const char* text);
char*       platform_clipboard_text_get(void);
int         platform_key_from_name(const char* key_name);
const char* platform_key_name_get(int key);

#endif
