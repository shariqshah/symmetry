#include "entity.h"
#include "array.h"
#include "log.h"
#include "string_utils.h"
#include "transform.h"
#include "camera.h"
#include "light.h"
#include "model.h"
#include "sound.h"
#include "material.h"
#include "geometry.h"
#include "file_io.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

static struct Entity* entity_list;
static int* empty_indices;

void entity_init(void)
{
	entity_list   = array_new(struct Entity);
	empty_indices = array_new(int);
}

void entity_cleanup(void)
{
	if(array_len(empty_indices) < array_len(entity_list))
	{
		for(int i = 0; i < array_len(entity_list); i++)
			entity_remove(i); 
	}

	array_free(entity_list);
	array_free(empty_indices);
}

void entity_remove(int index)
{
	struct Entity* entity = &entity_list[index];

	transform_destroy(entity);
	switch(entity->type)
	{
	case ET_CAMERA:       camera_destroy(entity);       break;
	case ET_LIGHT:        light_destroy(entity);        break;
	case ET_SOUND_SOURCE: sound_source_destroy(entity); break;
	case ET_STATIC_MESH:  model_destroy(entity);        break;
	case ET_ROOT: break;
	default: log_error("entity:remove", "Invalid entity type"); break;
	};
	entity->id                  = -1;
	entity->is_listener         = false;
	entity->marked_for_deletion = false;
	entity->renderable          = false;
	entity->name                = NULL;
	free(entity->name);
	array_push(empty_indices, index, int);
}

struct Entity* entity_create(const char* name, const int type, int parent_id)
{
	struct Entity* new_entity = NULL;
	int index = -1;
	
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		array_pop(empty_indices);
		new_entity = &entity_list[index];
	}
	else
	{
		new_entity = array_grow(entity_list, struct Entity);
		new_entity->name = NULL;
		index = array_len(entity_list) - 1;
	}
	
	if(new_entity->name) free(new_entity->name);
	new_entity->name                = name ? str_new(name) : str_new("DEFAULT_NAME");
	new_entity->id                  = index;
	new_entity->is_listener         = false;
	new_entity->type                = type;
	new_entity->marked_for_deletion = false;
	new_entity->renderable          = false;
	transform_create(new_entity, parent_id);
	return new_entity;	   
}

struct Entity* entity_get(int index)
{
	struct Entity* entity = NULL;
	if(index >= 0 && index < array_len(entity_list))
		entity = &entity_list[index];
	return entity;
}

struct Entity* entity_find(const char* name)
{
	/* Bruteforce search all entities and return the first match */
	struct Entity* entity = NULL;
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* curr_ent = &entity_list[i];
		if(!curr_ent->name)
			continue;
		if(strcmp(curr_ent->name, name) == 0)
		{
			entity = curr_ent;
			break;
		}
	}
	return entity; 
}

void entity_post_update(void)
{
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* entity = &entity_list[i];
		if(entity->id == -1) continue;
		
		if(entity->marked_for_deletion)
		{
			entity_remove(i);
			continue;
		}
	}
	
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* entity = &entity_list[i];
		if(entity->id == -1) continue;

		if(entity->transform.is_modified)
		{
			if(entity->type == ET_CAMERA)
				camera_update_view(entity);
			else if(entity->type == ET_SOUND_SOURCE)
				sound_source_update(entity);

			if(entity->is_listener) sound_listener_update();
		}	
	}
}

struct Entity* entity_get_all(void)
{
	return entity_list;
}

struct Entity* entity_get_parent(int node)
{
	struct Entity* parent = NULL;
	struct Entity* entity = entity_get(node);
	if(entity) parent = entity_get(entity->transform.parent);
	return parent;
}

bool entity_save(struct Entity* entity, const char* filename, int directory_type)
{
	bool success = false;
	FILE* entity_file = io_file_open(directory_type, filename, "w");
	if(!entity_file)
	{
		log_error("entity:save", "Failed to open entity file %s for writing");
		return success;
	}

	/* First write all properties common to all entity types */
	fprintf(entity_file, "name: %s\n", entity->name);
	fprintf(entity_file, "type: %d\n", entity->type);
	fprintf(entity_file, "is_listener: %d\n", entity->is_listener);
	fprintf(entity_file, "renderable: %d\n", entity->renderable);

	struct Entity* parent = entity_get_parent(entity->id);
	fprintf(entity_file, "parent: %s\n", parent->name);

	/* Transform */
	fprintf(entity_file, "position: %.5f %.5f %.5f\n",
			entity->transform.position.x,
			entity->transform.position.y,
			entity->transform.position.z);
	fprintf(entity_file, "scale: %.5f %.5f %.5f\n",
			entity->transform.scale.x,
			entity->transform.scale.y,
			entity->transform.scale.z);
	fprintf(entity_file, "rotation: %.5f %.5f %.5f %.5f\n",
			entity->transform.rotation.x,
			entity->transform.rotation.y,
			entity->transform.rotation.z,
			entity->transform.rotation.w);

	switch(entity->type)
	{
	case ET_CAMERA:
	{
		fprintf(entity_file, "ortho: %d\n", entity->camera.ortho);
		fprintf(entity_file, "resizeable: %d\n", entity->camera.resizeable);
		fprintf(entity_file, "fov: %.5f\n", entity->camera.fov);
		fprintf(entity_file, "nearz: %.5f\n", entity->camera.nearz);
		fprintf(entity_file, "farz: %.5f\n", entity->camera.farz);
		fprintf(entity_file, "render_texture: %d\n", entity->camera.render_tex == -1 ? 0 : 1);
		break;
	}
	case ET_STATIC_MESH:
	{
		/* TODO: Change this after adding proper support for exported models from blender */
		struct Material* material = material_get(entity->model.material);
		struct Geometry* geom = geom_get(entity->model.geometry_index);
		fprintf(entity_file, "material: %s\n", material->name);
		fprintf(entity_file, "geometry: %s\n", geom->filename);
		break;
	}
	case ET_LIGHT:
	{
		fprintf(entity_file, "type: %df\n", entity->light.valid);
		fprintf(entity_file, "outer_angle: %.5f\n", entity->light.outer_angle);
		fprintf(entity_file, "inner_angle: %.5f\n", entity->light.inner_angle);
		fprintf(entity_file, "falloff: %.5f\n", entity->light.falloff);
		fprintf(entity_file, "radius: %d\n", entity->light.radius);
		fprintf(entity_file, "intensity: %.5f\n", entity->light.intensity);
		fprintf(entity_file, "depth_bias: %.5f\n", entity->light.depth_bias);
		fprintf(entity_file, "valid: %df\n", entity->light.valid);
		fprintf(entity_file, "cast_shadow: %df\n", entity->light.cast_shadow);
		fprintf(entity_file, "pcf_enabled: %df\n", entity->light.pcf_enabled);
		fprintf(entity_file, "color: %.5f %.5f %.5f",
				entity->light.color.x,
				entity->light.color.y,
				entity->light.color.z);
		break;
	}
	case ET_SOUND_SOURCE:
	{
		fprintf(entity_file, "active: %df\n", entity->sound_source.active);
		fprintf(entity_file, "relative: %df\n", entity->sound_source.relative);
		break;
	}
	};

	fprintf(entity_file, "\n");
	log_message("Entity %s written to %s", entity->name, filename);
	success = true;
	fclose(entity_file);
	return success;
}
