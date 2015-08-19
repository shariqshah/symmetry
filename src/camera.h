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
};

struct Camera* camera_get(int index);
void camera_init(void);
void camera_cleanup(void);
void camera_remove(int index);
int  camera_create(int node, int width, int height);
void camera_update_view_proj(struct Camera* camera);
void camera_update_view(struct Camera* camera);
void camera_update_proj(struct Camera* camera);


#endif
