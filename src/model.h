#ifndef MODEL_H
#define MODEL_H

#include "linmath.h"

struct Material_Param;
struct Entity;

void model_init(void);
void model_create(struct Entity* entity, const char* geo_name, const char* material_name);
void model_destroy(struct Entity* entity);
void model_render_all(struct Entity* camera_entity, int draw_mode);
int  model_set_material_param(struct Entity* entity, const char* name, void* value);
int  model_get_material_param(struct Entity* entity, const char* name, void* value_out);
void model_render_all_debug(struct Entity* camera_entity,
							int            debug_shader,
							int            draw_mode,
							const vec4*    debug_color);

#endif
