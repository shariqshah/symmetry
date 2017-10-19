#include "entity.h"
#include "../common/array.h"
#include "../common/log.h"
#include "../common/string_utils.h"
#include "transform.h"
#include "camera.h"
#include "light.h"
#include "model.h"
#include "material.h"
#include "geometry.h"
#include "framebuffer.h"
#include "scene.h"
#include "../common/variant.h"
#include "../common/common.h"
#include "../common/parser.h"
#include "../common/hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

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
	case ET_STATIC_MESH:  model_destroy(entity);        break;
    case ET_SOUND_SOURCE:
    {
        
    }
    break;
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
            {
				camera_update_view(entity);
            }
			else if(entity->type == ET_SOUND_SOURCE)
            {
                vec3 abs_pos = {0.f, 0.f,  0.f};
                vec3 abs_fwd = {0.f, 0.f, -1.f};
                vec3 abs_up  = {0.f, 1.f, 0.f};
                transform_get_absolute_pos(entity, &abs_pos);
                transform_get_absolute_forward(entity, &abs_fwd);
                transform_get_absolute_up(entity, &abs_up);

                platform->sound.source_update(entity->sound_source.source_handle, abs_pos.x, abs_pos.y, abs_pos.z);
            }

            if(entity->is_listener)
            {
                vec3 abs_pos = {0.f, 0.f,  0.f};
                vec3 abs_fwd = {0.f, 0.f, -1.f};
                vec3 abs_up  = {0.f, 1.f, 0.f};
                transform_get_absolute_pos(entity, &abs_pos);
                transform_get_absolute_forward(entity, &abs_fwd);
                transform_get_absolute_up(entity, &abs_up);

                platform->sound.listener_update(abs_pos.x, abs_pos.y, abs_pos.z,
                                                abs_fwd.x, abs_fwd.y, abs_fwd.z,
                                                abs_up.x,  abs_up.y,  abs_up.z);
            }
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

bool entity_write(struct Entity* entity, struct Parser_Object* object)
{
	if(!object)
	{
		log_error("entity:write", "Invalid object");
		return false;
	}

	/* First write all properties common to all entity types */
	struct Hashmap* entity_data = object->data;

	hashmap_str_set(entity_data, "name", entity->name);
	hashmap_int_set(entity_data, "type", entity->type);
	hashmap_bool_set(entity_data, "is_listener", entity->is_listener);
	hashmap_bool_set(entity_data, "renderable", entity->renderable);

	struct Entity* parent = entity_get_parent(entity->id);
	hashmap_str_set(entity_data, "parent", parent ? parent->name  : "NONE");

	/* Transform */
	hashmap_vec3_set(entity_data, "position", &entity->transform.position);
	hashmap_vec3_set(entity_data, "scale", &entity->transform.scale);
	hashmap_quat_set(entity_data, "rotation", &entity->transform.rotation);
	switch(entity->type)
	{
	case ET_CAMERA:
	{
		struct Camera* camera = &entity->camera;
		hashmap_bool_set(entity_data, "ortho", camera->ortho);
		hashmap_bool_set(entity_data, "resizeable", camera->resizeable);
		hashmap_float_set(entity_data, "fov", camera->fov);
		hashmap_float_set(entity_data, "nearz", camera->nearz);
		hashmap_float_set(entity_data, "farz", camera->farz);
		hashmap_vec4_set(entity_data, "clear_color", &camera->clear_color);
		if(entity->camera.fbo != -1)
		{
			hashmap_bool_set(entity_data, "has_fbo", true);
			hashmap_int_set(entity_data, "fbo_height", framebuffer_height_get(camera->fbo));
			hashmap_int_set(entity_data, "fbo_width", framebuffer_width_get(camera->fbo));
			hashmap_bool_set(entity_data, "fbo_has_render_tex", camera->render_tex == -1 ? false : true);
			hashmap_bool_set(entity_data, "fbo_has_depth_tex", camera->depth_tex == -1 ? false : true);
		}
		else
		{
			hashmap_bool_set(entity_data, "has_fbo", true);
		}
		break;
	}
	case ET_STATIC_MESH:
	{
		/* TODO: Change this after adding proper support for exported models from blender */
		struct Material* material = material_get(entity->model.material);
		struct Geometry* geom = geom_get(entity->model.geometry_index);
		hashmap_str_set(entity_data, "material", material->name);
		hashmap_str_set(entity_data, "geometry", geom->filename);
		break;
	}
	case ET_LIGHT:
	{
		struct Light* light = &entity->light;
		hashmap_int_set(entity_data, "light_type", light->type);
		hashmap_float_set(entity_data, "outer_angle", light->outer_angle);
		hashmap_float_set(entity_data, "inner_angle", light->inner_angle);
		hashmap_float_set(entity_data, "falloff", light->falloff);
		hashmap_float_set(entity_data, "radius", light->radius);
		hashmap_float_set(entity_data, "intensity", light->intensity);
		hashmap_float_set(entity_data, "depth_bias", light->depth_bias);
		hashmap_bool_set(entity_data, "valid", light->valid);
		hashmap_bool_set(entity_data, "cast_shadow", light->cast_shadow);
		hashmap_bool_set(entity_data, "pcf_enabled", light->pcf_enabled);
		hashmap_vec3_set(entity_data, "color", &light->color);
		break;
	}
	case ET_SOUND_SOURCE:
	{
		hashmap_bool_set(entity_data, "active", entity->sound_source.active);
		hashmap_bool_set(entity_data, "relative", entity->sound_source.relative);
		hashmap_int_set(entity_data, "sound_type", entity->sound_source.type);
		hashmap_str_set(entity_data, "wav_filename", entity->sound_source.wav_filename);
		hashmap_bool_set(entity_data, "loop", entity->sound_source.loop);
		break;
	}
	};

	return true;
}

