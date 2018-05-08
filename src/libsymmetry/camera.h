#ifndef CAMERA_H
#define CAMERA_H

#include "../common/num_types.h"

struct Camera;

void camera_reset(struct Camera* camera);
void camera_init(struct Camera* camera, int width, int height);
void camera_update_view_proj(struct Camera* camera);
void camera_update_view(struct Camera* camera);
void camera_update_proj(struct Camera* camera);
void camera_attach_fbo(struct Camera* camera,
					   int            width,
					   int            height,
					   bool           has_depth,
					   bool           has_color,
					   bool           resizeable);
/* void camera_resize_all(int width, int height); */

#endif
