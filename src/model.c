#include "model.h"
#include "array.h"
#include "log.h"
#include "geometry.h"
#include "camera.h"
#include "entity.h"
#include "shader.h"
#include "transform.h"
#include "texture.h"
#include "renderer.h"
#include "material.h"
#include "light.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static struct Model* model_list;
static int* empty_indices;
struct Model* model_get(int index)
{
	struct Model* model = NULL;
	if(index > -1 && index < array_len(model_list))
		model = &model_list[index];
	else
		log_error("model:get", "Invalid index");
	return model;
}

void model_init(void)
{
	model_list = array_new(struct Model);
	empty_indices = array_new(int);
}

int model_create(int node, const char* geo_name, const char* material_name)
{
	/* if no name is given for geometry, use default */
	if(!geo_name) geo_name = "default.pamesh";
	int geo_index = geom_create_from_file(geo_name);
	int index = -1;
	struct Model* new_model = NULL;
	if(geo_index > -1)
	{
		if(array_len(empty_indices) > 0)
		{
			index = *array_get_last(empty_indices, int);
			array_pop(empty_indices);
			new_model = &model_list[index];
		}
		else
		{
			 new_model = array_grow(model_list, struct Model);
			 index = array_len(model_list) - 1;
		}
		new_model->node = node;
		new_model->geometry_index = geo_index;
		if(!material_register_model(new_model, index, material_name ? material_name : "Unshaded"))
		{
			log_error("model:create", "Unable to register model with Unshaded material, component not added");
			model_remove(index);
			index = -1;
		}
	}
	else
	{
		log_error("model:create", "Geometry '%s' not found.", geo_name);
	}
	return index;
}

void model_remove(int index)
{
	if(index > -1 && index < array_len(model_list))
	{
		struct Model* model = &model_list[index];
		model->node = -1;
		geom_remove(model->geometry_index);
		model->geometry_index = -1;
		material_unregister_model(model, index);
		/* deallocate all params */
		for(int i = 0; i < array_len(model->material_params); i++)
			free(model->material_params[i].value);

		array_free(model->material_params);
		array_push(empty_indices, index, int);
	}
	else
	{
		log_error("model:remove", "Invalid index");
	}
}

void model_cleanup(void)
{
	for(int i = 0; i < array_len(model_list); i++)
	{
		if(model_list[i].node != -1)
			model_remove(i);
	}
	array_free(model_list);
	array_free(empty_indices);
}

