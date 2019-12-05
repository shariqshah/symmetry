#include "model.h"
#include "../common/array.h"
#include "../common/log.h"
#include "game.h"
#include "renderer.h"
#include "entity.h"
#include "texture.h"
#include "material.h"
#include "geometry.h"
#include "shader.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

void model_init(struct Model* model, struct Static_Mesh* mesh, const char* geometry_name, int material_type)
{
	assert(model && material_type > -1 && material_type < MAT_MAX);

	/* if no name is given for geometry, use default */
	int geo_index = geom_create_from_file(geometry_name ? geometry_name : "default.symbres");

	if(!model_geometry_set(model, geometry_name))
		return;

	struct Material* material = &game_state_get()->renderer->materials[material_type];
	if(!material_register_static_mesh(material, mesh))
	{
		log_error("model:create", "Unable to register model with Unshaded material");
		model_reset(model, mesh);
	}
}

bool model_geometry_set(struct Model* model, const char* geometry_name)
{
	/* if no name is given for geometry, use default */
	int geo_index = geom_create_from_file(geometry_name ? geometry_name : "default.symbres");
	if(geo_index == -1)
	{
		log_error("model:geometry_set", "Failed to load model %s", geometry_name);
		geo_index = geom_create_from_file("default.symbres");
		if(geo_index == -1)
		{
			log_error("model:geometry_set", "Could not load default model 'default.symbres' ");
			return false;
		}
	}
	model->geometry_index = geo_index;
	return true;
}

void model_reset(struct Model* model, struct Static_Mesh* mesh)
{
	assert(model);
	geom_remove(model->geometry_index);
	model->geometry_index = -1;
	struct Renderer* renderer = game_state_get()->renderer;
	if(model->material)
	{
		struct Material* material = &renderer->materials[model->material->type];
		material_unregister_static_mesh(material, mesh);
	}
}