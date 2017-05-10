#include "light.h"
#include "log.h"
#include "array.h"
#include <stdio.h>
#include <string.h>

static struct Light* light_list;
static int* empty_indices;
static int* valid_light_indices;
static int  max_lights;

struct Light* light_get(int index)
{
	struct Light* light = NULL;
	if(index > -1 && index < array_len(light_list))
		light = &light_list[index];
	return light;
}

struct Light* light_get_all(void)
{
	return light_list;
}

void light_init(void)
{
	max_lights = 128;
	light_list = array_new_cap(struct Light, max_lights);
	for(int i = 0; i < max_lights; i++) light_list[i].valid = 0;
	empty_indices = array_new(int);
	valid_light_indices = array_new_cap(int, max_lights);
	memset(valid_light_indices, -1, max_lights);
}

void light_cleanup(void)
{
	for(int i = 0; i < array_len(light_list); i++)
		light_remove(i);
	array_free(light_list);
	array_free(empty_indices);
	array_free(valid_light_indices);
}

void light_remove(int index)
{
	if(index > -1 && index < array_len(light_list))
	{
		light_list[index].valid = 0;
		array_push(empty_indices, index, int);
	}
}

int light_create(int node, int light_type)
{
	int index = -1;
	struct Light* new_light = NULL;
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		new_light = &light_list[index];
		array_pop(empty_indices);
	}
	else
	{
		for(index = 0; index < max_lights; index++)
		{
			if(!light_list[index].valid)
				break;
		}
		
		if(index == max_lights - 1)
		{
			index = -1;
			log_warning("Max light limit(%d) reached, cannot add light", max_lights);
			return index;
		}

		new_light = &light_list[index];
	}
	new_light->node = node;
	new_light->valid = 1;
	new_light->cast_shadow = 0;
	vec3_fill(&new_light->color, 1.f, 1.f, 1.f);
	new_light->depth_bias = 0.0005f;
	new_light->type = light_type;
	new_light->pcf_enabled = 0;
	new_light->intensity = 1.f;
	new_light->falloff = 1.5f;
	new_light->outer_angle = TO_RADIANS(30.f);
	new_light->inner_angle = TO_RADIANS(20.f);
	new_light->radius = 20;
	return index;
}

int light_get_max_lights(void)
{
	return max_lights;
}

int* light_get_valid_indices(int* valid_light_count)
{
	/* First, get all the valid(active) lights, then sort them in the
	   order directional, point, spot
	 */
	int light_count = 0;
	for(int i = 0; i < array_len(light_list); i++)
	{
		if(light_list[i].valid)
		{
			valid_light_indices[light_count] = i;
			light_count++;
		}
	}
	*valid_light_count = light_count;
	
	return valid_light_indices;
}
