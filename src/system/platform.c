#include "platform.h"
#include "../common/log.h"
#include "config_vars.h"
#include "../common/hashmap.h"
#include "../common/string_utils.h"
#include "../common/memory_utils.h"

#include <SDL.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

struct Window
{
    void*         sdl_window;
    SDL_GLContext gl_context;
    bool          is_fullscreen;
};

struct Window* window_create(const char* title, int width, int height, int msaa, int msaa_levels, bool vsync)
{
    struct Window* new_window = NULL;
    if(!new_window)
    {
		new_window = memory_allocate(sizeof(*new_window));
		if(!new_window)
		{
			log_error("window_create", "Out of memory");
			return NULL;
		}
		new_window->sdl_window    = NULL;
		new_window->gl_context    = NULL;
		new_window->is_fullscreen = false;
    }

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if(msaa)
    {
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, msaa);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, msaa_levels);
    }
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    //SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#ifdef GL_DEBUG_CONTEXT
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
    SDL_Window* sdl_window = SDL_CreateWindow(title,
											  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
											  width, height,
											  SDL_WINDOW_OPENGL |
											  SDL_WINDOW_RESIZABLE |
											  SDL_WINDOW_INPUT_FOCUS |
											  SDL_WINDOW_MOUSE_FOCUS);
    if(!sdl_window)
    {
		log_error("window_create:SDL_CreateWindow", "Could not create window :  %s", SDL_GetError());
		memory_free(new_window);
		new_window = NULL;
		return new_window;
    }
	
    new_window->sdl_window = sdl_window;
    SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
    if(!gl_context)
    {
		log_error("window_create:SDL_GL_CreateContext", "Failed to create GL context : %s", SDL_GetError());
        window_destroy(new_window);
		memory_free(new_window);
		new_window = NULL;
		return new_window;
    }

    new_window->gl_context = gl_context;
    SDL_Window* current_window = SDL_GL_GetCurrentWindow();
    SDL_GLContext current_context = SDL_GL_GetCurrentContext();
    SDL_GL_MakeCurrent((SDL_Window*)new_window->sdl_window, new_window->gl_context);
    
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);	/* 0: Vsync disabled, 1: Vsync enabled*/
    if(current_window && current_context) SDL_GL_MakeCurrent(current_window, current_context);

    int major = 0, minor = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    log_message("Window created and initialized with opengl core context %d.%d", major, minor);
	
    return new_window;
}

