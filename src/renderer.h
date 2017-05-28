#ifndef renderer_H
#define renderer_H

#include "linmath.h"

enum Fog_Mode
{
	FM_NONE             = 0,
	FM_LINEAR           = 1,
	FM_EXPONENTIAL      = 2,
	FM_EXPONENTIAL_SQRD = 3
};

struct Fog
{
	int mode;
	float density;
	float start_dist;
	float max_dist;
	vec3 color;
};


struct Render_Settings
{
	struct Fog fog;
	vec3       ambient_light;
	int        max_gui_vertex_memory;
	int        max_gui_element_memory;
	int        debug_draw_enabled;
	vec4       debug_draw_color;
	int        debug_draw_mode;
};

struct Entity;

struct Render_Settings* renderer_settings_get(void);
void 					renderer_init(void);
void 					renderer_draw(struct Entity* active_viewer);
void 					renderer_cleanup(void);
void 					renderer_clearcolor_set(float r, float g, float b, float a);
void 					renderer_debug_draw_enabled(int enabled);
int  					renderer_check_glerror(const char* context);

#endif
