#include "light.h"
#include "../common/array.h"
#include "entity.h"

#include <assert.h>

void light_init(struct Light* light, int light_type)
{
	light->valid       = true;
	light->cast_shadow = 0;
	light->depth_bias  = 0.0005f;
	light->type        = light_type;
	light->pcf_enabled = false;
	light->intensity   = 1.f;
	light->falloff     = 1.5f;
	light->outer_angle = TO_RADIANS(30.f);
	light->inner_angle = TO_RADIANS(20.f);
	light->radius      = 20;
	vec3_fill(&light->color, 1.f, 1.f, 1.f);
}

void light_reset(struct Light* light)
{
	light->valid       = false;
	light->cast_shadow = 0;
	light->depth_bias  = 0.f;
	light->type        = LT_INVALID;
	light->pcf_enabled = false;
	light->intensity   = 10.f;
	light->falloff     = 0.f;
	light->outer_angle = 0.f;
	light->inner_angle = 0.f;
	light->radius      = 0.f;
	vec3_fill(&light->color, 1.f, 0.f, 1.f);
}