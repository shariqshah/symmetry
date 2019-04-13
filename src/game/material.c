#include "material.h"
#include "../common/array.h"
#include "shader.h"
#include "entity.h"
#include "../common/string_utils.h"
#include "../common/log.h"
#include "texture.h"
#include "light.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

bool material_init(struct Material* material, int material_type)
{
	assert(material && material_type > -1 && material_type < MAT_MAX);

	material->type = material_type;
	memset(material->registered_static_meshes, '\0', sizeof(struct Static_Mesh*) * MAX_MATERIAL_REGISTERED_STATIC_MESHES);
	memset(material->model_params, 0, sizeof(struct Uniform) * MMP_MAX);
	memset(material->pipeline_params, 0, sizeof(struct Uniform) * MPP_MAX);

	switch(material_type)
	{
	case MAT_BLINN:
	{
		material->lit  = true;
		material->shader = shader_create("blinn_phong.vert", "blinn_phong.frag");

		material->pipeline_params[MPP_INV_MODEL_MAT].type = UT_MAT4;
		material->pipeline_params[MPP_INV_MODEL_MAT].location = shader_get_uniform_location(material->shader, "inv_model_mat");

		material->pipeline_params[MPP_VIEW_MAT].type = UT_MAT4;
		material->pipeline_params[MPP_VIEW_MAT].location = shader_get_uniform_location(material->shader, "view_mat");

		material->pipeline_params[MPP_CAM_POS].type = UT_VEC3;
		material->pipeline_params[MPP_CAM_POS].location = shader_get_uniform_location(material->shader, "camera_pos");

		material->pipeline_params[MPP_TOTAL_LIGHTS].type = UT_INT;
		material->pipeline_params[MPP_TOTAL_LIGHTS].location = shader_get_uniform_location(material->shader, "total_active_lights");

		material->model_params[MMP_DIFFUSE_TEX].type = UT_TEX;
		material->model_params[MMP_DIFFUSE_TEX].location = shader_get_uniform_location(material->shader, "diffuse_texture");

		material->model_params[MMP_DIFFUSE_COL].type = UT_VEC4;
		material->model_params[MMP_DIFFUSE_COL].location = shader_get_uniform_location(material->shader, "diffuse_color");

		material->model_params[MMP_DIFFUSE].type = UT_FLOAT;
		material->model_params[MMP_DIFFUSE].location = shader_get_uniform_location(material->shader, "diffuse");

		material->model_params[MMP_SPECULAR].type = UT_FLOAT;
		material->model_params[MMP_SPECULAR].location = shader_get_uniform_location(material->shader, "specular");

		material->model_params[MMP_SPECULAR_STRENGTH].type = UT_FLOAT;
		material->model_params[MMP_SPECULAR_STRENGTH].location = shader_get_uniform_location(material->shader, "specular_strength");
	}
	break;
	case MAT_UNSHADED:
	{
		material->lit = false;
		material->shader = shader_create("unshaded.vert", "unshaded.frag");

		material->model_params[MMP_DIFFUSE_TEX].type = UT_TEX;
		material->model_params[MMP_DIFFUSE_TEX].location = shader_get_uniform_location(material->shader, "diffuse_texture");

		material->model_params[MMP_DIFFUSE_COL].type = UT_VEC4;
		material->model_params[MMP_DIFFUSE_COL].location = shader_get_uniform_location(material->shader, "diffuse_color");
	};
	break;
	default:
		log_error("material:init", "Invalid material type");
		return false;
	}

	// Setup common pipeline parameters
	material->pipeline_params[MPP_FOG_MODE].type = UT_INT;
	material->pipeline_params[MPP_FOG_MODE].location = shader_get_uniform_location(material->shader, "fog.mode");

	material->pipeline_params[MPP_FOG_DENSITY].type = UT_FLOAT;
	material->pipeline_params[MPP_FOG_DENSITY].location = shader_get_uniform_location(material->shader, "fog.density");

	material->pipeline_params[MPP_FOG_START_DIST].type = UT_FLOAT;
	material->pipeline_params[MPP_FOG_START_DIST].location = shader_get_uniform_location(material->shader, "fog.start_dist");

	material->pipeline_params[MPP_FOG_MAX_DIST].type = UT_FLOAT;
	material->pipeline_params[MPP_FOG_MAX_DIST].location = shader_get_uniform_location(material->shader, "fog.max_dist");

	material->pipeline_params[MPP_FOG_COLOR].type = UT_VEC3;
	material->pipeline_params[MPP_FOG_COLOR].location = shader_get_uniform_location(material->shader, "fog.color");

	material->pipeline_params[MPP_MVP].type = UT_MAT4;
	material->pipeline_params[MPP_MVP].location = shader_get_uniform_location(material->shader, "mvp");

	material->pipeline_params[MPP_MODEL_MAT].type = UT_MAT4;
	material->pipeline_params[MPP_MODEL_MAT].location = shader_get_uniform_location(material->shader, "model_mat");

	material->pipeline_params[MPP_AMBIENT_LIGHT].type = UT_VEC3;
	material->pipeline_params[MPP_AMBIENT_LIGHT].location = shader_get_uniform_location(material->shader, "ambient_light");

	return true;
}

void material_reset(struct Material* material)
{
	assert(material);

	material->type = -1;
	memset(material->registered_static_meshes, '\0', sizeof(struct Static_Mesh*) * MAX_MATERIAL_REGISTERED_STATIC_MESHES);
	memset(material->model_params, 0, sizeof(struct Uniform) * MMP_MAX);
	memset(material->pipeline_params, 0, sizeof(struct Uniform) * MPP_MAX);
}

bool material_register_static_mesh(struct Material* material, struct Static_Mesh* mesh)
{
	assert(material);

	for(int i = 0; i < MAX_MATERIAL_REGISTERED_STATIC_MESHES; i++)
	{
		if(!material->registered_static_meshes[i])
		{
			material->registered_static_meshes[i] = mesh;

			// Set default values for instance parameters
			switch(material->type)
			{
			case MAT_BLINN:
			{
				variant_assign_vec4f(&mesh->model.material_params[MMP_DIFFUSE_COL], 1.f, 0.f, 1.f, 1.f);
				variant_assign_float(&mesh->model.material_params[MMP_DIFFUSE], 1.f);
				variant_assign_int(&mesh->model.material_params[MMP_DIFFUSE_TEX], texture_create_from_file("default.tga", TU_DIFFUSE));
				variant_assign_float(&mesh->model.material_params[MMP_SPECULAR], 1.f);
				variant_assign_float(&mesh->model.material_params[MMP_SPECULAR_STRENGTH], 50.f);
			}
			break;
			case MAT_UNSHADED:
			{
				variant_assign_vec4f(&mesh->model.material_params[MMP_DIFFUSE_COL], 1.f, 0.f, 1.f, 1.f);
				variant_assign_int(&mesh->model.material_params[MMP_DIFFUSE_TEX], texture_create_from_file("default.tga", TU_DIFFUSE));
			}
			break;
			default:
				log_error("material:register_model", "Invalid material type");
				break;
			}
			return true;
		}
	}

	return false;
}

void material_unregister_static_mesh(struct Material* material, struct Static_Mesh* mesh)
{
	assert(material);

	for(int i = 0; i < MAX_MATERIAL_REGISTERED_STATIC_MESHES; i++)
	{
		if(material->registered_static_meshes[i] == mesh)
		{
			material->registered_static_meshes[i] = NULL;
			for(int i = 0; i < MMP_MAX; i++)
				variant_free(&mesh->model.material_params[i]);
			break;
		}
	}
}