#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../common/linmath.h"
#include "../common/num_types.h"

enum Transform_Space { TS_LOCAL = 0, TS_PARENT, TS_WORLD};

struct Entity;

void transform_init(struct Entity* entity, struct Entity* parent);
void transform_destroy(struct Entity* entity);
void transform_translate(struct Entity* entity, vec3* amount, enum Transform_Space space);
void transform_rotate(struct Entity*    transform,
					  vec3*                axis,
					  float                angle,
					  enum Transform_Space space);
void transform_scale(struct Entity* entity, vec3* scale);
void transform_set_position(struct Entity* entity, vec3* new_position);
void transform_get_forward(struct Entity* entity, vec3* res);
void transform_get_lookat(struct Entity* entity, vec3* res);
void transform_get_up(struct Entity* entity, vec3* res);
void transform_get_right(struct Entity* entity, vec3* res);
void transform_update_transmat(struct Entity* entity);
void transform_get_absolute_position(struct Entity* entity, vec3* res);
void transform_get_absolute_rot(struct Entity* entity, quat* res);
void transform_get_absolute_scale(struct Entity* entity, vec3* res);
void transform_get_absolute_lookat(struct Entity* entity, vec3* res);
void transform_get_absolute_up(struct Entity* entity, vec3* res);
void transform_get_absolute_right(struct Entity* entity, vec3* res);
void transform_get_absolute_forward(struct Entity* entity, vec3* res);
void transform_child_add(struct Entity* entity, struct Entity* child, bool update_transmat);
bool transform_child_remove(struct Entity* entity, struct Entity* child);
void transform_parent_set(struct Entity* entity, struct Entity* parent, bool update_transmat);

#endif
