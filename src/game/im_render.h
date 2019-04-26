#ifndef IM_RENDER_H
#define IM_RENDER_H

#include "../common/linmath.h"

struct IM_Vertex
{
	vec3 position;
};

enum IM_Geom_Type
{
	IGT_PRIMITIVE = 0,
	IGT_DYNAMIC
};

struct IM_Geom
{
	vec3 position;
	quat rotation;
	vec3 scale;
	vec4 color;
	int  type;
	union
	{
		struct
		{
			int  start_index;
			int  num_vertices;
		};
		int prim_geom_index;
		
	};
	int draw_mode;
	int draw_order;
};

struct Camera;

void im_init(void);
void im_cleanup(void);
void im_begin(vec3 position, quat rotation, vec3 scale, vec4 color, int draw_mode, int draw_order);
void im_pos(float x, float y, float z);
void im_box(float x, float y, float z, vec3 position, quat rotation, vec4 color, int draw_mode, int draw_order);
void im_sphere(float radius, vec3 position, quat rotation, vec4 color, int draw_mode, int draw_order);
void im_line(vec3 p1, vec3 p2, vec3 position, quat rotation, vec3 scale, vec4 color, int draw_mode, int draw_order);
void im_end(void);
void im_render(struct Camera* active_viewer);

#endif