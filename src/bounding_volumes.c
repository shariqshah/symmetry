#include "bounding_volumes.h"
#include "entity.h"
#include "transform.h"

#include <math.h>

int bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box, struct Transform* transform)
{
	vec3 min, max, size, center, half_ext, half_size;
	vec3_fill(&min, 0.f, 0.f, 0.f);
	vec3_fill(&max, 0.f, 0.f, 0.f);
	vec3_fill(&size, 0.f, 0.f, 0.f);
	vec3_fill(&center, 0.f, 0.f, 0.f);
	vec3_fill(&half_ext, 0.f, 0.f, 0.f);
	vec3_fill(&half_size, 0.f, 0.f, 0.f);
	
	vec3_add(&min, &box->min, &transform->position);
	vec3_mul(&min, &min, &transform->scale);
	vec3_add(&max, &box->max, &transform->position);
	vec3_mul(&min, &min, &transform->scale);
	vec3_sub(&size, &max, &min);
	vec3_add(&center, &max, &min);
	vec3_scale(&center, &center, 0.5f);
	vec3_assign(&half_ext, &size);
	vec3_scale(&half_size, &size, 0.5f);
	for(int i = 0; i < 6; i++)
	{
		vec3  normal     = {frustum[i].x, frustum[i].y, frustum[i].z};
		float distance   = frustum[i].w;
		float d          = vec3_dot(&normal, &center);
		vec3  abs_normal = {fabsf(normal.x), fabsf(normal.y), fabsf(normal.z)};
		float r          = vec3_dot(&half_ext, &abs_normal);
		if(d + r < -distance)
			return IT_OUTSIDE;
	}
	return IT_INSIDE;
}

int bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, struct Transform* transform)
{
	int   intersect_type = IT_INSIDE;
	vec3  center, abs_pos, abs_scale;
	float radius = sphere->radius;
	vec3_fill(&center, 0.f, 0.f, 0.f);
	vec3_fill(&abs_pos, 0.f, 0.f, 0.f);
	vec3_fill(&abs_scale, 0.f, 0.f, 0.f);
	
	transform_get_absolute_pos(transform, &abs_pos);
	transform_get_absolute_scale(transform, &abs_scale);
	float max_scale_dimension = fabsf(abs_scale.x);
	if(fabsf(abs_scale.y) > max_scale_dimension) max_scale_dimension = fabsf(abs_scale.y);
	if(fabsf(abs_scale.z) > max_scale_dimension) max_scale_dimension = fabsf(abs_scale.z);
	radius *= max_scale_dimension;
	vec3_add(&center, &sphere->center, &abs_pos);
	//vec3_mul(&center, &center, &transform->scale);
	
	for(int i = 0; i < 6; i++)
	{
		vec3  plane_normal = {frustum[i].x, frustum[i].y, frustum[i].z};
		float distance     = frustum[i].w;
		float dot          = vec3_dot(&plane_normal, &center) + distance;
		if(dot < -radius)
		{
			intersect_type = IT_OUTSIDE;
			return intersect_type;
		}

		if(fabsf(dot) < radius)
		{
			intersect_type = IT_INTERSECT;
			return intersect_type;
		}
	}
	return intersect_type;
}

int bv_intersect_frustum_point(vec4* frustum, const vec3* point)
{
	int success = 1;
	for(int i = 0; i < 6; i++)
	{
		if((frustum[i].x * point->x +
			frustum[i].y * point->y +
			frustum[i].z * point->z +
			frustum[i].w) < 0 )
			success = 0;
	}
	return success;
}
