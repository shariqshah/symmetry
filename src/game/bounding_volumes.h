#ifndef BOUNDING_VOLUMES_H
#define BOUNDING_VOLUMES_H

#include "../common/linmath.h"
#include "../common/num_types.h"

#define MAX_RAYCAST_ENTITIES_INTERSECT 256

struct Bounding_Box
{
	vec3 min;
	vec3 max;
};

struct Bounding_Sphere
{
	vec3 center;
	float radius;
};

enum Intersection_Type
{
	IT_OUTSIDE = 0,
	IT_INTERSECT,
	IT_INSIDE,
};

enum Frustum_Planes
{
	FP_LEFT = 0,
	FP_RIGHT,
	FP_BOTTOM,
	FP_TOP,
	FP_NEAR,
	FP_FAR,
	FP_NUM_PLANES
};

struct Ray
{
	vec3 direction;
	vec3 origin;
};


struct Raycast_Result
{
	struct Entity* entities_intersected[MAX_RAYCAST_ENTITIES_INTERSECT];
	int            num_entities_intersected;
};

int   bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box, vec3* box_abs_position, vec3* box_abs_scale);
int   bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, vec3* sphere_abs_pos, vec3* sphere_abs_scale);
bool  bv_intersect_frustum_point(vec4* frustum, const vec3* point);
bool  bv_intersect_sphere_ray(struct Bounding_Sphere* sphere, vec3* sphere_abs_position, struct Ray* ray);
float bv_distance_ray_plane(struct Ray* ray, Plane* plane);

#endif
