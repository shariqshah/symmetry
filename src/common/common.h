#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#include "num_types.h"

#ifdef GAME_LIB
extern struct Platform_Api* platform;
#endif

#ifdef GAME
extern struct Game_Api game;
#endif

struct Window;
struct Hashmap;

// Function Pointer decls
typedef void (*Keyboard_Event_Func)     (int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
typedef void (*Mousebutton_Event_Func)  (int button, int state, int x, int y, int8 num_clicks);
typedef void (*Mousemotion_Event_Func)  (int x, int y, int xrel, int yrel);
typedef void (*Mousewheel_Event_Func)   (int x, int y);
typedef void (*Windowresize_Event_Func) (int x, int y);
typedef void (*Textinput_Event_Func)    (const char* text);

enum Directory_Type
{
    DIRT_USER,					/* User directory or preferences directory */
    DIRT_INSTALL,					/* Directory where the game's assets are, usually alongside the game's executable where the game is installed */
	DIRT_EXECUTABLE,				/* Directory where the game's executable is located */
	DIRT_COUNT
};

struct Sound_Api
{
    void (*volume_set)(float volume);
    void (*listener_update)(float apos_x, float apos_y, float apos_z,
                            float afwd_x, float afwd_y, float afwd_z,
                            float aup_x,  float aup_y,  float aup_z);
    void (*source_update)(uint source_handle,
                          float apos_x, float apos_y, float apos_z,
                          float afwd_x, float afwd_y, float afwd_z,
                          float aup_x,  float aup_y,  float aup_z);
    void (*source_create)(bool relative, uint num_buffers, uint* out_handle, uint* out_buffer_handles);
    void (*source_destroy)(uint source_handle, uint* attached_buffers, uint num_buffers);
    void (*source_volume_set)(uint source_handle, float volume);
    void (*source_pitch_set)(uint source_handle, float pitch);
    void (*source_load_wav)(uint source_handle, uint buffer_handle, const char* file_name);
    void (*source_loop_set)(uint source_handle, bool loop);
    void (*source_relative_set)(uint source_handle, bool relative);
    void (*source_play)(uint source_handle);
    void (*source_pause)(uint source_handle);
    void (*source_rewind)(uint source_handle);
    void (*source_stop)(uint source_handle);
};

struct Window_Api
{
    struct Window* (*create)(const char* title, int width, int height, int msaa, int msaa_levels);
    void           (*destroy)(struct Window* window);
    void           (*show)(struct Window* window);
    void           (*hide)(struct Window* window);
    void           (*raise)(struct Window* window);
    void           (*make_context_current)(struct Window* window);
    void           (*set_size)(struct Window* window, int width, int height);
    void           (*get_size)(struct Window* window, int* out_width, int* out_height);
    void           (*get_drawable_size)(struct Window* window, int* out_width, int* out_height);
    void           (*swap_buffers)(struct Window* window);
    int            (*fullscreen_set)(struct Window* window, int fullscreen);
};

struct File_Api
{
    char* (*read)(const int directory_type, const char* path, const char* mode, long* file_size);
    FILE* (*open)(const int directory_type, const char* path, const char* mode);
	bool  (*copy)(const int directory_type, const char* source, const char* destination);
	bool  (*delete)(const int directory_type, const char* filename);
};

struct Config_Api
{
    bool            (*load)(const char* filename, int directory_type);
    bool            (*save)(const char* filename, int directory_types);
    struct Hashmap* (*get)(void);
};

struct Log_Api
{
    FILE* (*file_handle_get)(void);
};

struct Platform_Api
{
    // General platform api
    void        (*poll_events)(bool* out_quit);
    void        (*keyboard_callback_set)(Keyboard_Event_Func func);
    void        (*mousebutton_callback_set)(Mousebutton_Event_Func func);
    void        (*mousemotion_callback_set)(Mousemotion_Event_Func func);
    void        (*mousewheel_callback_set)(Mousewheel_Event_Func func);
    void        (*windowresize_callback_set)(Windowresize_Event_Func func);
    void        (*textinput_callback_set)(Textinput_Event_Func func);
    int         (*is_key_pressed)(int key);
    int         (*mousebutton_state_get)(uint button);
    void        (*mouse_position_get)(int* x, int* y);
    void        (*mouse_delta_get)(int* x, int* y);
    void        (*mouse_position_set)(struct Window* window, int x, int y);
    void        (*mouse_global_position_set)(int x, int y);
    void        (*mouse_relative_mode_set)(int relative_mode);
    int         (*mouse_relative_mode_get)(void);
    uint32      (*ticks_get)(void);
    char*       (*install_directory_get)(void);
    char*       (*user_directory_get)(const char* organization, const char* application);
    void        (*clipboard_text_set)(const char* text);
    char*       (*clipboard_text_get)(void);
    int         (*key_from_name)(const char* key_name);
    const char* (*key_name_get)(int key);
    void*       (*load_function_gl)(const char* name);
    void        (*reload_game_lib)(void);

    struct Window_Api window;
    struct Sound_Api  sound;
    struct File_Api   file;
    struct Config_Api config;
    struct Log_Api    log;
};

struct Game_Api
{
    bool (*init)(struct Window*, struct Platform_Api* platform_api);
    void (*cleanup)(void);
};

#endif
