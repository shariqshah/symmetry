#ifndef DEBUG_VARS_H
#define DEBUG_VARS_H

#include "../common/variant.h"
#include "../common/limits.h"

struct Debug_Variable
{
	char name[MAX_DEBUG_VAR_NAME];
	struct Variant value;
};

enum Debug_Variables_Location
{
	DVL_TOP_LEFT = 0,
	DVL_TOP_RIGHT,
	DVL_BOTTOM_LEFT,
	DVL_BOTTOM_RIGHT,
	DVL_FREE,
	DVL_MAX
};

struct Debug_Vars
{
	bool visible;
	int  location;
	int  window_width;
	int  window_height;
	int  row_height;
	int  row_height_color;
	int  row_height_texture;
	struct Debug_Variable numeric_vars[MAX_DEBUG_VARS_PER_FRAME_NUMERIC];
	struct Debug_Variable texture_vars[MAX_DEBUG_VARS_PER_FRAME_NUMERIC];
};

void debug_vars_init(struct Debug_Vars* debug_vars);
void debug_vars_cleanup(struct Debug_Vars* debug_vars);
void debug_vars_location_set(struct Debug_Vars* debug_vars, int location);
void debug_vars_post_update(struct Debug_Vars* debug_vars);
void debug_vars_cycle_location(struct Debug_Vars* debug_vars);

void debug_vars_show(const char* name, const struct Variant* value, bool is_numeric);
void debug_vars_show_int(const char* name, int value);
void debug_vars_show_bool(const char* name, bool value);
void debug_vars_show_float(const char* name, float value);
void debug_vars_show_texture(const char* name, int texture_index);
void debug_vars_show_vec3(const char* name, const vec3* value);
void debug_vars_show_vec2(const char* name, const vec3* value);
void debug_vars_show_vec4(const char* name, const vec3* value);
void debug_vars_show_color_rgb(const char* name, const vec3* value);
void debug_vars_show_color_rgba(const char* name, const vec4* value);


#endif