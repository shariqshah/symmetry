#ifndef _transform_H
#define _transform_H

#include "linmath.h"

enum Transform_Space { TS_LOCAL, TS_PARENT, TS_WORLD};

struct Transform
{
	int  node;
	vec3 position;
	vec3 scale;
	//mat4 rotation;
	mat4 trans_mat;
	quat rotq;
};

struct Transform* transform_get(int index);
void transform_remove(int index);
void transform_init(void);
void transform_cleanup(void);
int  transform_create(int node);
void transform_translate(struct Transform* transform, vec3 amount, enum Transform_Space space);
void transform_rotate(struct Transform* transform,
					  vec3  axis,
					  float angle,
					  enum Transform_Space space);
void transform_scale(struct Transform* transform, vec3 scale);
void transform_set_position(struct Transform* transform, vec3 new_position);
void transform_get_forward(struct Transform* transform, vec3 res);
void transform_get_lookat(struct Transform* transform, vec3 res);
void transform_get_up(struct Transform* transform, vec3 res);
void transform_get_right(struct Transform* transform, vec3 res);
void transform_update_transmat(struct Transform* transform);
void transform_get_absolute_pos(struct Transform* transform, vec3 res);
void transform_get_absolute_lookat(struct Transform* transform, vec3 res);

#endif