bool entity_save(struct Entity* entity, const char* filename, int directory_type)
{
    FILE* entity_file = platform->file.open(directory_type, filename, "w");
	if(!entity_file)
	{
		log_error("entity:save", "Failed to open entity file %s for writing");
		return false;
	}

    struct Parser* parser = parser_new();
    struct Parser_Object* object = parser_object_new(parser, PO_ENTITY);
    if(!entity_write(entity, object))
    {
        log_error("entity:save", "Failed to save entity : %s to file : %s", entity->name, filename);
        fclose(entity_file);
        return false;
    }

    if(parser_write_objects(parser, entity_file, filename))
        log_message("Entity %s saved to %s", entity->name, filename);

    parser_free(parser);
	fclose(entity_file);
	return false;
}

struct Entity* entity_read(struct Parser_Object* object)
{
	assert(object);

	if(object->type != PO_ENTITY)
	{
		log_error("entity:read", "Invalid object type");
		return NULL;
	}

	const char* name = hashmap_str_get(object->data, "name");
	const char* parent_name = hashmap_str_get(object->data, "parent");
	int type = hashmap_int_get(object->data, "type");

	if(!name)
	{
		log_error("entity:read", "No entity name provided");
		return NULL;
	}

	if(!parent_name)
	{
		log_error("entity:read", "No parent name provided");
		return NULL;
	}

	if(type < 0 || type >= ET_MAX)
	{
		log_error("entity:read", "Invalid entity type");
		return NULL;
	}

	struct Entity* parent = entity_find(parent_name);
	struct Entity* entity = entity_create(name, type, parent ? parent->id : -1);
	if(!entity)
	{
		log_error("entity:read", "Failed to create new entity");
		return NULL;
	}

	// Common entity properties
	if(hashmap_value_exists(object->data, "is_listener"))
		entity->is_listener = hashmap_bool_get(object->data, "is_listener");
	else
		entity->is_listener = false;

	if(hashmap_value_exists(object->data, "renderable"))
		entity->renderable= hashmap_bool_get(object->data, "renderable");
	else
		entity->renderable= false;

	// Transform properties
	if(hashmap_value_exists(object->data, "position"))
	{
		vec3 position = hashmap_vec3_get(object->data, "position");
		transform_translate(entity, &position, TS_PARENT);
	}

	if(hashmap_value_exists(object->data, "rotation"))
	{
		quat rotation = hashmap_quat_get(object->data, "rotation");
		quat_assign(&entity->transform.rotation, &rotation);
	}

