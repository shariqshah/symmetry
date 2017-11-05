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
struct Sound_Source_Buffer;

//Physics Decls
typedef void* Rigidbody;
typedef void (*RigidbodyMoveCB)(Rigidbody);

// Function Pointer decls
typedef void (*Keyboard_Event_Func)     (int key, int scancode, int state, int repeat, int mod_ctrl, int mod_shift, int mod_alt);
typedef void (*Mousebutton_Event_Func)  (int button, int state, int x, int y, int8 num_clicks);
typedef void (*Mousemotion_Event_Func)  (int x, int y, int xrel, int yrel);
typedef void (*Mousewheel_Event_Func)   (int x, int y);
typedef void (*Windowresize_Event_Func) (int x, int y);
typedef void (*Textinput_Event_Func)    (const char* text);

enum Directory_Type
{
    DIRT_USER,		 /* User directory or preferences directory */
    DIRT_INSTALL,	 /* Directory where the game's assets are, usually alongside the game's executable where the game is installed */
	DIRT_EXECUTABLE, /* Directory where the game's executable is located */
	DIRT_COUNT
};

enum Sound_Source_Type
{
	ST_WAV = 0,
	ST_WAV_STREAM
};

enum Sound_Attenuation_Type
{
	SA_NONE = 0,   // No attenuation
	SA_INVERSE,    // Inverse distance attenuation model
	SA_LINEAR,     // Linear distance attenuation model
	SA_EXPONENTIAL // Exponential distance attenuation model
};

struct Physics_Api
{
	void      (*init)(void);
	void      (*cleanup)(void);
	void      (*step)(float);
	void      (*gravity_set)(float x, float y, float z);
	void      (*gravity_get)(float* x, float* y, float* z);
	void      (*body_position_set)(Rigidbody body, float x, float y, float z);
	void      (*body_position_get)(Rigidbody body, float* x, float* y, float* z);
	void      (*body_rotation_set)(Rigidbody body, float x, float y, float z, float w);
	void      (*body_rotation_get)(Rigidbody body, float* x, float* y, float* z, float* w);
	Rigidbody (*plane_create)(float a, float b, float c, float d);
	Rigidbody (*box_create)(float length, float width, float height);
	void      (*body_set_moved_callback)(Rigidbody body, RigidbodyMoveCB callback);
	void      (*body_kinematic_set)(Rigidbody body);
};

struct Sound_Api
{
	void (*update_3d)(void);
    void (*volume_set)(float volume);
    void (*listener_update)(float apos_x, float apos_y, float apos_z,
                            float afwd_x, float afwd_y, float afwd_z,
                            float aup_x,  float aup_y,  float aup_z);

    void  (*source_instance_update_position)(uint source_instance, float apos_x, float apos_y, float apos_z);
	uint  (*source_instance_create)(struct Sound_Source_Buffer* source, bool is3d);
    void  (*source_instance_destroy)(uint source_instance);
    void  (*source_instance_volume_set)(uint source_instance, float volume);
    void  (*source_instance_loop_set)(uint source_instance, bool loop);
    void  (*source_instance_play)(uint source_instance);
    void  (*source_instance_pause)(uint source_instance);
    void  (*source_instance_rewind)(uint source_instance);
    void  (*source_instance_stop)(uint source_instance);
	void  (*source_instance_min_max_distance_set)(uint source_instance, float min_distance, float max_distance);
	void  (*source_instance_attenuation_set)(uint source_instance, int attenuation_type, float rolloff_factor);
	float (*source_instance_volume_get)(uint source_instance);
	bool  (*source_instance_loop_get)(uint source_instance);
	bool  (*source_instance_is_paused)(uint source_instance);

	struct Sound_Source_Buffer* (*source_create)(const char* filename, int type);
	struct Sound_Source_Buffer* (*source_get)(const char* name);
	void                        (*source_destroy)(const char* buffer_name);
	void                        (*source_volume_set)(struct Sound_Source_Buffer* source, float volume);
	void                        (*source_loop_set)(struct Sound_Source_Buffer* source, bool loop);
	void                        (*source_stop_all)(struct Sound_Source_Buffer* source);
	void                        (*source_min_max_distance_set)(struct Sound_Source_Buffer* source, float min_distance, float max_distance);
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

    struct Window_Api  window;
    struct Sound_Api   sound;
    struct File_Api    file;
    struct Config_Api  config;
    struct Log_Api     log;
	struct Physics_Api physics;
};

struct Game_Api
{
    bool (*init)(struct Window*, struct Platform_Api* platform_api);
    void (*cleanup)(void);
};

#endif
