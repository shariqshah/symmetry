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
        platform->sound.source_destroy(entity->sound_source.source_handle,
                                       &entity->sound_source.buffer_handles[0],
                                       entity->sound_source.num_attached_buffers);
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

                platform->sound.source_update(entity->sound_source.source_handle,
                                              abs_pos.x, abs_pos.y, abs_pos.z,
                                              abs_fwd.x, abs_fwd.y, abs_fwd.z,
                                              abs_up.x,  abs_up.y,  abs_up.z);
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

bool entity_write(struct Entity* entity, FILE* file)
{
	if(!file)
	{
		log_error("entity:write", "Invalid file handle");
		return false;
	}

	/* First write all properties common to all entity types */
	fprintf(file, "Entity\n{\n");
	fprintf(file, "\tname: %s\n", entity->name);
	fprintf(file, "\ttype: %d\n", entity->type);
	fprintf(file, "\tis_listener: %s\n", entity->is_listener ? "true" : "false");
	fprintf(file, "\trenderable: %s\n", entity->renderable ? "true" : "false");

	struct Entity* parent = entity_get_parent(entity->id);
	fprintf(file, "\tparent: %s\n", parent ? parent->name : "NONE");

	/* Transform */
	fprintf(file, "\tposition: %.5f %.5f %.5f\n",
			entity->transform.position.x,
			entity->transform.position.y,
			entity->transform.position.z);
	fprintf(file, "\tscale: %.5f %.5f %.5f\n",
			entity->transform.scale.x,
			entity->transform.scale.y,
			entity->transform.scale.z);
	fprintf(file, "\trotation: %.5f %.5f %.5f %.5f\n",
			entity->transform.rotation.x,
			entity->transform.rotation.y,
			entity->transform.rotation.z,
			entity->transform.rotation.w);

	switch(entity->type)
	{
	case ET_CAMERA:
	{
		fprintf(file, "\tortho: %s\n", entity->camera.ortho ? "true" : "false");
		fprintf(file, "\tresizeable: %s\n", entity->camera.resizeable ? "true" : "false");
		fprintf(file, "\tfov: %.5f\n", entity->camera.fov);
		fprintf(file, "\tnearz: %.5f\n", entity->camera.nearz);
		fprintf(file, "\tfarz: %.5f\n", entity->camera.farz);
		fprintf(file, "\tclear_color: %.5f %.5f %.5f %.5f\n",
				entity->camera.clear_color.x,
				entity->camera.clear_color.y,
				entity->camera.clear_color.z,
				entity->camera.clear_color.w);
		if(entity->camera.fbo != -1)
		{
			fprintf(file, "\thas_fbo: true\n");
			fprintf(file, "\tfbo_height: %d\n", framebuffer_height_get(entity->camera.fbo));
			fprintf(file, "\tfbo_width: %d\n", framebuffer_width_get(entity->camera.fbo));
			fprintf(file, "\tfbo_has_render_tex: %s\n", entity->camera.render_tex == -1 ? "false" : "true");
			fprintf(file, "\tfbo_has_depth_tex: %s\n", entity->camera.depth_tex == -1 ? "false" : "true");
		}
		else
		{
			fprintf(file, "\thas_fbo: false\n");
		}
		break;
	}
	case ET_STATIC_MESH:
	{
		/* TODO: Change this after adding proper support for exported models from blender */
		struct Material* material = material_get(entity->model.material);
		struct Geometry* geom = geom_get(entity->model.geometry_index);
		fprintf(file, "\tmaterial: %s\n", material->name);
		fprintf(file, "\tgeometry: %s\n", geom->filename);
		break;
	}
	case ET_LIGHT:
	{
		fprintf(file, "\tlight_type: %d\n", entity->light.valid);
		fprintf(file, "\touter_angle: %.5f\n", entity->light.outer_angle);
		fprintf(file, "\tinner_angle: %.5f\n", entity->light.inner_angle);
		fprintf(file, "\tfalloff: %.5f\n", entity->light.falloff);
		fprintf(file, "\tradius: %d\n", entity->light.radius);
		fprintf(file, "\tintensity: %.5f\n", entity->light.intensity);
		fprintf(file, "\tdepth_bias: %.5f\n", entity->light.depth_bias);
		fprintf(file, "\tvalid: %s\n", entity->light.valid ? "true" : "false");
		fprintf(file, "\tcast_shadow: %s\n", entity->light.cast_shadow ? "true" : "false");
		fprintf(file, "\tpcf_enabled: %s\n", entity->light.pcf_enabled ? "true" : "false");
		fprintf(file, "\tcolor: %.5f %.5f %.5f\n",
				entity->light.color.x,
				entity->light.color.y,
				entity->light.color.z);
		break;
	}
	case ET_SOUND_SOURCE:
	{
		fprintf(file, "\tactive: %s\n", entity->sound_source.active ? "true" : "false");
		fprintf(file, "\trelative: %s\n", entity->sound_source.relative ? "true" : "false");
		break;
	}
	};

	fprintf(file, "}\n\n");
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

	if(entity_write(entity, entity_file))
		log_message("Entity %s saved to %s", entity->name, filename);
	else
		log_error("entity:save", "Failed to save entity : %s to file : %s", entity->name, filename);

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
		if(hashmap_value_exists(object->data, "active"))               entity->sound_source.active = hashmap_bool_get(object->data, "active");
		if(hashmap_value_exists(object->data, "relative"))             entity->sound_source.relative = hashmap_bool_get(object->data, "relative");
		if(hashmap_value_exists(object->data, "num_attached_buffers")) entity->sound_source.num_attached_buffers = (uint)hashmap_int_get(object->data, "num_attached_buffers");
		platform->sound.source_create(entity->sound_source.relative, 
									  entity->sound_source.num_attached_buffers, 
									  &entity->sound_source.source_handle,
									  &entity->sound_source.buffer_handles);
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
	default:
		break;
	}

	return entity;

	//struct Entity entity =
	//{
	//	.id                  = -1,
	//	.type                = ET_NONE,
	//	.is_listener         = false,
	//	.renderable          = false,
	//	.marked_for_deletion = false,
	//	.name                = "DEFAULT_ENTITY_NAME",
	//	.editor_selected     = 0
	//};
	//
 //   int   current_line  	= 0;
	//char* material_name 	= NULL;
	//char* entity_name   	= NULL;
	//char* geometry_name 	= NULL;
	//char* parent_name   	= NULL;
	//int   camera_fbo_width  = -1;
	//int   camera_fbo_height = -1;
 //   char line_buffer[MAX_LINE_LEN];
 //   char prop_str[MAX_ENTITY_PROP_NAME_LEN];
	//static struct Variant var_value = { .type = VT_NONE};

 //   variant_free(&var_value);
	//memset(prop_str, '\0', MAX_ENTITY_PROP_NAME_LEN);
	//memset(line_buffer, '\0', MAX_LINE_LEN);

	//while(fgets(line_buffer, MAX_LINE_LEN -1, file))
	//{
	//	current_line++;
	//	memset(prop_str, '\0', MAX_ENTITY_PROP_NAME_LEN);

	//	if(line_buffer[0] == '#') continue;
	//	if(strlen(line_buffer) == 0 || isspace(line_buffer[0])) break;

	//	char* value_str = strstr(line_buffer, ":");
	//	if(!value_str)
	//	{
	//		log_warning("Malformed value in line %d", current_line);
	//		continue;
	//	}

	//	value_str++; /* Ignore the colon(:) and set the pointer after it */
	//	
	//	if(sscanf(line_buffer, " %1024[^: ] : %*s", prop_str) != 1)
	//	{
	//		log_warning("Unable to read property name in line %d", current_line);
	//		continue;
	//	}

	//	/* Common entity properties */
	//	if(strncmp("name", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_STR);
	//		entity_name = str_new(var_value.val_str);
	//		//variant_copy_out(&entity.name, &var_value);
	//	}
	//	else if(strncmp("parent", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_STR);
	//		parent_name = str_new(var_value.val_str);
	//		//variant_copy_out(&entity.name, &var_value);
	//	}
	//	else if(strncmp("type", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_INT);
	//		variant_copy_out(&entity.type, &var_value);
	//	}
	//	else if(strncmp("is_listener", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.is_listener, &var_value);
	//	}
	//	else if(strncmp("renderable", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.renderable, &var_value);
	//	}
	//	
	//	/* Transform */
	//	else if(strncmp("position", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_VEC3);
	//		variant_copy_out(&entity.transform.position, &var_value);
	//	}
	//	else if(strncmp("scale", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_VEC3);
	//		variant_copy_out(&entity.transform.scale, &var_value);
	//	}
	//	else if(strncmp("rotation", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_QUAT);
	//		variant_copy_out(&entity.transform.rotation, &var_value);
	//	}

	//	/* Camera */
	//	else if(strncmp("ortho", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.camera.ortho, &var_value);
	//	}
	//	else if(strncmp("resizeable", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.camera.resizeable, &var_value);
	//	}
	//	else if(strncmp("fov", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.camera.fov, &var_value);
	//	}
	//	else if(strncmp("nearz", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.camera.nearz, &var_value);
	//	}
	//	else if(strncmp("farz", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.camera.farz, &var_value);
	//	}
	//	else if(strncmp("has_fbo", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		entity.camera.fbo = var_value.val_bool ? 0 : -1;
	//	}		
	//	else if(strncmp("fbo_height", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_INT);
	//		variant_copy_out(&camera_fbo_height, &var_value);
	//	}		
	//	else if(strncmp("fbo_width", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_INT);
	//		variant_copy_out(&camera_fbo_width, &var_value);
	//	}
	//	else if(strncmp("fbo_has_depth_tex", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		entity.camera.depth_tex = var_value.val_bool ? 0 : -1;
	//	}
	//	else if(strncmp("fbo_has_render_tex", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		entity.camera.render_tex = var_value.val_bool ? 0 : -1;
	//	}
	//	else if(strncmp("clear_color", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_VEC4);
	//		variant_copy_out(&entity.camera.clear_color, &var_value);
	//	}

	//	/* Light */
	//	else if(strncmp("light_type", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_INT);
	//		variant_copy_out(&entity.light.type, &var_value);
	//	}
	//	else if(strncmp("outer_angle", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.light.outer_angle, &var_value);
	//	}
	//	else if(strncmp("inner_angle", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.light.inner_angle, &var_value);
	//	}
	//	else if(strncmp("falloff", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.light.falloff, &var_value);
	//	}
	//	else if(strncmp("radius", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_INT);
	//		variant_copy_out(&entity.light.radius, &var_value);
	//	}
	//	else if(strncmp("intensity", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.light.intensity, &var_value);
	//	}
	//	else if(strncmp("depth_bias", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_FLOAT);
	//		variant_copy_out(&entity.light.depth_bias, &var_value);
	//	}
	//	else if(strncmp("valid", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.light.valid, &var_value);
	//	}
	//	else if(strncmp("cast_shadow", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.light.cast_shadow, &var_value);
	//	}
	//	else if(strncmp("pcf_enabled", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.light.pcf_enabled, &var_value);
	//	}
	//	else if(strncmp("color", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_VEC3);
	//		variant_copy_out(&entity.light.color, &var_value);
	//	}

	//	/* Model */
	//	else if(strncmp("material", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_STR);
	//		material_name = str_new(var_value.val_str);
	//	}
	//	else if(strncmp("geometry", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_STR);
	//		geometry_name = str_new(var_value.val_str);
	//	}

	//	/* Sound Source */
	//	else if(strncmp("active", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.sound_source.active, &var_value);
	//	}
	//	else if(strncmp("relative", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
	//	{
	//		variant_from_str(&var_value, value_str, VT_BOOL);
	//		variant_copy_out(&entity.sound_source.relative, &var_value);
	//	}
 //       else if(strncmp("num_attached_buffers", prop_str, MAX_ENTITY_PROP_NAME_LEN) == 0)
 //       {
 //           variant_from_str(&var_value, value_str, VT_INT);
 //           variant_copy_out(&entity.sound_source.num_attached_buffers, &var_value);
 //       }
	//	else
	//	{
	//		log_warning("Unknown entity property '%s' in line %d", prop_str, current_line);
	//	}

	//	variant_free(&var_value);
	//}

	///* Do the things after assignment */
	//struct Entity* parent_entity = NULL;
	//if(strcmp(parent_name, "NONE") != 0)
	//	parent_entity = entity_find(parent_name);
	//
	//struct Entity* new_entity = entity_create(entity_name, entity.type, parent_entity ? parent_entity->id : -1);
	//free(entity_name);
	//transform_translate(new_entity, &entity.transform.position, TS_PARENT);
	//quat_assign(&new_entity->transform.rotation, &entity.transform.rotation);
	//transform_scale(new_entity, &entity.transform.scale);
	//
	//if(entity.renderable)  new_entity->renderable = true;
	//
	//switch(new_entity->type)
	//{
	//case ET_CAMERA:
	//	new_entity->camera.fbo        = -1;
	//	new_entity->camera.depth_tex  = -1;
	//	new_entity->camera.render_tex = -1;
	//	new_entity->camera.resizeable = false;
	//	new_entity->camera.nearz = entity.camera.nearz;
	//	new_entity->camera.farz  = entity.camera.farz;
	//	new_entity->camera.ortho = entity.camera.ortho;
	//	new_entity->camera.fov   = entity.camera.fov;
	//	float aspect_ratio = (float)camera_fbo_width / (float)camera_fbo_height;
	//	new_entity->camera.aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;
	//	camera_update_view(new_entity);
	//	camera_update_proj(new_entity);
	//	if(entity.camera.fbo != -1)
	//	{
	//		camera_attach_fbo(new_entity, camera_fbo_width, camera_fbo_height,
	//						  entity.camera.depth_tex == -1 ? false : true,
	//						  entity.camera.render_tex == -1 ? false : true,
	//						  entity.camera.resizeable);
	//	}
	//	vec4_assign(&new_entity->camera.clear_color, &entity.camera.clear_color);
	//	break;
	//case ET_STATIC_MESH:
	//	model_create(new_entity, geometry_name, material_name);
	//	free(geometry_name);
	//	free(material_name);
	//	break;
	//case ET_LIGHT:
	//	memcpy(&new_entity->light, &entity.light, sizeof(struct Light));
	//	light_add(new_entity);
	//	break;
	//case ET_SOUND_SOURCE:
 //       platform->sound.source_create(entity.sound_source.relative,
 //                                     entity.sound_source.num_attached_buffers,
 //                                     &new_entity->sound_source.source_handle,
 //                                     &new_entity->sound_source.buffer_handles[0]);
	//	break;
	//};

	//return new_entity;
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
