#include "bounding_volumes.h"
#include "transform.h"

#include <math.h>

int bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box, struct Transform* transform)
{
	vec3 min, max, size, center, half_ext, half_size;
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
		vec3 normal = {frustum[i].x, frustum[i].y, frustum[i].z};
		float distance = frustum[i].w;
		float d = vec3_dot(&normal, &center);
		vec3 abs_normal = {fabsf(normal.x), fabsf(normal.y), fabsf(normal.z)};
		float r = vec3_dot(&half_ext, &abs_normal);
		if(d + r < -distance)
		{
			return IT_OUTSIDE;
		}
	}
	return IT_INSIDE;
}

int bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, struct Transform* transform)
{
	int intersect_type = IT_INSIDE;
	vec3 center;
	vec3_add(&center, &sphere->center, &transform->position);
	vec3_mul(&center, &center, &transform->scale);
	
	for(int i = 0; i < 6; i++)
	{
		vec3 plane_normal = {frustum[i].x, frustum[i].y, frustum[i].z};
		float distance = frustum[i].w;
		float dot = vec3_dot(&plane_normal, &center) + distance;
		if(dot < -sphere->radius)
		{
			intersect_type = IT_OUTSIDE;
			return intersect_type;
		}

		if(fabsf(dot) < sphere->radius)
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
