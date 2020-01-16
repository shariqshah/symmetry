#ifndef BOUNDING_VOLUMES_H
#define BOUNDING_VOLUMES_H

#include "../common/linmath.h"
#include "../common/num_types.h"

#define MAX_RAYCAST_ENTITIES_INTERSECT 32

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

int   bv_intersect_frustum_box_with_abs_transform(vec4* frustum, struct Bounding_Box* box, vec3* box_abs_position, vec3* box_abs_scale);
int   bv_intersect_frustum_box(vec4* frustum, struct Bounding_Box* box);
int   bv_intersect_frustum_sphere(vec4* frustum, struct Bounding_Sphere* sphere, vec3* sphere_abs_pos, vec3* sphere_abs_scale);
bool  bv_point_inside_frustum(vec4* frustum, const vec3* point);
bool  bv_point_inside_bounding_box(struct Bounding_Box* box, vec3 point);
int   bv_intersect_bounding_box_ray(struct Bounding_Box* box, struct Ray* ray);
int   bv_intersect_sphere_ray(struct Bounding_Sphere* sphere, vec3* sphere_abs_position, vec3* sphere_abs_scale, struct Ray* ray);
int   bv_intersect_bounding_boxes(struct Bounding_Box* b1, struct Bounding_Box* b2);
float bv_distance_ray_plane(struct Ray* ray, Plane* plane);
void  bv_bounding_box_vertices_get(struct Bounding_Box* box, vec3 out_vertices[8]);
void  bv_bounding_box_vertices_get_line_visualization(struct Bounding_Box* bounding_box, vec3 out_vertices[24]);
float bv_distance_ray_bounding_box(struct Ray* ray, struct Bounding_Box* box);
vec3  bv_bounding_box_normal_from_intersection_point(struct Bounding_Box* box, struct Ray* ray, vec3 intersection_point);

#endif
