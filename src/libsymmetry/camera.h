#ifndef CAMERA_H
#define CAMERA_H

struct Entity;

void camera_destroy(struct Entity* entity);
void camera_create(struct Entity* entity, int width, int height);
void camera_update_view_proj(struct Entity* entity);
void camera_update_view(struct Entity* entity);
void camera_update_proj(struct Entity* entity);
void camera_attach_fbo(struct Entity* entity,
					   int            width,
					   int            height,
					   int            has_depth,
					   int            has_color,
					   int            resizeable);
/* void camera_resize_all(int width, int height); */

#endif
