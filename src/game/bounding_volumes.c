#include "bounding_volumes.h"

#include <math.h>

int bv_intersect_frustum_box_(vec4* frustum, struct Bounding_Box* box)
{
	vec3 size, center, half_ext, half_size;
	vec3_fill(&size, 0.f, 0.f, 0.f);
	vec3_fill(&center, 0.f, 0.f, 0.f);
	vec3_fill(&half_ext, 0.f, 0.f, 0.f);
	vec3_fill(&half_size, 0.f, 0.f, 0.f);
	
	vec3_sub(&size, &box->max, &box->min);
	vec3_add(&center, &box->max, &box->min);
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

int bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box, vec3* box_abs_position, vec3* box_abs_scale)
{
	vec3 min, max, size, center, half_ext, half_size;
	vec3_fill(&min, 0.f, 0.f, 0.f);
	vec3_fill(&max, 0.f, 0.f, 0.f);
	vec3_fill(&size, 0.f, 0.f, 0.f);
	vec3_fill(&center, 0.f, 0.f, 0.f);
	vec3_fill(&half_ext, 0.f, 0.f, 0.f);
	vec3_fill(&half_size, 0.f, 0.f, 0.f);
	
	vec3_add(&min, &box->min, box_abs_position);
	vec3_mul(&min, &min, box_abs_scale);
	vec3_add(&max, &box->max, box_abs_position);
	vec3_mul(&min, &min, box_abs_scale);
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

int bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, vec3* sphere_abs_pos, vec3* sphere_abs_scale)
{
	int   intersect_type = IT_INSIDE;
	vec3  center;
	float radius = sphere->radius;
	vec3_fill(&center, 0.f, 0.f, 0.f);
	
	float max_scale_dimension = fabsf(sphere_abs_scale->x);
	if(fabsf(sphere_abs_scale->y) > max_scale_dimension) max_scale_dimension = fabsf(sphere_abs_scale->y);
	if(fabsf(sphere_abs_scale->z) > max_scale_dimension) max_scale_dimension = fabsf(sphere_abs_scale->z);
	radius *= max_scale_dimension;
	vec3_add(&center, &sphere->center, sphere_abs_pos);
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

bool bv_intersect_frustum_point(vec4* frustum, const vec3* point)
{
	bool success = true;
	for(int i = 0; i < 6; i++)
	{
		if((frustum[i].x * point->x +
			frustum[i].y * point->y +
			frustum[i].z * point->z +
			frustum[i].w) < 0 )
			success = false;
	}
	return success;
}

int bv_intersect_sphere_ray(struct Bounding_Sphere* sphere, vec3* sphere_abs_position, vec3* sphere_abs_scale, struct Ray* ray)
{
	int intersection_type = IT_OUTSIDE;
	vec3 center = {0.f, 0.f, 0.f};
	vec3_add(&center, &sphere->center, sphere_abs_position);
	float radius = sphere->radius * sphere_abs_scale->x;
	//float squared_radius = sphere->radius * sphere->radius;
	float squared_radius = radius * radius;

	vec3 centered_origin;
	vec3_sub(&centered_origin, &ray->origin, &center);
	//vec3_sub(&centered_origin, &center, &ray->origin);
	float centered_origin_len_sqrd = vec3_len(&centered_origin);
	centered_origin_len_sqrd *= centered_origin_len_sqrd;

	//Check if ray originates inside the sphere
	if(centered_origin_len_sqrd <= squared_radius)
		return IT_INSIDE;

	// Calculate the intersection by quatratic equation'
	float a = vec3_dot(&ray->direction, &ray->direction);
	float b = 2.f * vec3_dot(&centered_origin, &ray->direction);
	float c = vec3_dot(&centered_origin, &centered_origin) - squared_radius;
	float d = b * b - 4.f * a * c;

	//No solution
	if(d < 0.f)
		return IT_OUTSIDE;

	//Get the near solution
	float d_sqrt = sqrtf(d);
	float dist = (-b - d_sqrt) / (2.f * a);
	if(dist >= 0.f)
		return IT_INTERSECT;
	else
		return IT_OUTSIDE;

	//float tca = vec3_dot(&centered_origin, &ray->direction);
	//if(tca < 0.0) return IT_OUTSIDE;

	//float L_dot = vec3_dot(&centered_origin, &centered_origin);
	//float d2 = L_dot - (tca * tca);
	//float radius_sqr = radius * radius;

	//if (d2 > radius_sqr) return IT_OUTSIDE;
	//float thc = sqrtf(radius_sqr - d2);
	//float t0 = tca - thc;
	//float t1 = tca + thc;

	//if(t0 > t1)
	//{
	//	float temp = t0;
	//	t0 = t1;
	//	t1 = temp;
	//}

	//if(t0 < 0)
	//{
	//	t0 = t1;
	//	if(t0 < 0) return IT_OUTSIDE;
	//}

	//return IT_INTERSECT;
}

float bv_distance_ray_plane(struct Ray* ray, Plane* plane)
{
	float dot = vec3_dot(&plane->normal, &ray->direction);
	float abs_dot = fabsf(dot);
	if(abs_dot >= EPSILON)
	{
		float dot_origin = vec3_dot(&plane->normal, &ray->origin);
		float t = -(dot_origin + plane->constant) / dot;
		if(t >= 0.0f)
			return t;
		else
			return INFINITY;
	}
	else
		return INFINITY;

}

void bv_bounding_box_vertices_get(struct Bounding_Box* bounding_box, vec3 out_vertices[8])
{
	vec3_fill(&out_vertices[0], bounding_box->min.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[1], bounding_box->max.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[2], bounding_box->min.x, bounding_box->max.y, bounding_box->min.z);
	vec3_fill(&out_vertices[3], bounding_box->min.x, bounding_box->min.y, bounding_box->max.z);

	vec3_fill(&out_vertices[4], bounding_box->max.x, bounding_box->max.y, bounding_box->max.z);
	vec3_fill(&out_vertices[5], bounding_box->min.x, bounding_box->max.y, bounding_box->max.z);
	vec3_fill(&out_vertices[6], bounding_box->max.x, bounding_box->min.y, bounding_box->max.z);
	vec3_fill(&out_vertices[7], bounding_box->max.x, bounding_box->max.y, bounding_box->min.z);
}
