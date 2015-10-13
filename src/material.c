#include "material.h"
#include "array.h"
#include "shader.h"
#include "string_utils.h"
#include "log.h"
#include "model.h"

#include <string.h>
#include <stdlib.h>

static struct Material* material_list;
static int* empty_indices;

void material_init(void)
{
	material_list = array_new(struct Material);
	empty_indices = array_new(int);

	/* TODO: implement reading material definitions from files */
	/* Simple unshaded material */
	struct Material* unshaded_mat = array_grow(material_list, struct Material);
	unshaded_mat->name = str_new("Unshaded");
	unshaded_mat->shader = shader_create("unshaded.vert", "unshaded.frag");
	unshaded_mat->registered_models = array_new(int);
	unshaded_mat->model_params = array_new(struct Uniform);
	unshaded_mat->pipeline_params = array_new(struct Uniform);
	unshaded_mat->active = 1;

	struct Uniform* uniform = array_grow(unshaded_mat->pipeline_params, struct Uniform);
	uniform->name = str_new("mvp");
	uniform->type = UT_MAT4;
	uniform->location = shader_get_uniform_location(unshaded_mat->shader, uniform->name);

	uniform = array_grow(unshaded_mat->model_params, struct Uniform);
	uniform->name = str_new("diffuse_color");
	uniform->type = UT_VEC4;
	uniform->location = shader_get_uniform_location(unshaded_mat->shader, "diffuse_color");
}

struct Material* material_get_all_materials(void)
{
	return material_list;
}

void material_cleanup(void)
{
	for(int i = 0; i < array_len(material_list); i++)
		material_remove(i);
	array_free(empty_indices);
}

int material_register_model(struct Model* model, int model_index, const char* material_name)
{
	assert(material_name && model);
	int success = 0;
	int index = material_get_index(material_name);
	if(index < -1)
	{
		log_error("material:register_model", "Material '%s' not found", material_name);
		return success;
	}
	
	struct Material* material = &material_list[index];
	model->material = index;
	model->material_params = array_new(struct Material_Param);
	for(int i = 0; i < array_len(material->model_params); i++)
	{
		/* set up parameters */
		struct Uniform* uniform = &material->model_params[i];
		struct Material_Param* param = array_grow(model->material_params, struct Material_Param);
		param->uniform_index = i;
		switch(uniform->type)
		{
		case UT_INT:
			param->value = malloc(sizeof(int));
			*((int*)param->value) = -1;
			break;
		case UT_FLOAT:
			param->value = malloc(sizeof(float));
			*((float*)param->value) = -1.f;
			break;
		case UT_VEC2:
			param->value = malloc(sizeof(vec2));
			vec2_fill((vec2*)param->value, 0.f, 0.f);
			break;
		case UT_VEC3:
			param->value = malloc(sizeof(vec3));
			vec3_fill((vec3*)param->value, 0.f, 0.f, 0.f);
			break;
		case UT_VEC4:
			param->value = malloc(sizeof(vec4));
			vec4_fill((vec4*)param->value, 0.f, 0.f, 0.f, 0.f);
			break;
		case UT_MAT4:
			param->value = malloc(sizeof(mat4));
			mat4_identity((mat4*)param->value);
			break;
		}
	}
	array_push(material->registered_models, model_index, int);
	success = 1;	
	return success;
}

void material_unregister_model(struct Model* model, int model_index)
{
	assert(model);
	struct Material* material = &material_list[model->material];
	/* Remove model index from material registry*/
	for(int i = 0; i < array_len(material->registered_models); i++)
	{
		if(material->registered_models[i] == model_index)
		{
			array_remove_at(material->registered_models, i);
			break;
		}
	}
}

struct Material* material_find(const char* material_name)
{
	struct Material* material = NULL;
	int index = material_get_index(material_name);
	if(index > -1) material = &material_list[index];
	return material;
}

int material_get_index(const char* material_name)
{
	int index = -1;
	for(int i = 0; i < array_len(material_list); i++)
	{
		if(!material_list[i].name) continue;
		
		if(strcmp(material_name, material_list[i].name) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

struct Material* material_get(int index)
{
	struct Material* material = NULL;
	if(index > -1 && index < array_len(material_list))
		material = &material_list[index];
	return material;
}

void material_remove(int index)
{
	assert(index > -1 && index < array_len(material_list));
	struct Material* material = &material_list[index];
	if(!material->active)
		return;
	material->active = 0;
	array_free(material->registered_models);
	for(int i  = 0; i < array_len(material->model_params); i++)
		free(material->model_params[i].name);
	array_free(material->model_params);
	
	for(int i  = 0; i < array_len(material->pipeline_params); i++)
		free(material->pipeline_params[i].name);
	array_free(material->pipeline_params);
	
	log_message("Removed material '%s'", material->name);
	free(material->name);
	array_push(empty_indices, index, int);
}