	if(hashmap_value_exists(object->data, "scale"))
	{
		vec3 scale = hashmap_vec3_get(object->data, "scale");
		transform_scale(entity, &scale);
	}

	switch(entity->type)
	{
	case ET_CAMERA:
	{
		bool has_fbo = false;
		bool fbo_has_depth_tex = false;
		bool fbo_has_render_tex = false;
		int fbo_width = -1;
		int fbo_height = -1;

		if(hashmap_value_exists(object->data, "fov"))                entity->camera.fov = hashmap_float_get(object->data, "fov");
		if(hashmap_value_exists(object->data, "resizeable"))         entity->camera.resizeable = hashmap_bool_get(object->data, "resizeable");
		if(hashmap_value_exists(object->data, "nearz"))              entity->camera.nearz = hashmap_float_get(object->data, "nearz");
		if(hashmap_value_exists(object->data, "farz"))               entity->camera.farz = hashmap_float_get(object->data, "farz");
		if(hashmap_value_exists(object->data, "ortho"))              entity->camera.ortho = hashmap_bool_get(object->data, "ortho");
		if(hashmap_value_exists(object->data, "has_fbo"))            has_fbo = hashmap_bool_get(object->data, "has_fbo");
		if(hashmap_value_exists(object->data, "fbo_has_depth_tex"))  fbo_has_depth_tex = hashmap_bool_get(object->data, "fbo_has_depth_tex");
		if(hashmap_value_exists(object->data, "fbo_has_render_tex")) fbo_has_render_tex = hashmap_bool_get(object->data, "fbo_has_render_tex");
		if(hashmap_value_exists(object->data, "fbo_width"))          fbo_width = hashmap_int_get(object->data, "fbo_width");
		if(hashmap_value_exists(object->data, "fbo_height"))         fbo_height = hashmap_int_get(object->data, "fbo_height");
		if(hashmap_value_exists(object->data, "clear_color"))
		{
			vec4 color = hashmap_vec4_get(object->data, "clear_color");
			vec4_assign(&entity->camera.clear_color, &color);
		}

		float aspect_ratio = (float)fbo_width / (float)fbo_height;
		entity->camera.aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;

		entity->camera.fbo = -1;
		entity->camera.render_tex = -1;
		entity->camera.depth_tex = -1;

		if(has_fbo)
		{
			camera_attach_fbo(entity, fbo_width, fbo_height, fbo_has_depth_tex,	fbo_has_render_tex, entity->camera.resizeable);
		}

		camera_update_proj(entity);
		camera_update_view(entity);

	}
	break;
	case ET_LIGHT:
	{
		if(hashmap_value_exists(object->data, "light_type"))  entity->light.type = hashmap_int_get(object->data, "type");
		if(hashmap_value_exists(object->data, "outer_angle")) entity->light.outer_angle = hashmap_float_get(object->data, "outer_angle");
		if(hashmap_value_exists(object->data, "inner_angle")) entity->light.inner_angle = hashmap_float_get(object->data, "inner_angle");
		if(hashmap_value_exists(object->data, "falloff"))     entity->light.falloff = hashmap_float_get(object->data, "falloff");
		if(hashmap_value_exists(object->data, "intensity"))   entity->light.intensity = hashmap_float_get(object->data, "intensity");
		if(hashmap_value_exists(object->data, "depth_bias"))  entity->light.depth_bias = hashmap_float_get(object->data, "depth_bias");
		if(hashmap_value_exists(object->data, "color"))       entity->light.color = hashmap_vec3_get(object->data, "color");
		if(hashmap_value_exists(object->data, "cast_shadow")) entity->light.cast_shadow = hashmap_bool_get(object->data, "cast_shadow");
		if(hashmap_value_exists(object->data, "pcf_enabled")) entity->light.pcf_enabled = hashmap_bool_get(object->data, "pcf_enabled");
		if(hashmap_value_exists(object->data, "radius"))      entity->light.radius = hashmap_int_get(object->data, "radius");
		light_add(entity);
	}
	break;
	case ET_SOUND_SOURCE:
	{
		struct Sound_Source* sound_source = &entity->sound_source;
		sound_source->active               = false;
		sound_source->relative             = false;
		sound_source->loop                 = false;
		sound_source->source_handle        = 0;
		sound_source->wav_filename         = NULL;
		sound_source->type                 = ST_WAV;

		if(hashmap_value_exists(object->data, "active"))       sound_source->active = hashmap_bool_get(object->data, "active");
		if(hashmap_value_exists(object->data, "relative"))     sound_source->relative = hashmap_bool_get(object->data, "relative");
		if(hashmap_value_exists(object->data, "loop"))         sound_source->loop = hashmap_bool_get(object->data, "loop");
		if(hashmap_value_exists(object->data, "wav_filename")) sound_source->wav_filename = str_new(hashmap_str_get(object->data, "wav_filename"));
		if(hashmap_value_exists(object->data, "sound_type"))   sound_source->type = hashmap_int_get(object->data, "sound_type");
		if(sound_source->wav_filename)
		{
			sound_source->source_handle = platform->sound.source_create(sound_source->relative, sound_source->wav_filename, sound_source->type);
			platform->sound.source_loop_set(sound_source->source_handle, sound_source->loop);
		}

		vec3 abs_pos = {0.f, 0.f,  0.f};
		vec3 abs_fwd = {0.f, 0.f, -1.f};
		vec3 abs_up  = {0.f, 1.f, 0.f};
		transform_get_absolute_pos(entity, &abs_pos);
		transform_get_absolute_forward(entity, &abs_fwd);
		transform_get_absolute_up(entity, &abs_up);
		platform->sound.source_update(entity->sound_source.source_handle, abs_pos.x, abs_pos.y, abs_pos.z);
	}
	break;
	case ET_PLAYER:
	{

	}
	break;
	case ET_STATIC_MESH:
	{
		char* geometry_name = NULL;
		char* material_name = NULL;
		if(hashmap_value_exists(object->data, "geometry")) geometry_name = hashmap_str_get(object->data, "geometry");
		if(hashmap_value_exists(object->data, "material")) material_name = hashmap_str_get(object->data, "material");
		model_create(entity, geometry_name, material_name);
	}
	break;
	case ET_ROOT:
	{
		scene_root_set(entity);
	}
	break;
	default:
		log_warning("Unhandled Entity type '%d' detected", entity->type);
		break;
	}

