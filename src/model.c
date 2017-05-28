#include "model.h"
#include "array.h"
#include "log.h"
#include "entity.h"
#include "shader.h"
#include "transform.h"
#include "texture.h"
#include "renderer.h"
#include "material.h"
#include "light.h"
#include "editor.h"
#include "geometry.h"
#include "variant.h"
#include "bounding_volumes.h"
#include "gl_load.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_NAME_LEN 64

static int num_culled = 0, num_rendered = 0, num_indices = 0;
static int num_culled_slot = -1, num_rendered_slot = -1, num_indices_slot = -1;

void model_init(void)
{
	num_culled_slot   = editor_debugvar_slot_create("Culled Geom",   VT_INT);
	num_rendered_slot = editor_debugvar_slot_create("Rendered Geom", VT_INT);
	num_indices_slot  = editor_debugvar_slot_create("Total Indices", VT_INT);
}

void model_create(struct Model* model, int entity_id, const char* geo_name, const char* material_name)
{
	assert(model);
	/* if no name is given for geometry, use default */
	if(!geo_name) geo_name = "default.pamesh";
	int geo_index = geom_create_from_file(geo_name);

	model->geometry_index = geo_index;
	if(!material_register_model(model, entity_id, material_name ? material_name : "Unshaded"))
	{
		log_error("model:create", "Unable to register model with Unshaded material, component not added");
		model_destroy(model, entity_id);
	}
}

void model_destroy(struct Model* model, int entity_id)
{
	assert(model);
	geom_remove(model->geometry_index);
	model->geometry_index = -1;
	material_unregister_model(model, entity_id);
	/* deallocate all params */
	for(int i = 0; i < array_len(model->material_params); i++)
		free(model->material_params[i].value);

	array_free(model->material_params);
}

void model_cleanup(void)
{

}

