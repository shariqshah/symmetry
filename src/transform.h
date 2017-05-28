#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "linmath.h"

enum Transform_Space { TS_LOCAL, TS_PARENT, TS_WORLD};

struct Transform;

void transform_create(struct Transform* transform, int parent_entity);
void transform_destroy(struct Transform* transform);
void transform_translate(struct Transform* transform, vec3* amount, enum Transform_Space space);
void transform_rotate(struct Transform*    transform,
					  vec3*                axis,
					  float                angle,
					  enum Transform_Space space);
void transform_scale(struct Transform* transform, vec3* scale);
void transform_set_position(struct Transform* transform, vec3* new_position);
void transform_get_forward(struct Transform* transform, vec3* res);
void transform_get_lookat(struct Transform* transform, vec3* res);
void transform_get_up(struct Transform* transform, vec3* res);
void transform_get_right(struct Transform* transform, vec3* res);
void transform_update_transmat(struct Transform* transform);
void transform_get_absolute_pos(struct Transform* transform, vec3* res);
void transform_get_absolute_rot(struct Transform* transform, quat* res);
void transform_get_absolute_scale(struct Transform* transform, vec3* res);
void transform_get_absolute_lookat(struct Transform* transform, vec3* res);
void transform_get_absolute_up(struct Transform* transform, vec3* res);
void transform_get_absolute_right(struct Transform* transform, vec3* res);
void transform_get_absolute_forward(struct Transform* transform, vec3* res);

#endif
