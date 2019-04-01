#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include "../common/num_types.h"

enum Video_Drivers_Linux
{
    VD_X11 = 0,
    VD_WAYLAND,
    VD_DUMMY
};

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
bool           window_fullscreen_set(struct Window* window, bool fullscreen);

// Platform functions
bool        platform_init(void);
bool        platform_init_video(void);
void        platform_cleanup(void);
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
void*       platform_load_library(const char* name);
void        platform_unload_library(void* library_handle);
void*       platform_load_function(void* library_handle, const char* func_name);
bool        platform_load_gl(const char* name);
void        platform_unload_gl(void);
void*       platform_load_function_gl(const char* func_name);

#endif
