#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../common/num_types.h"
#include "../common/linmath.h"
#include "bounding_volumes.h"
#include "gl_load.h"

struct Entity;

extern GLenum* draw_modes;

enum Geometry_Draw_Mode
{
	GDM_TRIANGLES = 0,
	GDM_LINES,
	GDM_POINTS,
	GDM_LINE_STRIP,
	GDM_LINE_LOOP,
	GDM_TRIANGLE_FAN,
	GDM_NUM_DRAWMODES
};

struct Geometry 
{
	char* 		  		   filename;
	int   		  		   draw_indexed;
	uint  		  		   vao;
	uint  		  		   vertex_vbo;
	uint  		  		   uv_vbo;
	uint  		  		   normal_vbo;
	uint  		  		   color_vbo;
	uint  		  		   index_vbo;
	uint                   vertices_length;
	uint                   indices_length;
	int   		  		   ref_count;
	struct Bounding_Box    bounding_box;
	struct Bounding_Sphere bounding_sphere;
};

void 			 geom_init(void);
int  			 geom_create_from_file(const char* name);
int  			 geom_find(const char* filename);
void 			 geom_remove(int index);
void 			 geom_cleanup(void);
void 			 geom_render(int index, enum Geometry_Draw_Mode draw_mode);
struct Geometry* geom_get(int index);
int  			 geom_render_in_frustum(int                      index,
	 			 						vec4*                   frustum,
	 			 						struct Entity*          transform,
	 			 						enum Geometry_Draw_Mode draw_mode);
int  			 geom_create(const char* name,
							 vec3* vertices,
							 vec2* uvs,
							 vec3* normals,
							 uint* indices,
							 vec3* vertex_colors); // Note: caller responsible for freeing up the data passed in

#endif
