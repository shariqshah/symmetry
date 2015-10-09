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

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <assert.h>

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

int model_create(int node, const char* geo_name)
{
	assert(geo_name);
	int geo_index = geom_create(geo_name);
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
		new_model->shader = 0;	/* Temporary, for test run only till materials are added */
		vec4_fill(&new_model->color, 0.7f, 0.7f, 0.5f, 1.f);
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
		model->shader = -1;
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
	int texture = texture_find("test_comp.tga");
	mat4 mvp;
	for(int i = 0; i < array_len(model_list); i++)
	{
		struct Model* model = &model_list[i];
		struct Entity* entity = entity_get(model->node);
		struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
		mat4_identity(&mvp);
		
		shader_bind(model->shader);
		shader_set_uniform_int(model->shader, "sampler", (GL_TEXTURE0 + 4) - GL_TEXTURE0);
		texture_bind(texture, 4);
		renderer_check_glerror("model:render_all");
		mat4_mul(&mvp, &camera->view_proj_mat, &transform->trans_mat);
		shader_set_uniform_mat4(model->shader, "mvp", &mvp);
		shader_set_uniform_vec4(model->shader, "diffuseColor", &model->color);
		geom_render(model->geometry_index);
		texture_unbind(4);
		shader_unbind();
	}
}
