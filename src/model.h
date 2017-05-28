#ifndef MODEL_H
#define MODEL_H

#include "linmath.h"

struct Camera;
struct Material_Param;
struct Model;
struct Entity;

void 		  model_init(void);
void  		  model_create(struct Model* model, int entity_id, const char* geo_name, const char* material_name);
void 		  model_destroy(struct Model* model, int entity_id);
void 		  model_cleanup(void);
void 		  model_render_all(struct Entity* camera_entity, int draw_mode);
int  		  model_set_material_param(struct Model* model, const char* name, void* value);
int  		  model_get_material_param(struct Model* model, const char* name, void* value_out);
void          model_render_all_debug(struct Entity* camera_entity,
									 int            debug_shader,
									 int            draw_mode,
									 const vec4*    debug_color);

#endif
