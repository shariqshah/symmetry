#include "platform.h"
#include "log.h"
#include "input.h"
#include <SDL2/SDL.h>

//#define GL_DEBUG_CONTEXT

struct Window
{
	void*         sdl_window;
	SDL_GLContext gl_context;
};

struct Platform_State
{
	Keyboard_Event_Func     on_keyboard_func;
	Mousebutton_Event_Func  on_mousebutton_func;
	Mousemotion_Event_Func  on_mousemotion_func;
	Windowresize_Event_Func on_windowresize_func;
};

/* TODO: Find a better way to handle internal state */
static struct Platform_State* platform_state = NULL;

struct Window* window_create(const char* title, int width, int height)
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
		new_window->sdl_window = NULL;
		new_window->gl_context = NULL;
	}

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
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

void window_cleanup(void)
{
	
}

void window_set_size(struct Window* window, int width, int height)
{
	SDL_SetWindowSize((SDL_Window*)window->sdl_window, width, height);
}

void window_get_size(struct Window* window, int* out_width, int* out_height)
{
	SDL_GetWindowSize((SDL_Window*)window->sdl_window, out_width, out_height);
}

void window_swap_buffers(struct Window* window)
{
	SDL_GL_SwapWindow(window->sdl_window);
}

int platform_init(void)
{
	int success = 1;
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		success = 0;
		log_error("platform_init", "SDL Init failed : %s", SDL_GetError());
	}
	else
	{
		platform_state = malloc(sizeof(*platform_state));
		if(!platform_state)
		{
			log_error("platform_init", "Could not create platform state, out of memory");
			success = 0;
		}
	}
	return success;
}

void platform_cleanup(void)
{
	if(platform_state) free(platform_state);
	SDL_Quit();
}

void platform_poll_events(int* out_quit)
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
			int mod_ctrl  = (event.key.keysym.mod & KMOD_CTRL);
			int mod_shift = (event.key.keysym.mod & KMOD_SHIFT);
			platform_state->on_keyboard_func(key, scancode, state, mod_ctrl, mod_shift);
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
		}
		case SDL_WINDOWEVENT:
		{
			if(event.window.type == SDL_WINDOWEVENT_RESIZED)
			{
				/* Notify renderer here */
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

void platform_windowresize_callback_set(Windowresize_Event_Func func)
{
	platform_state->on_windowresize_func = func;
}

int platform_key_state_get(int key)
{
	/* Returns 1 if key is pressed, 0 otherwise */
	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	return keyboard_state[SDL_GetScancodeFromKey(key)];
}

int platform_mousebutton_state_get(uint button)
{
	uint32 current_button_state = SDL_GetMouseState(NULL, NULL);
	//return (current_button_state & button);
	return current_button_state & button;
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

uint32 platform_get_ticks(void)
{
	return SDL_GetTicks();
}
