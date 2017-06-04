#ifndef RENDERER_H
#define RENDERER_H

#include "linmath.h"
#include "num_types.h"

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
	int        debug_draw_enabled;
	vec4       debug_draw_color;
	int        debug_draw_mode;
};

struct Entity;

void renderer_settings_get(struct Render_Settings* settings);
void renderer_settings_set(const struct Render_Settings* settings);
void renderer_init(void);
void renderer_draw(struct Entity* active_viewer);
void renderer_cleanup(void);
void renderer_clearcolor_set(float r, float g, float b, float a);
void renderer_debug_draw_enabled(bool enabled);
int  renderer_check_glerror(const char* context);

#endif