void model_render_all(struct Entity* camera_entity, int draw_mode)
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
			struct Entity*    entity    = entity_get(material->registered_models[i]);
			struct Model*     model     = &entity->model;
			struct Transform* transform = &entity->transform;

			/* set material params for the model */
			for(int k = 0; k < array_len(model->material_params); k++)
			{
				struct Material_Param* param = &model->material_params[k];
				struct Uniform* uniform = &material->model_params[param->uniform_index];
			    shader_set_uniform(uniform->type, uniform->location, param->value);
				renderer_check_glerror("model:render_all:material_param");
			}

			/* Set pipeline uniforms */
			struct Render_Settings* render_settings = renderer_settings_get();
			for(int k = 0; k < array_len(material->pipeline_params); k++)
			{
				/* TODO: change this into something better */
				struct Uniform* uniform = &material->pipeline_params[k];
				if(strcmp(uniform->name, "mvp") == 0)
				{
					mat4_identity(&mvp);
					mat4_mul(&mvp, &camera_entity->camera.view_proj_mat, &transform->trans_mat);
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
					shader_set_uniform(uniform->type, uniform->location, &camera_entity->camera.view_mat);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "inv_model_mat") == 0)
				{
					mat4 inv_mat;
					mat4_identity(&inv_mat);
					mat4_inverse(&inv_mat, &transform->trans_mat);
					shader_set_uniform(uniform->type, uniform->location, &inv_mat);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "fog.mode") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->fog.mode);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "fog.density") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->fog.density);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "fog.start_dist") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->fog.start_dist);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "fog.max_dist") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->fog.max_dist);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "fog.color") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->fog.color);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
				else if(strcmp(uniform->name, "ambient_light") == 0)
				{
					shader_set_uniform(uniform->type, uniform->location, &render_settings->ambient_light);
					renderer_check_glerror("model:render_all:material_pipeline");
				}
			}

			if(material->lit)	/* Set light information */
			{
				int valid_light_count = 0;
				int* light_index_list = light_get_valid_indices(&valid_light_count);
				char uniform_name[MAX_NAME_LEN];
				memset(uniform_name, '\0', MAX_NAME_LEN);
				for(int i = 0; i < valid_light_count; i++)
				{
					struct Entity*    light_entity = entity_get(light_index_list[i]);
					struct Light*     light        = &light_entity->light; /* TODO: Cull lights according to camera frustum */
					struct Transform* light_transform    = &light_entity->transform;
					vec3 light_pos = {0, 0, 0};
					transform_get_absolute_pos(light_transform, &light_pos);

					if(light->type != LT_POINT)
					{
						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].direction", i);
						transform_get_absolute_lookat(light_transform, &light_pos);
						vec3_norm(&light_pos, &light_pos);
						shader_set_uniform_vec3(material->shader, uniform_name, &light_pos);
						memset(uniform_name, '\0', MAX_NAME_LEN);
					}


					if(light->type != LT_DIR)
					{
						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].position", i);
						shader_set_uniform_vec3(material->shader,  uniform_name, &light_pos);
						memset(uniform_name, '\0', MAX_NAME_LEN);

						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].outer_angle", i);
						shader_set_uniform_float(material->shader, uniform_name, light->outer_angle);
						memset(uniform_name, '\0', MAX_NAME_LEN);
					
						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].inner_angle", i);
						shader_set_uniform_float(material->shader, uniform_name, light->inner_angle);
						memset(uniform_name, '\0', MAX_NAME_LEN);
					
						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].falloff", i);
						shader_set_uniform_float(material->shader, uniform_name, light->falloff);
						memset(uniform_name, '\0', MAX_NAME_LEN);

						snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].radius", i);
						shader_set_uniform_int(material->shader, uniform_name, light->radius);
						memset(uniform_name, '\0', MAX_NAME_LEN);
					}
					
					snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].color", i);
					shader_set_uniform_vec3(material->shader,  uniform_name, &light->color);
					memset(uniform_name, '\0', MAX_NAME_LEN);
					
					snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].intensity", i);
					shader_set_uniform_float(material->shader, uniform_name, light->intensity);
					memset(uniform_name, '\0', MAX_NAME_LEN);
					
					snprintf(uniform_name, MAX_NAME_LEN, "lights[%d].type", i);
					shader_set_uniform_int(material->shader, uniform_name, light->type);
					memset(uniform_name, '\0', MAX_NAME_LEN);
				}

				shader_set_uniform_int(material->shader, "total_active_lights", valid_light_count);
				struct Transform* camera_tran = &camera_entity->transform;
				vec3 camera_pos = {0, 0, 0};
				transform_get_absolute_pos(camera_tran, &camera_pos);
				shader_set_uniform_vec3(material->shader, "camera_pos", &camera_pos);
			}
			
			/* Render the geometry */
			int indices = geom_render_in_frustum(model->geometry_index, &camera_entity->camera.frustum[0], transform, draw_mode);
			if(indices > 0)
			{
				num_rendered++;
				num_indices += indices;
			}
			else
			{
				num_culled++;
			}
			//geom_render(model->geometry_index, draw_mode);

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
	editor_debugvar_slot_set_int(num_rendered_slot, num_rendered);
	editor_debugvar_slot_set_int(num_culled_slot, num_culled);
	editor_debugvar_slot_set_int(num_indices_slot, num_indices);

	num_culled = num_rendered = num_indices = 0;
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

int model_get_material_param(struct Model* model, const char* name, void* value_out)
{
	assert(model && name && value_out);
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

void model_render_all_debug(struct Entity* camera_entity,
							int            debug_shader,
							int            draw_mode,
							const vec4*    debug_color)
{
	assert(debug_shader > -1);
	shader_bind(debug_shader);
	{
		static mat4 mvp;
		shader_set_uniform_vec4(debug_shader, "debug_color", debug_color);
		struct Entity* entity_list = entity_get_all();
		for(int i = 0; i < array_len(entity_list); i++)
		{
			if(!entity_list[i].renderable) continue;
			struct Model*     model     = &entity_list[i].model;
			struct Transform* transform = &entity_list[i].transform;
			int               geometry  = model->geometry_index;
			mat4_identity(&mvp);
			mat4_mul(&mvp, &camera_entity->camera.view_proj_mat, &transform->trans_mat);
			shader_set_uniform_mat4(debug_shader, "mvp", &mvp);
			geom_render(geometry, draw_mode);
		}
	}
	shader_unbind();
}
