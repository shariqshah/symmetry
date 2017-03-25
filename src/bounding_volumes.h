#ifndef bounding_volumes_H
#define bounding_volumes_H

#include "linmath.h"

struct Transform;

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

int  bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box, struct Transform* transform);
int  bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, struct Transform* transform);
int  bv_intersect_frustum_point(vec4* frustum, const vec3* point);

#endif
