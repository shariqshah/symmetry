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
#include "variant.h"
#include "file_io.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define MAX_ENTITY_PROP_NAME_LEN 128
#define MAX_ENTITY_PROP_LEN      256
#define MAX_LINE_LEN             512

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
	entity->editor_selected     = 0;
	entity->renderable          = false;
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
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
		index = array_len(entity_list) - 1;
	}
	
	strncpy(new_entity->name, name ? name : "DEFAULT_ENTITY_NAME", MAX_ENTITY_NAME_LEN);
	new_entity->name[MAX_ENTITY_NAME_LEN - 1] = '\0';
	new_entity->id                  = index;
	new_entity->is_listener         = false;
	new_entity->type                = type;
	new_entity->marked_for_deletion = false;
	new_entity->renderable          = false;
	new_entity->editor_selected     = 0;
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
		if(curr_ent->id == -1)
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
	fprintf(entity_file, "is_listener: %s\n", entity->is_listener ? "true" : "false");
	fprintf(entity_file, "renderable: %s\n", entity->renderable ? "true" : "false");

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
		fprintf(entity_file, "ortho: %s\n", entity->camera.ortho ? "true" : "false");
		fprintf(entity_file, "resizeable: %s\n", entity->camera.resizeable ? "true" : "false");
		fprintf(entity_file, "fov: %.5f\n", entity->camera.fov);
		fprintf(entity_file, "nearz: %.5f\n", entity->camera.nearz);
		fprintf(entity_file, "farz: %.5f\n", entity->camera.farz);
		fprintf(entity_file, "render_texture: %s\n", entity->camera.render_tex == -1 ? "false" : "true");
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
		fprintf(entity_file, "light_type: %d\n", entity->light.valid);
		fprintf(entity_file, "outer_angle: %.5f\n", entity->light.outer_angle);
		fprintf(entity_file, "inner_angle: %.5f\n", entity->light.inner_angle);
		fprintf(entity_file, "falloff: %.5f\n", entity->light.falloff);
		fprintf(entity_file, "radius: %d\n", entity->light.radius);
		fprintf(entity_file, "intensity: %.5f\n", entity->light.intensity);
		fprintf(entity_file, "depth_bias: %.5f\n", entity->light.depth_bias);
		fprintf(entity_file, "valid: %s\n", entity->light.valid ? "true" : "false");
		fprintf(entity_file, "cast_shadow: %s\n", entity->light.cast_shadow ? "true" : "false");
		fprintf(entity_file, "pcf_enabled: %s\n", entity->light.pcf_enabled ? "true" : "false");
		fprintf(entity_file, "color: %.5f %.5f %.5f",
				entity->light.color.x,
				entity->light.color.y,
				entity->light.color.z);
		break;
	}
	case ET_SOUND_SOURCE:
	{
		fprintf(entity_file, "active: %s\n", entity->sound_source.active ? "true" : "false");
		fprintf(entity_file, "relative: %s\n", entity->sound_source.relative ? "true" : "false");
		break;
	}
	};

	fprintf(entity_file, "\n");
	log_message("Entity %s written to %s", entity->name, filename);
	success = true;
	fclose(entity_file);
	return success;
}

struct Entity* entity_load(const char* filename, int directory_type)
{
	FILE* entity_file = io_file_open(directory_type, filename, "r");
	if(!entity_file)
	{
		log_error("entity:load", "Failed to open entity file %s for writing", filename);
		return NULL;
	}

	struct Entity entity =
    {
		.id                  = -1,
		.type                = ET_NONE,
		.is_listener         = false,
		.renderable          = false,
		.marked_for_deletion = false,
		.name                = "DEFAULT_ENTITY_NAME",
		.editor_selected     = 0
    };
	
    int   current_line  = 0;
	char* material_name = NULL;
	char* entity_name   = NULL;
	char* geometry_name = NULL;
	char* parent_name   = NULL;
    char line_buffer[MAX_LINE_LEN];
    char prop_str[MAX_ENTITY_PROP_NAME_LEN];
	static struct Variant var_value = { .type = VT_NONE};

    variant_free(&var_value);
	memset(prop_str, '\0', MAX_ENTITY_PROP_NAME_LEN);
	memset(line_buffer, '\0', MAX_LINE_LEN);

