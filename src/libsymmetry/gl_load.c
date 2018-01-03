#include "gl_load.h"
#include "../common/log.h"
#include "../common/common.h"


#ifndef USE_GLAD

#define GLE(ret, name, ...) name##proc * gl##name;
SYMMETRY_GL_LIST
#undef GLE

#endif

int gl_load_library(void)
{
	int success = 1;
//	if(SDL_GL_LoadLibrary(NULL) < 0)
//	{
//		success = 0;
//		log_error("gl_load_library", "Failed to load GL library %s", SDL_GetError());
//	}
//	else
//	{
//		log_message("Loaded GL library");
//	}
	return success;
}

bool gl_load_extentions(void)
{
    bool success = true;
#ifdef USE_GLAD
    if(!gladLoadGLLoader(platform->load_function_gl)) success = false;
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

void gl_check_error(const char * expression, unsigned int line, const char * file)
{
	int error = 1;
	GLenum error_code = glGetError();
	const char* error_string = "No Error";
	switch(error_code)
	{
	case GL_INVALID_OPERATION: 			   error_string = "Invalid Operation"; 		       break;
	case GL_NO_ERROR:		   			   error_string = "No Error";		  		       break;
	case GL_INVALID_ENUM:	   			   error_string = "Invalid ENUM";	  		       break;
	case GL_INVALID_VALUE:	   			   error_string = "Invalid Value";	  		       break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: error_string = "Invalid FrameBuffer Operation"; break;
	case GL_OUT_OF_MEMORY:		           error_string = "Out of Memory";		           break;
	}

	if(error_code != GL_NO_ERROR)
		log_error("GL", "(%s:%d:%s) : %s", file, line, expression, error_string);
	else
		error = 0;

	return error;
}
