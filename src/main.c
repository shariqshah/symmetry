#define GLEW_STATIC
#include <GL/glew.h>
#include <stdio.h>

#include "log.h"
#include "window_system.h"
#include "game.h"


const int WIN_WIDTH  = 800;
const int WIN_HEIGHT = 600;

int  init();
void cleanup();

int main(int argc, char** args)
{
    //Initialize window system and Glew
    if(!init())
		log_error("Main:main", "Could not initialize");
    else
		game_init();
	
    cleanup();
	log_message("Program exiting!");
    return 0;
}

int init(void)
{
	bool success = true;
	if(window_init("Symmetry", WIN_WIDTH, WIN_HEIGHT))
	{	
		//Initialize GLEW
		glewExperimental = GL_TRUE;
		GLenum glewError = glewInit();
		if(glewError != GLEW_OK)
		{
			log_error("Main:init", "GLEW : %s", glewGetErrorString(glewError));
			success = false;
		}
	}
	else
	{
		success = false;
	}
	return success;
}

void cleanup()
{
	game_cleanup();
	window_cleanup();
}
