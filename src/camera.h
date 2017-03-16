#ifndef camera_H
#define camera_H

#include "linmath.h"

struct Camera
{
	int   node;
	mat4  proj_mat;
	mat4  view_mat;
	mat4  view_proj_mat;
	float fov;
	float aspect_ratio;
	float nearz;
	float farz;
	int   ortho;
	int   fbo;
	int   render_tex;
	int   depth_tex;
	vec4  clear_color;
	vec4  frustum[6];
};

struct Camera* camera_get(int index);
struct Camera* camera_get_all(void);
struct Camera* camera_get_primary(void);
void camera_init(void);
void camera_cleanup(void);
void camera_remove(int index);
int  camera_create(int node, int width, int height);
void camera_update_view_proj(struct Camera* camera);
void camera_update_view(struct Camera* camera);
void camera_update_proj(struct Camera* camera);
void camera_attach_fbo(struct Camera* camera,
					   int            width,
					   int            height,
					   int            has_depth,
					   int            has_color,
					   int            resizeable);
void camera_set_primary_viewer(struct Camera* camera);

#endif
