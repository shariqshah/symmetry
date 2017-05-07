#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "platform.h"
#include "gl_load.h"
#include "game.h"
#include "file_io.h"
#include "config_vars.h"
#include "hashmap.h"

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
		config_vars_init();
        if(platform_init())
		{
            success = gl_load_library();
            if(!success)
			{
                log_error("main:init", "Initializing OpenGL failed");
			}
            else
            {
				char* base_path = platform_base_path_get();
				io_file_init(base_path);
				free(base_path);
				if(!config_vars_load("config.cfg"))
					log_error("main:init", "Could not load config, reverting to defaults");
				
				struct Hashmap* cvars = config_vars_get();
				int width       = hashmap_int_get(cvars,  "render_width");
				int height      = hashmap_int_get(cvars,  "render_height");
				int msaa        = hashmap_bool_get(cvars, "msaa_enabled");
				int msaa_levels = hashmap_int_get(cvars,  "msaa_levels");
                window = window_create("Symmetry", width, height, msaa, msaa_levels);
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
	config_vars_cleanup();
	log_message("Program exiting!");
}
