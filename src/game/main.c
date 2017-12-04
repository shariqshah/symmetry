#include <stdio.h>
#include <stdlib.h>

#include "../common/log.h"
#include "sound.h"
#include "platform.h"
#include "physics.h"
#include "file_io.h"
#include "config_vars.h"
#include "../common/hashmap.h"
#include "../common/common.h"

static struct Platform_Api platform_api;
static struct Window*      window        = NULL;
static bool                reload_game   = false;
#if defined(_MSC_VER)
static const char*         lib_name      = "libSymmetry.dll";
static const char*         lib_copy_name = "libSymmetry.copy.dll";
#elif defined(__MINGW32__) || defined(__MINGW64__)
static const char*         lib_name      = "libSymmetry.dll";
static const char*         lib_copy_name = "libSymmetry.copy.dll";
#endif

void*           game_lib_handle = NULL;
struct Game_Api game;

bool init(void);
void cleanup(void);
bool game_lib_load(void);
void game_lib_reload(void);

int main(int argc, char** args)
{
    if(!init())
    {
		log_to_stdout("ERR:(Main) Could not initialize");
    }
    else
    {
        platform_api = (struct Platform_Api)
        {
            .poll_events               = &platform_poll_events,
            .keyboard_callback_set     = &platform_keyboard_callback_set,
            .mousebutton_callback_set  = &platform_mousebutton_callback_set,
            .mousemotion_callback_set  = &platform_mousemotion_callback_set,
            .mousewheel_callback_set   = &platform_mousewheel_callback_set,
            .windowresize_callback_set = &platform_windowresize_callback_set,
            .textinput_callback_set    = &platform_textinput_callback_set,
            .is_key_pressed            = &platform_is_key_pressed,
            .mousebutton_state_get     = &platform_mousebutton_state_get,
            .mouse_position_get        = &platform_mouse_position_get,
            .mouse_delta_get           = &platform_mouse_delta_get,
            .mouse_position_set        = &platform_mouse_position_set,
            .mouse_global_position_set = &platform_mouse_global_position_set,
            .mouse_relative_mode_set   = &platform_mouse_relative_mode_set,
            .mouse_relative_mode_get   = &platform_mouse_relative_mode_get,
            .ticks_get                 = &platform_ticks_get,
            .install_directory_get     = &platform_install_directory_get,
            .user_directory_get        = &platform_user_directory_get,
            .clipboard_text_set        = &platform_clipboard_text_set,
            .clipboard_text_get        = &platform_clipboard_text_get,
            .key_from_name             = &platform_key_from_name,
            .key_name_get              = &platform_key_name_get,
            .load_function_gl          = &platform_load_function_gl,
            .reload_game_lib           = &game_lib_reload,
            .sound =
            {
				.update_3d                            = &sound_update_3d,
                .volume_set                           = &sound_volume_set,
                .listener_update                      = &sound_listener_update,
                .source_instance_update_position      = &sound_source_instance_update_position,
                .source_instance_create               = &sound_source_instance_create,
                .source_instance_destroy              = &sound_source_instance_destroy,
                .source_instance_volume_set           = &sound_source_instance_volume_set,
                .source_instance_loop_set             = &sound_source_instance_loop_set,
                .source_instance_play                 = &sound_source_instance_play,
                .source_instance_pause                = &sound_source_instance_pause,
                .source_instance_rewind               = &sound_source_instance_rewind,
                .source_instance_stop                 = &sound_source_instance_stop,
				.source_instance_min_max_distance_set = &sound_source_instance_min_max_distance_set,
				.source_instance_attenuation_set      = &sound_source_instance_attenuation_set,
				.source_instance_volume_get           = &sound_source_instance_volume_get,
				.source_instance_loop_get			  = &sound_source_instance_loop_get,
				.source_instance_is_paused			  = &sound_source_instance_is_paused,
				.source_create                        = &sound_source_create,
				.source_get					          = &sound_source_get,
				.source_destroy				          = &sound_source_destroy,
				.source_volume_set			          = &sound_source_volume_set,
				.source_loop_set				      = &sound_source_loop_set,
				.source_stop_all				      = &sound_source_stop_all,
				.source_min_max_distance_set	      = &sound_source_min_max_distance_set
            },
            .window =
            {
                .create               = &window_create,
                .destroy              = &window_destroy,
                .show                 = &window_show,
                .raise                = &window_raise,
                .make_context_current = &window_make_context_current,
                .set_size             = &window_set_size,
                .get_size             = &window_get_size,
                .get_drawable_size    = &window_get_drawable_size,
                .swap_buffers         = &window_swap_buffers,
                .fullscreen_set       = &window_fullscreen_set
            },
            .file =
            {
                .read  	= &io_file_read,
                .open  	= &io_file_open,
				.copy  	= &io_file_copy,
				.delete = &io_file_delete
            },
            .config =
            {
                .load = &config_vars_load,
                .save = &config_vars_save,
                .get  = &config_vars_get
            },
            .log =
            {
                .file_handle_get = &log_file_handle_get
            },
			.physics = 
			{
				.init                        = &physics_init,
				.cleanup                     = &physics_cleanup,
				.step                        = &physics_step,
				.gravity_set                 = &physics_gravity_set,
				.gravity_get                 = &physics_gravity_get,
				.body_position_set           = &physics_body_position_set,
				.body_position_get           = &physics_body_position_get,
				.body_rotation_set           = &physics_body_rotation_set,
				.body_rotation_get           = &physics_body_rotation_get,
				.body_kinematic_set          = &physics_body_kinematic_set,
				.body_mass_set               = &physics_body_mass_set,
				.body_mass_get               = &physics_body_mass_get,
				.body_data_set               = &physics_body_data_set,
				.body_data_get               = &physics_body_data_get,
				.body_set_moved_callback     = &physics_body_set_moved_callback,
				.body_set_collision_callback = &physics_body_set_collision_callback,
				.plane_create                = &physics_plane_create,
				.box_create                  = &physics_box_create
			}
        };

        if(!game_lib_load())
            log_error("main", "Failed to load  game library");
        else
		{
			bool done = false;
			while(!done)
			{
				bool game_init_status = game.init(window, &platform_api);
				if(!game_init_status)
				{
					log_error("main", "Game init failed");
				}

				if(reload_game)
				{
					reload_game = false;
					if(game_lib_handle)
					{
						if(game.cleanup) game.cleanup();
						platform_unload_library(game_lib_handle);
						game_lib_handle = NULL;
						game.cleanup    = NULL;
						game.init       = NULL;
#if defined(_MSC_VER)
						if(!io_file_delete(DIRT_EXECUTABLE, lib_copy_name))
						{
							done = true;
							continue;
						}
#endif
					}
					
					if(!game_lib_load())
					{
						log_error("main", "Failed to load  game library");
						done = true;
					}
				}
				else
				{
					done = true;
				}
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

    config_vars_init();
    if(!platform_init()) return false;

    char* install_path = platform_install_directory_get();
    char* user_path    = platform_user_directory_get("SS_Games", "Symmetry");
    log_init("Log.txt", user_path);
    io_file_init(install_path, user_path);
    free(install_path);
    free(user_path);
    if(!config_vars_load("config.symtres", DIRT_USER))
    {
        log_error("main:init", "Could not load config, reverting to defaults");
        config_vars_save("config.symtres", DIRT_USER);
    }

    if(!platform_init_video()) return false;

    if(!platform_load_gl(NULL))
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

    if(!sound_init())
    {
        log_error("main:init", "Failed to initialize sound");
        return false;
    }

    return true;
}

void cleanup(void)
{
	if(game.cleanup) game.cleanup();
	if(game_lib_handle) platform_unload_library(game_lib_handle);
    if(window) window_destroy(window);
    sound_cleanup();
    platform_unload_gl();
    platform_cleanup();
	config_vars_cleanup();
    io_file_cleanup();
	log_message("Program exiting!");
	log_cleanup();
}

void game_lib_reload(void)
{
	reload_game = true;
}

bool game_lib_load(void)
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__MINGW64__)
	if(!io_file_copy(DIRT_EXECUTABLE, lib_name, lib_copy_name))
	{
		log_error("main:game_lib_load", "Failed to copy dll");
		return false;
	}
	
    game_lib_handle = platform_load_library("libSymmetry.copy");
#else
	game_lib_handle = platform_load_library("Symmetry");
#endif
    if(!game_lib_handle)
    {
        log_error("main:game_lib_load", "Failed to load game library");
        return false;
    }

    log_message("Game library loaded");
    game.init    = platform_load_function(game_lib_handle, "game_init");
    game.cleanup = platform_load_function(game_lib_handle, "game_cleanup");
    if(!game.init && !game.cleanup)
        return false;

    log_message("Game api loaded");
    return true;
}
