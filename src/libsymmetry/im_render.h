#ifndef IM_RENDER_H
#define IM_RENDER_H

#include "../common/linmath.h"

struct IM_Vertex
{
	vec3 position;
	vec4 color;
};

struct IM_Geom
{
	vec3 position;
	quat rotation;
	vec3 scale;
	int  start_index;
	int  num_vertices;
	int  draw_mode;
};

struct Entity;

void im_init(void);
void im_cleanup(void);
void im_begin(vec3 position, quat rotation, vec3 scale, int draw_mode);
void im_pos(float x, float y, float z);
void im_color(float r, float g, float b, float a); // set active color
void im_end(void);
void im_render(struct Entity* active_viewer);

#endif