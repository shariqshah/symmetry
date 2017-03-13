#include "gl_load.h"
#include "log.h"
#include <SDL2/SDL.h>


#ifndef USE_GLAD

#define GLE(ret, name, ...) name##proc * gl##name;
SYMMETRY_GL_LIST
#undef GLE

#endif

int gl_load_library(void)
{
	int success = 1;
	if(SDL_GL_LoadLibrary(NULL) < 0)
	{
		success = 0;
		log_error("gl_load_library", "Failed to load GL library %s", SDL_GetError());
	}
	else
	{
		log_message("Loaded GL library");
	}
	return success;
}

int gl_load_extentions(void)
{
	int success = 1;
#ifdef USE_GLAD
	if(!gladLoadGLLoader(&SDL_GL_GetProcAddress)) success = 0;
#else
	
#define GLE(ret, name, ...)												\
	gl##name = (name##proc *) SDL_GL_GetProcAddress("gl" #name);		\
	if (!gl##name) {													\
		log_error("gl_load_extentions:SDL_GL_GetProcAddress", "Function gl" #name " failed to load"); \
		success = 0;													\
		return success;													\
	}
	SYMMETRY_GL_LIST
#undef GLE
#endif
	return success;
}

void gl_cleanup(void)
{
	SDL_GL_UnloadLibrary();
}
