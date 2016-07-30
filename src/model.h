#ifndef model_H
#define model_H

#include "linmath.h"

struct Camera;
struct Material_Param;

struct Model
{
	int node;
	int geometry_index;
	int material;
	struct Material_Param* material_params;
};

struct Model* model_get(int index);
void model_init(void);
int  model_create(int node, const char* geo_name, const char* material_name);
void model_remove(int index);
void model_cleanup(void);
void model_render_all(struct Camera* camera);
int  model_set_material_param(struct Model* model, const char* name, void* value);
int  model_get_material_param(struct Model* model, const char* name, void* value_out);

#endif
