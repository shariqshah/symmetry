#include "bounding_volumes.h"
#include "../common/log.h"

#include <math.h>

int bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box)
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
	//for(int i = 0; i < 6; i++)
	//{
	//	vec3  normal     = {frustum[i].x, frustum[i].y, frustum[i].z};
	//	float distance   = frustum[i].w;
	//	float d          = vec3_dot(&normal, &center);
	//	vec3  abs_normal = {fabsf(normal.x), fabsf(normal.y), fabsf(normal.z)};
	//	float r          = vec3_dot(&half_ext, &abs_normal);
	//	if(d + r < -distance)
	//		return IT_OUTSIDE;
	//}
	//return IT_INSIDE;

	vec3 edge = { 0.f, 0.f, 0.f };
	vec3_sub(&edge, &center, &box->min);
	bool all_inside = true;
	for(int i = 0; i < 6; i++)
	{
		vec3  normal     = {frustum[i].x, frustum[i].y, frustum[i].z};
		float d = frustum[i].w;
		float dist = vec3_dot(&normal, &center) + d;
		vec3 abs_normal = { fabsf(normal.x), fabsf(normal.y), fabsf(normal.z) };
		float abs_dist = vec3_dot(&abs_normal, &edge);

		if(dist < -abs_dist)
			return IT_OUTSIDE;
		else if(dist < abs_dist)
			all_inside = false;
	}
	return all_inside ? IT_INSIDE : IT_INTERSECT;
}

int bv_intersect_frustum_box_with_abs_transform(vec4* frustum, struct Bounding_Box* box, vec3* box_abs_position, vec3* box_abs_scale)
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

bool bv_point_inside_frustum(vec4* frustum, const vec3* point)
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

void bv_bounding_box_vertices_get_line_visualization(struct Bounding_Box* bounding_box, vec3 out_vertices[24])
{
	// Back
	vec3_fill(&out_vertices[0], bounding_box->min.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[1], bounding_box->min.x, bounding_box->max.y, bounding_box->min.z);

	vec3_fill(&out_vertices[2], bounding_box->min.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[3], bounding_box->max.x, bounding_box->min.y, bounding_box->min.z);

	vec3_fill(&out_vertices[4], bounding_box->min.x, bounding_box->max.y, bounding_box->min.z);
	vec3_fill(&out_vertices[5], bounding_box->max.x, bounding_box->max.y, bounding_box->min.z);

	vec3_fill(&out_vertices[6], bounding_box->max.x, bounding_box->max.y, bounding_box->min.z);
	vec3_fill(&out_vertices[7], bounding_box->max.x, bounding_box->min.y, bounding_box->min.z);

	// Front
	vec3_fill(&out_vertices[8], bounding_box->min.x, bounding_box->min.y, bounding_box->max.z);
	vec3_fill(&out_vertices[9], bounding_box->min.x, bounding_box->max.y, bounding_box->max.z);

	vec3_fill(&out_vertices[10], bounding_box->min.x, bounding_box->min.y, bounding_box->max.z);
	vec3_fill(&out_vertices[11], bounding_box->max.x, bounding_box->min.y, bounding_box->max.z);
	
	vec3_fill(&out_vertices[12], bounding_box->min.x, bounding_box->max.y, bounding_box->max.z);
	vec3_fill(&out_vertices[13], bounding_box->max.x, bounding_box->max.y, bounding_box->max.z);

	vec3_fill(&out_vertices[14], bounding_box->max.x, bounding_box->max.y, bounding_box->max.z);
	vec3_fill(&out_vertices[15], bounding_box->max.x, bounding_box->min.y, bounding_box->max.z);

	// Left
	vec3_fill(&out_vertices[16], bounding_box->min.x, bounding_box->max.y, bounding_box->min.z);
	vec3_fill(&out_vertices[17], bounding_box->min.x, bounding_box->max.y, bounding_box->max.z);
	
	vec3_fill(&out_vertices[18], bounding_box->min.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[19], bounding_box->min.x, bounding_box->min.y, bounding_box->max.z);

	// Right
	vec3_fill(&out_vertices[20], bounding_box->max.x, bounding_box->min.y, bounding_box->min.z);
	vec3_fill(&out_vertices[21], bounding_box->max.x, bounding_box->min.y, bounding_box->max.z);
	
	vec3_fill(&out_vertices[22], bounding_box->max.x, bounding_box->max.y, bounding_box->min.z);
	vec3_fill(&out_vertices[23], bounding_box->max.x, bounding_box->max.y, bounding_box->max.z);
}

