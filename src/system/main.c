#include <stdio.h>
#include <stdlib.h>

#include "../common/log.h"
#include "sound.h"
#include "platform.h"
#include "physics.h"
#include "file_io.h"
#include "config_vars.h"
#include "../common/hashmap.h"
#include "../game/game.h"

struct Wndow;

static struct Window*  window = NULL;
static struct Hashmap* cvars  = NULL;

bool init(void);
void cleanup(void);

int main(int argc, char** args)
{
    if(!init())
    {
		log_to_stdout("ERR:(Main) Could not initialize");
    }
    else
    {
		bool done = false;
		while(!done)
		{
			bool game_init_status = game_init(window, cvars);
			if(!game_init_status)
			{
				log_error("main", "Game init failed");
			}
			else
			{
				done = game_run();
			}
		}
    }
	
    exit(EXIT_SUCCESS);
}

bool init(void)
{
	if(atexit(cleanup) != 0)
	{
		log_to_stdout("ERR: (main:init) Could not register cleanup func with atexit");
        return false;
	}

	cvars = hashmap_new();
    config_vars_init(cvars);
    if(!platform_init()) return false;

    char* install_path = platform_install_directory_get();
    char* user_path    = platform_user_directory_get("SS_Games", "Symmetry");
    log_init("Log.txt", user_path);
    io_file_init(install_path, user_path);
    free(install_path);
    free(user_path);
    if(!config_vars_load(cvars, "config.symtres", DIRT_USER))
    {
        log_error("main:init", "Could not load config, reverting to defaults");
        config_vars_save(cvars, "config.symtres", DIRT_USER);
    }

    if(!platform_init_video()) return false;

    if(!platform_load_gl(NULL))
    {
        log_error("main:init", "Initializing OpenGL failed");
        return false;
    }

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

    if(!sound_init())
    {
        log_error("main:init", "Failed to initialize sound");
        return false;
    }

    return true;
}

void cleanup(void)
{
	game_cleanup();
    if(window) window_destroy(window);
    log_reset_all_callbacks(); // Now that the game library has been unloaded, reset all callbacks to stubs so we don't crash on exit
    sound_cleanup();
    platform_unload_gl();
    platform_cleanup();
	config_vars_cleanup(cvars);
    io_file_cleanup();
	log_message("Program exiting!");
	log_cleanup();
}