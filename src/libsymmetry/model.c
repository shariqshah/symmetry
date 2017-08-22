#include "model.h"
#include "../common/array.h"
#include "../common/log.h"
#include "entity.h"
#include "texture.h"
#include "material.h"
#include "geometry.h"
#include "shader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void model_create(struct Entity* entity, const char* geo_name, const char* material_name)
{
	struct Model* model = &entity->model;
	/* if no name is given for geometry, use default */
	if(!geo_name) geo_name = "default.pamesh";
	int geo_index = geom_create_from_file(geo_name);

	model->geometry_index = geo_index;
	if(!material_register_model(entity, material_name ? material_name : "Unshaded"))
	{
		log_error("model:create", "Unable to register model with Unshaded material, component not added");
		model_destroy(entity);
	}
}

void model_destroy(struct Entity* entity)
{
	struct Model* model = &entity->model;
	geom_remove(model->geometry_index);
	model->geometry_index = -1;
	material_unregister_model(entity);
	/* deallocate all params */
	for(int i = 0; i < array_len(model->material_params); i++)
		free(model->material_params[i].value);

	array_free(model->material_params);
	model->material_params = NULL;
	model->material        = -1;
}

int model_set_material_param(struct Entity* entity, const char* name, void* value)
{
	assert(name && value);
	struct Model* model = &entity->model;
	int success = 0;
	struct Material* material = material_get(model->material);
	for(int i = 0; i < array_len(model->material_params); i++)
	{
		struct Material_Param* param = &model->material_params[i];
		struct Uniform* uniform = &material->model_params[param->uniform_index];
		if(strcmp(uniform->name, name) == 0)
		{
			success = 1;
			switch(uniform->type)
			{
			case UT_INT:   *((int*)param->value)   = *((int*)value);       break;
			case UT_FLOAT: *((float*)param->value) = *((float*)value);     break;
			case UT_VEC2:  vec2_assign((vec2*)param->value, (vec2*)value); break;
			case UT_VEC3:  vec3_assign((vec3*)param->value, (vec3*)value); break;
			case UT_VEC4:  vec4_assign((vec4*)param->value, (vec4*)value); break;
			case UT_MAT4:  mat4_assign((mat4*)param->value, (mat4*)value); break;
			case UT_TEX:   *((int*)param->value) = *((int*)value);		   break;
			default:
				log_error("model:set_material_param", "Invalid parameter type");
				success = 0;
				break;
			}
			break; /* break for */
		}
	}
	return success;
}

int model_get_material_param(struct Entity* entity, const char* name, void* value_out)
{
	assert(name && value_out);
	struct Model* model = &entity->model;
	int success = 0;
	struct Material* material = material_get(model->material);
	for(int i = 0; i < array_len(model->material_params); i++)
	{
		struct Material_Param* param = &model->material_params[i];
		struct Uniform* uniform      = &material->model_params[param->uniform_index];
		if(strcmp(uniform->name, name) == 0)
		{
			switch(uniform->type)
			{
			case UT_INT:   *((int*)value_out)   = *((int*)param->value);       break;
			case UT_FLOAT: *((float*)value_out) = *((float*)param->value);     break;
			case UT_VEC2:  vec2_assign((vec2*)value_out, (vec2*)param->value); break;
			case UT_VEC3:  vec3_assign((vec3*)value_out, (vec3*)param->value); break;
			case UT_VEC4:  vec4_assign((vec4*)value_out, (vec4*)param->value); break;
			case UT_MAT4:  mat4_assign((mat4*)value_out, (mat4*)param->value); break;
			}
			success = 1;
			break;
		}
	}
	return success;
}
