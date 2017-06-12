#ifndef MODEL_H
#define MODEL_H

#include "linmath.h"

struct Entity;

void model_create(struct Entity* entity, const char* geo_name, const char* material_name);
void model_destroy(struct Entity* entity);
int  model_set_material_param(struct Entity* entity, const char* name, void* value);
int  model_get_material_param(struct Entity* entity, const char* name, void* value_out);

#endif
