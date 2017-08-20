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

bool init();
void cleanup();

int main(int argc, char** args)
{
    if(!init())
		log_to_stdout("ERR:(Main) Could not initialize");
    else
		game_init(window);
	
    exit(EXIT_SUCCESS);
}

bool init(void)
{
	if(atexit(cleanup) != 0)
	{
		log_to_stdout("ERR: (main:init) Could not register cleanup func with atexit");
        return false;
	}

    config_vars_init();
    if(!platform_init()) return false;

    log_init("Log.txt");

    char* install_path = platform_install_directory_get();
    char* user_path    = platform_user_directory_get("SS_Games", "Symmetry");
    io_file_init(install_path, user_path);
    free(install_path);
    free(user_path);
    if(!config_vars_load("config.cfg", DT_USER))
    {
        log_error("main:init", "Could not load config, reverting to defaults");
        config_vars_save("config.cfg", DT_USER);
    }

    if(!platform_init_video()) return false;

    if(!gl_load_library())
    {
        log_error("main:init", "Initializing OpenGL failed");
        return false;
    }

    struct Hashmap* cvars = config_vars_get();
    int width       = hashmap_int_get(cvars,  "render_width");
    int height      = hashmap_int_get(cvars,  "render_height");
    int msaa        = hashmap_bool_get(cvars, "msaa_enabled");
    int msaa_levels = hashmap_int_get(cvars,  "msaa_levels");
    window = window_create("Symmetry", width, height, msaa, msaa_levels);
    if(!window)
    {
        log_error("main:init", "Window creation failed");
        return false;
    }

    if(!gl_load_extentions())
    {
        log_error("main:init", "Failed to load opengl extentions");
        return false;
    }

    return true;
}

void cleanup()
{
	game_cleanup();
    platform_cleanup();
	config_vars_cleanup();
	log_message("Program exiting!");
	log_cleanup();
}