bool window_fullscreen_set(struct Window* window, bool fullscreen)
{
	int success = false;
    int rc = SDL_SetWindowFullscreen(window->sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    if(rc == 0)
    {
		window->is_fullscreen = fullscreen;
		success = true;
		log_message("Window set to %s mode", fullscreen ? "fullscreen" : "windowed");
		int w, h;
		window_get_size(window, &w, &h);
		log_message("Window size   : %dx%d", w, h);
		window_get_drawable_size(window, &w, &h);
		log_message("Drawable size : %dx%d", w, h);
    }
    else
    {
		log_error("platform:window_fullscreen", "window_fullscreen_set failed, %s", SDL_GetError());
    }
    return success;
}

bool window_fullscreen_get(struct Window* window)
{
	return window->is_fullscreen;
}

void window_make_context_current(struct Window* window)
{
    SDL_GL_MakeCurrent((SDL_Window*)window->sdl_window, window->gl_context);
}

void window_show(struct Window* window)
{
    SDL_ShowWindow((SDL_Window*)window->sdl_window);
}

void window_hide(struct Window* window)
{
    SDL_HideWindow((SDL_Window*)window->sdl_window);
}

void window_raise(struct Window* window)
{
    SDL_RaiseWindow((SDL_Window*)window->sdl_window);
}

void window_destroy(struct Window* window)
{
    if(window->gl_context) SDL_GL_DeleteContext(window->gl_context);
    SDL_DestroyWindow((SDL_Window*)window->sdl_window);
}

void window_set_size(struct Window* window, int width, int height)
{
    SDL_SetWindowSize((SDL_Window*)window->sdl_window, width, height);
}

void window_get_size(struct Window* window, int* out_width, int* out_height)
{
    SDL_GetWindowSize((SDL_Window*)window->sdl_window, out_width, out_height);
}

void window_get_drawable_size(struct Window* window, int* out_width, int* out_height)
{
    SDL_GL_GetDrawableSize((SDL_Window*)window->sdl_window, out_width, out_height);
}

void window_swap_buffers(struct Window* window)
{
    SDL_GL_SwapWindow(window->sdl_window);
}

bool platform_init(void)
{
    bool success = true;
    if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
    {
        success = false;
		if(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL Init failed", SDL_GetError(), NULL) != 0)
			log_to_stdout("platform_init", "SDL Init failed : %s", SDL_GetError());
    }

    return success;
}

bool platform_init_video()
{
    bool success = true;
    const char* video_driver_str = NULL;

#ifdef __linux__
    struct Hashmap* cvars = config_vars_get();
    int driver_type = hashmap_bool_get(cvars, "video_driver_linux");
    switch (driver_type)
    {
    case VD_WAYLAND: video_driver_str = "wayland"; break;
    case VD_X11:     video_driver_str = "x11";     break;
    };
#endif

    if(SDL_VideoInit(video_driver_str) != 0)
    {
        log_error("platform:init_video", "Failed to initialize video subsystem, SDL : (%s)", SDL_GetError());
        success = false;
    }
    else
    {
        log_message("Video subsystem initialized with %s", SDL_GetCurrentVideoDriver());
    }
    return success;
}

void platform_cleanup(void)
{
    SDL_VideoQuit();
    SDL_Quit();
}

int platform_is_key_pressed(int key)
{
    /* Returns 1 if key is pressed, 0 otherwise */
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
    return keyboard_state[SDL_GetScancodeFromKey(key)];
}

int platform_mousebutton_state_get(uint button)
{
    int pressed = 0;
    uint32 current_button_state = SDL_GetMouseState(NULL, NULL);
    if((current_button_state & SDL_BUTTON(button))) pressed = 1;
    return pressed;
}

void platform_mouse_position_get(int* x, int* y)
{
    SDL_GetMouseState(x, y);
}

void platform_mouse_position_set(struct Window* window, int x, int y)
{
    SDL_WarpMouseInWindow(window->sdl_window, x, y);
}

void platform_mouse_global_position_set(int x, int y)
{
    SDL_WarpMouseGlobal(x, y);
}

void platform_mouse_relative_mode_set(int relative_mode)
{
    SDL_SetRelativeMouseMode(relative_mode ? SDL_TRUE : SDL_FALSE);
}

int platform_mouse_relative_mode_get(void)
{
    return SDL_GetRelativeMouseMode() == SDL_TRUE ? 1 : 0;
}

uint32 platform_ticks_get(void)
{
    return SDL_GetTicks();
}

void platform_mouse_delta_get(int* x, int* y)
{
    SDL_GetRelativeMouseState(x, y);
}

char* platform_install_directory_get(void)
{
    char* returned_path = SDL_GetBasePath();
    char* path = NULL;
    if(returned_path)
    {
		path = str_new(returned_path);
		SDL_free(returned_path);
    }
    return path;
}

void platform_clipboard_text_set(const char* text)
{
    SDL_SetClipboardText(text);
}

char* platform_clipboard_text_get(void)
{
    char* returned_text = SDL_GetClipboardText();
    char* text = NULL;
    if(returned_text)
    {
		text = str_new(returned_text);
		SDL_free(returned_text);
    }
    return text;
}

int platform_key_from_name(const char* key_name)
{
    if(!key_name || strlen(key_name) == 0) return SDLK_UNKNOWN;
	
    /* Remove leading/trailing spaces, preserve spaces in-between */
#define max_name_len  30
    char trimmed_key_name[max_name_len] = {'\0'};

    int start = 0;
    while(isspace(key_name[start]) != 0) start++;
	
    size_t end = strlen(key_name) - 1;
    while(isspace(key_name[end]) != 0) end--;

    strncpy(trimmed_key_name, &key_name[start], (end - start) + 1);
    int key =  SDL_GetKeyFromName(trimmed_key_name);
    /*if(key == SDLK_UNKNOWN)
      log_error("platform:key_from_name", "Unrecognized key '%s', SDL (%s)", trimmed_key_name, SDL_GetError());*/
    return key;
}

const char* platform_key_name_get(int key)
{
    if(key < 0) return "SDLK_UNKNOWN";
    return SDL_GetKeyName(key);
}

char* platform_user_directory_get(const char* organization, const char* application)
{
    char* user_directory = NULL;
    char* temp_path      = SDL_GetPrefPath(organization, application);
    if(temp_path)
    {
		user_directory = str_new(temp_path);
		SDL_free(temp_path);
    }
    else
    {
        log_to_stdout("ERR(platform:user_directory_get) Error getting user directory, %s", SDL_GetError());
    }
    return user_directory;
}

void* platform_load_library(const char *name)
{
    char* install_dir = platform_install_directory_get();
#define MAX_LIB_NAME_LEN 256
    char lib_name[MAX_LIB_NAME_LEN];
    memset(lib_name, '\0', MAX_LIB_NAME_LEN);
#ifdef __linux__
    snprintf(lib_name, MAX_LIB_NAME_LEN, "./lib%s.so", name);
#elif defined(_MSC_VER)
    snprintf(lib_name, MAX_LIB_NAME_LEN, "%s.dll", name);
#elif defined(__MING32__) || defined(__MINGW64__)
    snprintf(lib_name, MAX_LIB_NAME_LEN, "./%s.dll", name);
#endif
    void* lib_handle = SDL_LoadObject(lib_name);
    if(!lib_handle) log_error("platform:load_library", "Failed to load library '%s', SDL : (%s)", lib_name, SDL_GetError());
    memory_free(install_dir);
    return lib_handle;
}

void* platform_load_function(void *library_handle, const char *func_name)
{
    assert(library_handle);
    void* func_ptr = SDL_LoadFunction(library_handle, func_name);
    if(!func_ptr) log_error("platform:load_function", "Failed to load function '%s' from library, SDL : (%s)", func_name, SDL_GetError());
    return func_ptr;
}

void platform_unload_library(void* library_handle)
{
    assert(library_handle);
    SDL_UnloadObject(library_handle);
}

bool platform_load_gl(const char* name)
{
    if(SDL_GL_LoadLibrary(name) != 0)
    {
        log_error("platform:load_gl", "Failed to load GL'%s', SDL : (%s)", name, SDL_GetError());
        return false;
    }
    else
    {
        log_message("OpenGL loaded");
    }
    return true;
}

void platform_unload_gl(void)
{
    SDL_GL_UnloadLibrary();
    log_message("OpenGL unloaded");
}

void* platform_load_function_gl(const char* func_name)
{
    void* func_ptr = SDL_GL_GetProcAddress(func_name);
    if(!func_ptr) log_error("platform:load_function_gl", "Failed to load GL function '%s' from library, SDL : (%s)", func_name, SDL_GetError());
    return func_ptr;
}

int platform_timer_add(uint32 interval_ms, Timer_Callback_Func callback, void* param)
{
    SDL_TimerID timer_id = SDL_AddTimer(interval_ms, callback, param);
    if(timer_id == 0)
        log_error("platform:tiemr_add", "Failed to add timer, SDL : (%s)", SDL_GetError());
    return timer_id;
}

bool platform_timer_remove(int timer_id)
{
    return (bool)SDL_RemoveTimer(timer_id);
}