	if(entity->is_listener)
	{
		vec3 abs_pos = {0.f, 0.f,  0.f};
		vec3 abs_fwd = {0.f, 0.f, -1.f};
		vec3 abs_up  = {0.f, 1.f, 0.f};
		transform_get_absolute_pos(entity, &abs_pos);
		transform_get_absolute_forward(entity, &abs_fwd);
		transform_get_absolute_up(entity, &abs_up);

		platform->sound.listener_update(abs_pos.x, abs_pos.y, abs_pos.z,
										abs_fwd.x, abs_fwd.y, abs_fwd.z,
										abs_up.x,  abs_up.y,  abs_up.z);
	}

	return entity;
}

bool entity_load(const char* filename, int directory_type)
{
    FILE* entity_file = platform->file.open(directory_type, filename, "rb");
	if(!entity_file)
	{
		log_error("entity:load", "Failed to open entity file %s for reading", filename);
		return false;
	}

	struct Parser* parsed_file = parser_load_objects(entity_file, filename);
	struct Entity* new_entity = false;

	if(!parsed_file)
	{
		log_error("entity:load", "Failed to parse file '%s' for entity definition", filename);
		fclose(entity_file);
		return false;
	}

	if(array_len(parsed_file->objects) == 0)
	{
		log_error("entity:load", "No objects found in file %s", filename);
		parser_free(parsed_file);
		fclose(entity_file);
		return false;
	}

	int num_entites_loaded = 0;
	for(int i = 0; i < array_len(parsed_file->objects); i++)
	{
		struct Parser_Object* object = &parsed_file->objects[i];
		if(object->type != PO_ENTITY) continue;

		new_entity = entity_read(object);
		if(new_entity)
		{
			num_entites_loaded++;
			log_message("Entity %s loaded from %s", new_entity->name, filename);
		}
		else
		{
			log_error("entity:load", "Failed to load entity from %s", filename);
		}
	}	

	parser_free(parsed_file);
	fclose(entity_file);
	return num_entites_loaded > 0 ? true : false;
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
