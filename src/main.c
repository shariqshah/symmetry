#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "platform.h"
#include "gl_load.h"
#include "game.h"


const int WIN_WIDTH  = 1024;
const int WIN_HEIGHT = 768;
static struct Window* window = NULL;

int  init();
void cleanup();

int main(int argc, char** args)
{
    if(!init())
		log_error("Main:main", "Could not initialize");
    else
		game_init(window);
	
    exit(EXIT_SUCCESS);
}

int init(void)
{
	int success = 1;
	if(atexit(cleanup) != 0)
	{
		success = 0;
		log_error("main:init", "Could not register cleanup func with atexit");
	}
	else
	{
        if(platform_init())
		{	
            success = gl_load_library();
            if(!success)
			{
                log_error("main:init", "Initializing OpenGL failed");
			}
            else
            {
                window = window_create("Symmetry", WIN_WIDTH, WIN_HEIGHT);
                if(!window)
                {
                    log_error("main:init", "Window creation failed");
                    success = 0;
                }
				else
				{
					success = gl_load_extentions();
					if(!success) log_error("main:init", "Failed to load opengl extentions");
				}
            }
		}
		else
		{
			success = 0;
		}
	}
	return success;
}

void cleanup()
{
	game_cleanup();
    platform_cleanup();
	log_message("Program exiting!");
}
