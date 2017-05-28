#include "light.h"
#include "array.h"
#include "entity.h"

#include <assert.h>

static int* light_list = NULL;

void light_init(void)
{
	light_list = array_new(int);
}

void light_cleanup(void)
{
	array_free(light_list);
}


void light_create(struct Light* light, int entity_id, int light_type)
{
	assert(light && entity_id > -1);
	
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
	int* new_index = array_grow(light_list, int);
	*new_index = entity_id;
}

void light_destroy(struct Light* light, int entity_id)
{
	assert(light && entity_id > -1);
	
	int index_to_remove = -1;
	for(int i = 0; i < array_len(light_list); i++)
	{
		if(light_list[i] == entity_id)
		{
			index_to_remove = i;
			break;
		}
	}
	if(index_to_remove != -1) array_remove_at(light_list, index_to_remove);
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

int* light_get_valid_indices(int* out_count)
{
	*out_count = array_len(light_list);
	return light_list;
}
