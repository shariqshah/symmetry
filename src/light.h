#ifndef LIGHT_H
#define LIGHT_H

#include "num_types.h"
#include "linmath.h"

#define MAX_SHADOWMAPS 4

enum LightType
{
	LT_SPOT  = 0,
	LT_DIR,
	LT_POINT
};

struct Light
{
	float outer_angle;
	float inner_angle;
	float falloff;
	float intensity;
	vec4 color;
	int32 node;
	uint8 cast_shadow;
	uint8 pcf_enabled;
	uint8 valid;        
	int   type;
	int   radius; 
	int   shadow_map[4];
	float depth_bias;
};

struct Light* light_get(int index);
struct Light* light_get_all(void);
void light_init(void);
void light_cleanup(void);
void light_remove(int index);
int light_create(int node, int light_type);
void light_set_radius(struct Light* light, int radius);

#endif