void model_render_all(struct Camera* camera)
{
	static mat4 mvp;
	struct Material* material_list = material_get_all_materials();
	for(int i = 0; i < array_len(material_list); i++)
	{
		/* for each material, get all the registered models and render them */
		struct Material* material = &material_list[i];
		if(!material->active)
			continue;

		shader_bind(material->shader);
		renderer_check_glerror("model:render_all:shader_bind");
		for(int j = 0; j < array_len(material->registered_models); j++)
		{
			/* for each registered model, set up uniforms and render */
			struct Model* model = &model_list[material->registered_models[j]];
			struct Entity* entity = entity_get(model->node);
			struct Transform* transform = entity_component_get(entity, C_TRANSFORM);

			/* set material params for the model */
			for(int k = 0; k < array_len(model->material_params); k++)
			{
				struct Material_Param* param = &model->material_params[k];
				struct Uniform* uniform = &material->model_params[param->uniform_index];
			    shader_set_uniform(uniform->type, uniform->location, param->value);
				renderer_check_glerror("model:render_all:material_param");
			}

			/* Set pipeline uniforms */
			for(int k = 0; k < array_len(material->pipeline_params); k++)
			{
				/* TODO: change this into something better */
				struct Uniform* uniform = &material->pipeline_params[k];
				if(strcmp(uniform->name, "mvp") == 0)
				{
					mat4_identity(&mvp);
					mat4_mul(&mvp, &camera->view_proj_mat, &transform->trans_mat);
					shader_set_uniform(uniform->type, uniform->location, &mvp);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "model_mat") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &transform->trans_mat);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "view_mat") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &camera->view_mat);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
			}

			if(material->lit)	/* Set light information */
			{
				int valid_light_count = 0;
				int* light_index_list = light_get_valid_indices(&valid_light_count);
				const int max_name_len = 64;
				char uniform_name[max_name_len];
				memset(uniform_name, '\0', max_name_len);
				for(int i = 0; i < valid_light_count; i++)
				{
					struct Light* light = light_get(light_index_list[i]); /* TODO: Cull lights according to camera frustum */
					struct Entity* light_entity = entity_get(light->node);
					struct Transform* transform = entity_component_get(light_entity, C_TRANSFORM);
					vec3 light_pos = {0, 0, 0};
					transform_get_absolute_pos(transform, &light_pos);

					if(light->type != LT_POINT)
					{
						snprintf(uniform_name, max_name_len, "lights[%d].direction", i);
						transform_get_absolute_lookat(transform, &light_pos);
						vec3_norm(&light_pos, &light_pos);
						shader_set_uniform_vec3(material->shader, uniform_name, &light_pos);
						memset(uniform_name, '\0', max_name_len);
					}


					if(light->type != LT_DIR)
					{
						snprintf(uniform_name, max_name_len, "lights[%d].position", i);
						shader_set_uniform_vec3(material->shader,  uniform_name, &light_pos);
						memset(uniform_name, '\0', max_name_len);

						snprintf(uniform_name, max_name_len, "lights[%d].outer_angle", i);
						shader_set_uniform_float(material->shader, uniform_name, light->outer_angle);
						memset(uniform_name, '\0', max_name_len);
					
						snprintf(uniform_name, max_name_len, "lights[%d].inner_angle", i);
						shader_set_uniform_float(material->shader, uniform_name, light->inner_angle);
						memset(uniform_name, '\0', max_name_len);
					
						snprintf(uniform_name, max_name_len, "lights[%d].falloff", i);
						shader_set_uniform_float(material->shader, uniform_name, light->falloff);
						memset(uniform_name, '\0', max_name_len);

						snprintf(uniform_name, max_name_len, "lights[%d].radius", i);
						shader_set_uniform_int(material->shader, uniform_name, light->radius);
						memset(uniform_name, '\0', max_name_len);
					}
					
					snprintf(uniform_name, max_name_len, "lights[%d].color", i);
					shader_set_uniform_vec4(material->shader,  uniform_name, &light->color);
					memset(uniform_name, '\0', max_name_len);
					
					snprintf(uniform_name, max_name_len, "lights[%d].intensity", i);
					shader_set_uniform_float(material->shader, uniform_name, light->intensity);
					memset(uniform_name, '\0', max_name_len);
					
					snprintf(uniform_name, max_name_len, "lights[%d].type", i);
					shader_set_uniform_int(material->shader, uniform_name, light->type);
					memset(uniform_name, '\0', max_name_len);
				}

				shader_set_uniform_int(material->shader, "total_active_lights", valid_light_count);
				struct Entity* camera_entity = entity_get(camera->node);
				struct Transform* camera_tran = entity_component_get(camera_entity, C_TRANSFORM);
				vec3 camera_pos = {0, 0, 0};
				transform_get_absolute_pos(camera_tran, &camera_pos);
				shader_set_uniform_vec3(material->shader, "camera_pos", &camera_pos);
			}
			
			/* Render the geometry */
			//geom_render_in_frustum(model->geometry_index, &camera->frustum[0], transform);
			geom_render(model->geometry_index);

			for(int k = 0; k < array_len(model->material_params); k++)
			{
				/* unbind textures, if any */
				struct Material_Param* param = &model->material_params[k];
				struct Uniform* uniform = &material->model_params[param->uniform_index];
				if(uniform->type == UT_TEX)
				{
					texture_unbind(*(int*)param->value);
					renderer_check_glerror("model:render_all:unbind_texture_uniform");
				}
			}
		}
		shader_unbind();
	}
}

int model_set_material_param(struct Model* model, const char* name, void* value)
{
	assert(model && name && value);
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
			case UT_INT:
				*((int*)param->value) = *((int*)value);
				break;
			case UT_FLOAT:
				*((float*)param->value) = *((float*)value);
				break;
			case UT_VEC2:
				vec2_assign((vec2*)param->value, (vec2*)value);
				break;
			case UT_VEC3:
				vec3_assign((vec3*)param->value, (vec3*)value);
				break;
			case UT_VEC4:
				vec4_assign((vec4*)param->value, (vec4*)value);
				break;
			case UT_MAT4:
				mat4_assign((mat4*)param->value, (mat4*)value);
				break;
			case UT_TEX:
				*((int*)param->value) = *((int*)value);
				break;
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

int model_get_material_param(struct Model* model, const char* name, void* value_out)
{
	assert(model && name && value_out);
	int success = 0;
	struct Material* material = material_get(model->material);
	for(int i = 0; i < array_len(model->material_params); i++)
	{
		struct Material_Param* param = &model->material_params[i];
		struct Uniform* uniform = &material->model_params[param->uniform_index];
		if(strcmp(uniform->name, name) == 0)
		{
			switch(uniform->type)
			{
			case UT_INT:
				*((int*)value_out) = *((int*)param->value);
				break;
			case UT_FLOAT:
				*((float*)value_out) = *((float*)param->value);
				break;
			case UT_VEC2:
				vec2_assign((vec2*)value_out, (vec2*)param->value);
				break;
			case UT_VEC3:
				vec3_assign((vec3*)value_out, (vec3*)param->value);
				break;
			case UT_VEC4:
				vec4_assign((vec4*)value_out, (vec4*)param->value);
				break;
			case UT_MAT4:
				mat4_assign((mat4*)value_out, (mat4*)param->value);
				break;
			}
			break; /* break for */
			success = 1;
		}
	}
	return success;
}