	while(fgets(line_buffer, MAX_LINE_LEN -1, entity_file))
	{
		current_line++;
		memset(prop_str, '\0', MAX_ENTITY_PROP_NAME_LEN);

		if(line_buffer[0] == '#') continue;
		if(strlen(line_buffer) == 0) break;

		char* value_str = strstr(line_buffer, ":");
		if(!value_str)
		{
			log_warning("Malformed value in entity file %s, line %d", filename, current_line);
			continue;
		}

		value_str++; /* Ignore the colon(:) and set the pointer after it */
		
		if(sscanf(line_buffer, " %1024[^: ] : %*s", prop_str) != 1)
		{
			log_warning("Unable to read property name in entity file %s, line %d", filename, current_line);
			continue;
		}

		/* Common entity properties */
		if(strncmp("name", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_STR);
			entity_name = str_new(var_value.val_str);
			//variant_copy_out(&entity.name, &var_value);
		}
		if(strncmp("parent", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_STR);
			parent_name = str_new(var_value.val_str);
			//variant_copy_out(&entity.name, &var_value);
		}
		else if(strncmp("type", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_INT);
			variant_copy_out(&entity.type, &var_value);
		}
		else if(strncmp("is_listener", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.is_listener, &var_value);
		}
		else if(strncmp("renderable", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.renderable, &var_value);
		}
		
		/* Transform */
		else if(strncmp("position", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_VEC3);
			variant_copy_out(&entity.transform.position, &var_value);
		}
		else if(strncmp("scale", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_VEC3);
			variant_copy_out(&entity.transform.scale, &var_value);
		}
		else if(strncmp("rotation", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_QUAT);
			variant_copy_out(&entity.transform.rotation, &var_value);
		}

		/* Camera */
		else if(strncmp("ortho", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.camera.ortho, &var_value);
		}
		else if(strncmp("resizeable", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.camera.resizeable, &var_value);
		}
		else if(strncmp("fov", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.camera.fov, &var_value);
		}
		else if(strncmp("nearz", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.camera.nearz, &var_value);
		}
		else if(strncmp("farz", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.camera.farz, &var_value);
		}
		else if(strncmp("render_texture", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.camera.fbo, &var_value);
		}

		/* Light */
		else if(strncmp("light_type", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_INT);
			variant_copy_out(&entity.light.type, &var_value);
		}
		else if(strncmp("outer_angle", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.light.outer_angle, &var_value);
		}
		else if(strncmp("inner_angle", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.light.inner_angle, &var_value);
		}
		else if(strncmp("falloff", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.light.falloff, &var_value);
		}
		else if(strncmp("radius", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_INT);
			variant_copy_out(&entity.light.radius, &var_value);
		}
		else if(strncmp("intensity", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.light.intensity, &var_value);
		}
		else if(strncmp("depth_bias", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_FLOAT);
			variant_copy_out(&entity.light.depth_bias, &var_value);
		}
		else if(strncmp("valid", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.light.valid, &var_value);
		}
		else if(strncmp("cast_shadow", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.light.cast_shadow, &var_value);
		}
		else if(strncmp("pcf_enabled", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.light.pcf_enabled, &var_value);
		}
		else if(strncmp("color", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_VEC3);
			variant_copy_out(&entity.light.color, &var_value);
		}

		/* Model */
		else if(strncmp("material", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_STR);
			material_name = str_new(var_value.val_str);
		}
		else if(strncmp("geometry", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_STR);
			geometry_name = str_new(var_value.val_str);
		}

		/* Sound Source */
		else if(strncmp("active", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.sound_source.active, &var_value);
		}
		else if(strncmp("relative", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
		{
			variant_from_str(&var_value, value_str, VT_BOOL);
			variant_copy_out(&entity.sound_source.relative, &var_value);
		}

		variant_free(&var_value);
	}

	/* Do the things after assignment */
	struct Entity* parent_entity = entity_find(parent_name);
	struct Entity* new_entity = entity_create(entity_name, entity.type, parent_entity ? parent_entity->id : -1);
	free(entity_name);
	transform_translate(new_entity, &entity.transform.position, TS_PARENT);
	quat_assign(&new_entity->transform.rotation, &entity.transform.rotation);
	transform_scale(new_entity, &entity.transform.scale);
	
	if(entity.is_listener) sound_listener_set(new_entity->id);
	if(entity.renderable)  new_entity->renderable = true;
	
	switch(new_entity->type)
	{
	case ET_CAMERA:
		camera_update_view(new_entity);
		camera_update_proj(new_entity);
		break;
	case ET_STATIC_MESH:
		model_create(new_entity, geometry_name, material_name);
		free(geometry_name);
		free(material_name);
		break;
	case ET_LIGHT:
		memcpy(&new_entity->light, &entity.light, sizeof(struct Light));
		light_add(new_entity);
		break;
	case ET_SOUND_SOURCE:
		sound_source_create(new_entity, new_entity->sound_source.relative);
		break;
	};
	
	log_message("Entity %s loaded from %s", new_entity->name, filename);
	fclose(entity_file);
	return new_entity;
}

const char* entity_type_name_get(struct Entity* entity)
{
	const char* typename = "NONE";
	switch(entity->type)
	{
	case ET_NONE:         typename = "None";         break;
	case ET_CAMERA:       typename = "Camera";       break;
	case ET_LIGHT:        typename = "Light";        break;
	case ET_PLAYER:       typename = "Player";       break;
	case ET_ROOT:         typename = "Root";         break;
	case ET_SOUND_SOURCE: typename = "Sound Source"; break;
	case ET_STATIC_MESH:  typename = "Static Mesh";  break;
	default:              typename = "Unknown";      break;
	};
	return typename;
}
