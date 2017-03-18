#ifndef MODEL_H
#define MODEL_H

#include "linmath.h"
#include "geometry.h"

struct Camera;
struct Material_Param;

struct Model
{
	int                    node;
	int                    geometry_index;
	int                    material;
	struct Material_Param* material_params;
};

struct Model* model_get(int index);
struct Model* model_get_all(void);
void 		  model_init(void);
int  		  model_create(int node, const char* geo_name, const char* material_name);
void 		  model_remove(int index);
void 		  model_cleanup(void);
void 		  model_render_all(struct Camera* camera, enum Geometry_Draw_Mode draw_mode);
int  		  model_set_material_param(struct Model* model, const char* name, void* value);
int  		  model_get_material_param(struct Model* model, const char* name, void* value_out);
void          model_render_all_debug(struct Camera*          camera,
									 int                     debug_shader,
									 enum Geometry_Draw_Mode draw_mode,
									 const vec3*             debug_color);

#endif
