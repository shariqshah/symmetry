#include "platform.h"
#include "log.h"
#include "config_vars.h"
#include "hashmap.h"
#include "string_utils.h"

#include <SDL2/SDL.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

struct Window
{
	void*         sdl_window;
	SDL_GLContext gl_context;
	int           is_fullscreen;
};

struct Platform_State
{
	Keyboard_Event_Func     on_keyboard_func;
	Mousebutton_Event_Func  on_mousebutton_func;
	Mousemotion_Event_Func  on_mousemotion_func;
	Mousewheel_Event_Func   on_mousewheel_func;
	Windowresize_Event_Func on_windowresize_func;
	Textinput_Event_Func    on_textinput_func;
};

/* TODO: Find a better way to handle internal state */
static struct Platform_State* platform_state = NULL;

struct Window* window_create(const char* title, int width, int height, int msaa, int msaa_levels)
{
	struct Window* new_window = NULL;
	if(!new_window)
	{
		new_window = malloc(sizeof(*new_window));
		if(!new_window)
		{
			log_error("window_create", "Out of memory");
			return NULL;
		}
		new_window->sdl_window    = NULL;
		new_window->gl_context    = NULL;
		new_window->is_fullscreen = 0;
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
		free(new_window);
		new_window = NULL;
		return new_window;
	}
	
	new_window->sdl_window = sdl_window;
	SDL_GLContext gl_context = SDL_GL_CreateContext(sdl_window);
	if(!gl_context)
	{
		log_error("window_create:SDL_GL_CreateContext", "Failed to create GL context : %s", SDL_GetError());
        window_destroy(new_window);
		free(new_window);
		new_window = NULL;
		return new_window;
	}

	new_window->gl_context = gl_context;
	SDL_Window* current_window = SDL_GL_GetCurrentWindow();
	SDL_GLContext current_context = SDL_GL_GetCurrentContext();
	SDL_GL_MakeCurrent((SDL_Window*)new_window->sdl_window, new_window->gl_context);
	SDL_GL_SetSwapInterval(1);	/* 0: Vsync disabled, 1: Vsync enabled*/
	if(current_window && current_context) SDL_GL_MakeCurrent(current_window, current_context);

	int major = 0, minor = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
	log_message("Window created and initialized with opengl core context %d.%d", major, minor);
	return new_window;
}

int window_fullscreen_set(struct Window* window, int fullscreen)
{
	int success = 0;
	int rc = SDL_SetWindowFullscreen(window->sdl_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	if(rc == 0)
	{
		window->is_fullscreen = fullscreen ? 1 : 0;
		success = 1;
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
	else
	{
		platform_state = malloc(sizeof(*platform_state));
		if(!platform_state)
		{
			if(SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Allocation Failure", "Memory allocation failed, out of memory!", NULL) != 0)
				log_to_stdout("platform_init", "Could not create platform state, out of memory");
            success = false;
		}
		else
		{
			platform_state->on_keyboard_func     = NULL;
			platform_state->on_mousebutton_func  = NULL;
			platform_state->on_mousemotion_func  = NULL;
			platform_state->on_mousewheel_func   = NULL;
			platform_state->on_textinput_func    = NULL;
			platform_state->on_windowresize_func = NULL;
		}
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
	if(platform_state) free(platform_state);
	platform_state = NULL;
    SDL_VideoQuit();
	SDL_Quit();
}

void platform_poll_events(bool* out_quit)
{
	static SDL_Event event;
	while(SDL_PollEvent(&event) != 0)
	{
		switch(event.type)
		{
		case SDL_QUIT:
			*out_quit = 1;
			break;
		case SDL_KEYDOWN: case SDL_KEYUP:
		{
			int scancode  = event.key.keysym.scancode;
			int key       = event.key.keysym.sym;
			int state     = event.key.state;
			int repeat    = event.key.repeat;
			int mod_ctrl  = (event.key.keysym.mod & KMOD_CTRL)  ? 1 : 0;
			int mod_shift = (event.key.keysym.mod & KMOD_SHIFT) ? 1 : 0;
			int mod_alt   = (event.key.keysym.mod & KMOD_ALT)   ? 1 : 0;
			platform_state->on_keyboard_func(key, scancode, state, repeat, mod_ctrl, mod_shift, mod_alt);
			//log_message("Key name : %s", SDL_GetKeyName(key));
			break;
		}
		case SDL_MOUSEBUTTONDOWN: case SDL_MOUSEBUTTONUP:
		{
			int button     = event.button.button;
			int state      = event.button.state;
			int num_clicks = event.button.clicks;
			int x          = event.button.x;
			int y          = event.button.y;
			platform_state->on_mousebutton_func(button, state, x, y, num_clicks);
			break;
		}
		case SDL_MOUSEMOTION:
		{
			int xrel = event.motion.xrel;
			int yrel = event.motion.yrel;
			int x    = event.motion.x;
			int y    = event.motion.y;
			platform_state->on_mousemotion_func(x, y, xrel, yrel);
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			int x = event.wheel.x;
			int y = event.wheel.y;
			platform_state->on_mousewheel_func(x, y);
			break;
		}
		case SDL_TEXTINPUT:
		{
			platform_state->on_textinput_func(event.text.text);
			break;
		}
		case SDL_WINDOWEVENT:
		{
			if(event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				platform_state->on_windowresize_func(event.window.data1, event.window.data2);
			}
		}
		break;
		}
	}
}

void platform_keyboard_callback_set(Keyboard_Event_Func func)
{
	platform_state->on_keyboard_func = func;
}

void platform_mousebutton_callback_set(Mousebutton_Event_Func func)
{
	platform_state->on_mousebutton_func = func;
}

void platform_mousemotion_callback_set(Mousemotion_Event_Func func)
{
	platform_state->on_mousemotion_func = func;
}

void platform_mousewheel_callback_set(Mousewheel_Event_Func func)
{
	platform_state->on_mousewheel_func = func;
}

void platform_textinput_callback_set(Textinput_Event_Func func)
{
	platform_state->on_textinput_func = func;
}

void platform_windowresize_callback_set(Windowresize_Event_Func func)
{
	platform_state->on_windowresize_func = func;
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
	if((current_button_state & SDL_BUTTON(button)) > 0) pressed = 1;
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

	const char* start_ptr = &key_name[0];
	while(isspace(start_ptr[0])) start_ptr++;
		
	const char* end_ptr = &key_name[strlen(key_name)];
	while(isspace(end_ptr[0])) end_ptr--;
 
	strncpy(trimmed_key_name, start_ptr, (end_ptr - start_ptr));
	return SDL_GetKeyFromName(trimmed_key_name);
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
		log_error("platform:user_directory_get", "Error getting user directory, %s", SDL_GetError());
	}
	return user_directory;
}

void* platform_load_library(const char *name)
{
#define MAX_LIB_NAME_LEN 256
    char lib_name[MAX_LIB_NAME_LEN];
    memset(lib_name, '\0', MAX_LIB_NAME_LEN);
#ifdef __linux__
    snprintf(lib_name, MAX_LIB_NAME_LEN, "./lib%s.so", name);
#else
    snprintf(lib_name, MAX_LIB_NAME_LEN, "%s.dll", name);
#endif
    void* lib_handle = SDL_LoadObject(lib_name);
    if(!lib_handle) log_error("platform:load_library", "Failed to load library '%s', SDL : (%s)", lib_name, SDL_GetError());
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
