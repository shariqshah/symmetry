#ifndef CAMERA_H
#define CAMERA_H

struct Camera;
struct Transform;

void camera_destroy(struct Camera* camera);
void camera_create(struct Camera* camera, struct Transform* transform, int width, int height);
void camera_update_view_proj(struct Camera* camera);
void camera_update_view(struct Camera* camera, struct Transform* transform);
void camera_update_proj(struct Camera* camera);
void camera_attach_fbo(struct Camera* camera,
					   int            width,
					   int            height,
					   int            has_depth,
					   int            has_color,
					   int            resizeable);
/* void camera_resize_all(int width, int height); */

#endif
