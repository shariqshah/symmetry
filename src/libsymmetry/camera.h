#ifndef CAMERA_H
#define CAMERA_H

#include "../common/num_types.h"

struct Entity;

void camera_destroy(struct Entity* entity);
void camera_create(struct Entity* entity, int width, int height);
void camera_update_view_proj(struct Entity* entity);
void camera_update_view(struct Entity* entity);
void camera_update_proj(struct Entity* entity);
void camera_attach_fbo(struct Entity* entity,
					   int            width,
					   int            height,
					   bool           has_depth,
					   bool           has_color,
					   bool           resizeable);
/* void camera_resize_all(int width, int height); */

#endif
