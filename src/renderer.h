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
	vec3 ambient_light;
};

struct Render_Settings* renderer_get_settings(void);
void renderer_init(void);
void renderer_draw(void);
void renderer_cleanup(void);
void renderer_set_clearcolor(float r, float g, float b, float a);
int  renderer_check_glerror(const char* context);

#endif