int bv_intersect_bounding_box_ray(struct Bounding_Box* box, struct Ray* ray)
{
	float tmin = (box->min.x - ray->origin.x) / ray->direction.x;
	float tmax = (box->max.x - ray->origin.x) / ray->direction.x;

	if(tmin > tmax)
	{
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}

	float tymin = (box->min.y - ray->origin.y) / ray->direction.y;
	float tymax = (box->max.y - ray->origin.y) / ray->direction.y;

	if(tymin > tymax)
	{
		float temp = tymin;
		tymin = tymax;
		tymax = temp;
	}

	if((tmin > tymax) || (tymin > tmax))
		return IT_OUTSIDE;

	if(tymin > tmin)
		tmin = tymin;

	if(tymax < tmax)
		tmax = tymax;

	float tzmin = (box->min.z - ray->origin.z) / ray->direction.z;
	float tzmax = (box->max.z - ray->origin.z) / ray->direction.z;

	if(tzmin > tzmax)
	{
		float temp = tzmin;
		tzmin = tzmax;
		tzmax = temp;
	}

	if((tmin > tzmax) || (tzmin > tmax))
		return IT_OUTSIDE;

	if(tzmin > tmin)
		tmin = tzmin;

	if(tzmax < tmax)
		tmax = tzmax;

	return tmin < 0.f ? IT_INSIDE : IT_INTERSECT;
}

float bv_distance_ray_bounding_box(struct Ray* ray, struct Bounding_Box* box)
{
	float tmin = (box->min.x - ray->origin.x) / ray->direction.x;
	float tmax = (box->max.x - ray->origin.x) / ray->direction.x;

	if(tmin > tmax)
	{
		float temp = tmin;
		tmin = tmax;
		tmax = temp;
	}

	float tymin = (box->min.y - ray->origin.y) / ray->direction.y;
	float tymax = (box->max.y - ray->origin.y) / ray->direction.y;

	if(tymin > tymax)
	{
		float temp = tymin;
		tymin = tymax;
		tymax = temp;
	}

	if((tmin > tymax) || (tymin > tmax))
		return INFINITY;

	if(tymin > tmin)
		tmin = tymin;

	if(tymax < tmax)
		tmax = tymax;

	float tzmin = (box->min.z - ray->origin.z) / ray->direction.z;
	float tzmax = (box->max.z - ray->origin.z) / ray->direction.z;

	if(tzmin > tzmax)
	{
		float temp = tzmin;
		tzmin = tzmax;
		tzmax = temp;
	}

	if((tmin > tzmax) || (tzmin > tmax))
		return INFINITY;

	if(tzmin > tmin)
		tmin = tzmin;

	if(tzmax < tmax)
		tmax = tzmax;

	return tmin >= 0.f ? tmin : tmax; // if tmin < 0, return the max value since it represents the hit in front of us. We don't care about hits behind us
}

int bv_intersect_bounding_boxes(struct Bounding_Box* b1, struct Bounding_Box* b2)
{
	if(b2->max.x < b1->min.x || b2->min.x > b1->max.x || b2->max.y < b1->min.y || b2->min.y > b1->max.y ||
	   b2->max.z < b1->min.z || b2->min.z > b1->max.z)
		return IT_OUTSIDE;
	else if(b2->min.x < b1->min.x || b2->max.x > b1->max.x || b2->min.y < b1->min.y || b2->max.y > b1->max.y ||
			b2->min.z < b1->min.z || b2->max.z > b1->max.z)
		return IT_INTERSECT;
	else
		return IT_INSIDE; // b2 is inside b1
}

bool bv_point_inside_bounding_box(struct Bounding_Box* box, vec3 point)
{
	if(point.x < box->min.x || point.x > box->max.x || point.y < box->min.y || point.y > box->max.y || point.z < box->min.z || point.z > box->max.z)
		return false;
	else
		return true;
}

vec3 bv_bounding_box_normal_from_intersection_point(struct Bounding_Box* box, vec3 intersection_point)
{
	vec3 center = { (box->max.x + box->min.x) * 0.5f,
				    (box->max.y + box->min.y) * 0.5f,
				    (box->max.z + box->min.z) * 0.5f };

	vec3 local_point = { 0.f, 0.f, 0.f };
	vec3_sub(&local_point, &intersection_point, &center);

	vec3 d = { (box->min.x - box->max.x) * 0.5f,
			  (box->min.y - box->max.y) * 0.5f,
			  (box->min.z - box->max.z) * 0.5f };
	float bias = 1.000001f;
	vec3 normal = { (int)(local_point.x / fabsf(d.x) * bias),
					(int)(local_point.y / fabsf(d.y) * bias),
					(int)(local_point.z / fabsf(d.z) * bias) };
	vec3_norm(&normal, &normal);
	return normal;
}
