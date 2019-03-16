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
#include "../common/parser.h"
#include "../common/hashmap.h"
#include "../system/file_io.h"
#include "../system/physics.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>

#define MAX_ENTITY_PROP_NAME_LEN 128
#define MAX_ENTITY_PROP_LEN      256
#define MAX_LINE_LEN             512

void entity_init(struct Entity* entity, const char* name, struct Entity* parent)
{
	assert(entity);

	strncpy(entity->name, name ? name : "DEFAULT_ENTITY_NAME", MAX_ENTITY_NAME_LEN);
	entity->name[MAX_ENTITY_NAME_LEN - 1] = '\0';
	entity->type                = ET_DEFAULT;
	entity->active              = true;
	entity->marked_for_deletion = false;
	entity->editor_selected     = false;
	transform_init(entity, parent);
}

void entity_reset(struct Entity * entity, int id)
{
	assert(entity);
	entity->id                  = id;
	entity->active              = false;
	entity->marked_for_deletion = false;
	entity->editor_selected     = false;
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
}


bool entity_write(struct Entity* entity, struct Parser_Object* object)
{
	//if(!object)
	//{
	//	log_error("entity:write", "Invalid object");
	//	return false;
	//}

	///* First write all properties common to all entity types */
	//struct Hashmap* entity_data = object->data;

	//hashmap_str_set(entity_data, "name", entity->name);
	//hashmap_int_set(entity_data, "type", entity->type);
	//hashmap_bool_set(entity_data, "is_listener", entity->is_listener);
	//hashmap_bool_set(entity_data, "renderable", entity->renderable);
	//hashmap_bool_set(entity_data, "has_collision", entity->has_collision);

	//if(entity->has_collision)
	//{
	//	if(entity->collision.rigidbody)
	//		hashmap_bool_set(entity_data, "has_rigidbody", true);
	//	else
	//		hashmap_bool_set(entity_data, "has_rigidbody", false);

	//	int shape_type = platform->physics.cs_type_get(entity->collision.collision_shape);
	//	hashmap_int_set(entity_data, "collision_shape_type", shape_type);
	//	switch(shape_type)
	//	{
	//	case CST_BOX:
	//	{
	//		float x, y, z;
	//		x = y = z = 0.f;
	//		platform->physics.cs_box_params_get(entity->collision.collision_shape, &x, &y, &z);
	//		hashmap_float_set(entity_data, "collision_shape_x", x);
	//		hashmap_float_set(entity_data, "collision_shape_y", y);
	//		hashmap_float_set(entity_data, "collision_shape_z", z);
	//	}
	//	break;
	//	case CST_SPHERE:
	//	{
	//		float radius = 0.f;
	//		platform->physics.cs_sphere_radius_get(entity->collision.collision_shape);
	//		hashmap_float_set(entity_data, "collision_shape_radius", radius);
	//	}
	//	break;
	//	case CST_CAPSULE:
	//	{
	//		float length = 0.f, radius = 0.f;
	//		platform->physics.cs_capsule_params_get(entity->collision.collision_shape, &radius, &length);
	//		hashmap_float_set(entity_data, "collision_shape_length", length);
	//		hashmap_float_set(entity_data, "collision_shape_radius", radius);
	//	}
	//	break;
	//	case CST_PLANE:
	//	{
	//		float a, b, c, d;
	//		platform->physics.cs_plane_params_get(entity->collision.collision_shape, &a, &b, &c, &d);
	//		hashmap_float_set(entity_data, "collision_shape_a", a);
	//		hashmap_float_set(entity_data, "collision_shape_b", b);
	//		hashmap_float_set(entity_data, "collision_shape_c", c);
	//		hashmap_float_set(entity_data, "collision_shape_d", d);
	//	}
	//	break;
	//	default: break;
	//	}

	//}

	//struct Entity* parent = entity_get_parent(entity->id);
	//hashmap_str_set(entity_data, "parent", parent ? parent->name  : "NONE");

	///* Transform */
	//hashmap_vec3_set(entity_data, "position", &entity->transform.position);
	//hashmap_vec3_set(entity_data, "scale", &entity->transform.scale);
	//hashmap_quat_set(entity_data, "rotation", &entity->transform.rotation);
	//switch(entity->type)
	//{
	//case ET_CAMERA:
	//{
	//	struct Camera* camera = &entity->camera;
	//	hashmap_bool_set(entity_data, "ortho", camera->ortho);
	//	hashmap_bool_set(entity_data, "resizeable", camera->resizeable);
	//	hashmap_float_set(entity_data, "fov", camera->fov);
	//	hashmap_float_set(entity_data, "zoom", camera->zoom);
	//	hashmap_float_set(entity_data, "nearz", camera->nearz);
	//	hashmap_float_set(entity_data, "farz", camera->farz);
	//	hashmap_vec4_set(entity_data, "clear_color", &camera->clear_color);
	//	if(entity->camera.fbo != -1)
	//	{
	//		hashmap_bool_set(entity_data, "has_fbo", true);
	//		hashmap_int_set(entity_data, "fbo_height", framebuffer_height_get(camera->fbo));
	//		hashmap_int_set(entity_data, "fbo_width", framebuffer_width_get(camera->fbo));
	//		hashmap_bool_set(entity_data, "fbo_has_render_tex", camera->render_tex == -1 ? false : true);
	//		hashmap_bool_set(entity_data, "fbo_has_depth_tex", camera->depth_tex == -1 ? false : true);
	//	}
	//	else
	//	{
	//		hashmap_bool_set(entity_data, "has_fbo", true);
	//	}
	//	break;
	//}
	//case ET_STATIC_MODEL:
	//{
	//	/* TODO: Change this after adding proper support for exported models from blender */
	//	struct Material* material = material_get(entity->model.material);
	//	struct Geometry* geom = geom_get(entity->model.geometry_index);
	//	hashmap_str_set(entity_data, "material", material->name);
	//	hashmap_str_set(entity_data, "geometry", geom->filename);
	//	break;
	//}
	//case ET_LIGHT:
	//{
	//	struct Light* light = &entity->light;
	//	hashmap_int_set(entity_data, "light_type", light->type);
	//	hashmap_float_set(entity_data, "outer_angle", light->outer_angle);
	//	hashmap_float_set(entity_data, "inner_angle", light->inner_angle);
	//	hashmap_float_set(entity_data, "falloff", light->falloff);
	//	hashmap_float_set(entity_data, "radius", light->radius);
	//	hashmap_float_set(entity_data, "intensity", light->intensity);
	//	hashmap_float_set(entity_data, "depth_bias", light->depth_bias);
	//	hashmap_bool_set(entity_data, "valid", light->valid);
	//	hashmap_bool_set(entity_data, "cast_shadow", light->cast_shadow);
	//	hashmap_bool_set(entity_data, "pcf_enabled", light->pcf_enabled);
	//	hashmap_vec3_set(entity_data, "color", &light->color);
	//	break;
	//}
	//case ET_SOUND_SOURCE:
	//{
	//	struct Sound_Source* sound_source = &entity->sound_source;
	//	hashmap_str_set(entity_data, "source_filename", sound_source->source_filename);
	//	hashmap_bool_set(entity_data, "playing", sound_source->playing);
	//	hashmap_int_set(entity_data, "sound_type", sound_source->type);
	//	hashmap_bool_set(entity_data, "loop", sound_source->loop);
	//	hashmap_float_set(entity_data, "volume", sound_source->volume);
	//	hashmap_float_set(entity_data, "sound_min_distance", sound_source->min_distance);
	//	hashmap_float_set(entity_data, "sound_max_distance", sound_source->max_distance);
	//	hashmap_float_set(entity_data, "rolloff_factor", sound_source->rolloff_factor);
	//	hashmap_int_set(entity_data, "sound_attenuation_type", sound_source->attenuation_type);
	//	break;
	//}
	//};

	return true;
}

bool entity_save(struct Entity* entity, const char* filename, int directory_type)
{
    FILE* entity_file = io_file_open(directory_type, filename, "w");
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
	return NULL;
	//assert(object);

	//if(object->type != PO_ENTITY)
	//{
	//	log_error("entity:read", "Invalid object type");
	//	return NULL;
	//}

	//const char* name = hashmap_str_get(object->data, "name");
	//const char* parent_name = hashmap_str_get(object->data, "parent");
	//int type = hashmap_int_get(object->data, "type");

	//if(!name)
	//{
	//	log_error("entity:read", "No entity name provided");
	//	return NULL;
	//}

	//if(!parent_name)
	//{
	//	log_error("entity:read", "No parent name provided");
	//	return NULL;
	//}

	//if(type < 0 || type >= ET_MAX)
	//{
	//	log_error("entity:read", "Invalid entity type");
	//	return NULL;
	//}

	//struct Entity* parent = entity_find(parent_name);
	//struct Entity* entity = entity_create(name, type, parent ? parent->id : -1);
	//if(!entity)
	//{
	//	log_error("entity:read", "Failed to create new entity");
	//	return NULL;
	//}

	//// Common entity properties
	//if(hashmap_value_exists(object->data, "is_listener"))
	//	entity->is_listener = hashmap_bool_get(object->data, "is_listener");
	//else
	//	entity->is_listener = false;

	//if(hashmap_value_exists(object->data, "renderable"))
	//	entity->renderable= hashmap_bool_get(object->data, "renderable");
	//else
	//	entity->renderable= false;

	//// Transform properties
	//if(hashmap_value_exists(object->data, "position"))
	//{
	//	vec3 position = hashmap_vec3_get(object->data, "position");
	//	transform_translate(entity, &position, TS_PARENT);
	//}

	//if(hashmap_value_exists(object->data, "rotation"))
	//{
	//	quat rotation = hashmap_quat_get(object->data, "rotation");
	//	quat_assign(&entity->transform.rotation, &rotation);
	//}

	//if(hashmap_value_exists(object->data, "scale"))
	//{
	//	vec3 scale = hashmap_vec3_get(object->data, "scale");
	//	transform_scale(entity, &scale);
	//}

	//switch(entity->type)
	//{
	//case ET_CAMERA:
	//{
	//	bool has_fbo = false;
	//	bool fbo_has_depth_tex = false;
	//	bool fbo_has_render_tex = false;
	//	int fbo_width = -1;
	//	int fbo_height = -1;

	//	if(hashmap_value_exists(object->data, "fov"))                entity->camera.fov = hashmap_float_get(object->data, "fov");
	//	if(hashmap_value_exists(object->data, "resizeable"))         entity->camera.resizeable = hashmap_bool_get(object->data, "resizeable");
	//	if(hashmap_value_exists(object->data, "zoom"))               entity->camera.zoom = hashmap_float_get(object->data, "zoom");
	//	if(hashmap_value_exists(object->data, "nearz"))              entity->camera.nearz = hashmap_float_get(object->data, "nearz");
	//	if(hashmap_value_exists(object->data, "farz"))               entity->camera.farz = hashmap_float_get(object->data, "farz");
	//	if(hashmap_value_exists(object->data, "ortho"))              entity->camera.ortho = hashmap_bool_get(object->data, "ortho");
	//	if(hashmap_value_exists(object->data, "has_fbo"))            has_fbo = hashmap_bool_get(object->data, "has_fbo");
	//	if(hashmap_value_exists(object->data, "fbo_has_depth_tex"))  fbo_has_depth_tex = hashmap_bool_get(object->data, "fbo_has_depth_tex");
	//	if(hashmap_value_exists(object->data, "fbo_has_render_tex")) fbo_has_render_tex = hashmap_bool_get(object->data, "fbo_has_render_tex");
	//	if(hashmap_value_exists(object->data, "fbo_width"))          fbo_width = hashmap_int_get(object->data, "fbo_width");
	//	if(hashmap_value_exists(object->data, "fbo_height"))         fbo_height = hashmap_int_get(object->data, "fbo_height");
	//	if(hashmap_value_exists(object->data, "clear_color"))
	//	{
	//		vec4 color = hashmap_vec4_get(object->data, "clear_color");
	//		vec4_assign(&entity->camera.clear_color, &color);
	//	}

	//	float aspect_ratio = (float)fbo_width / (float)fbo_height;
	//	entity->camera.aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;

	//	entity->camera.fbo = -1;
	//	entity->camera.render_tex = -1;
	//	entity->camera.depth_tex = -1;

	//	if(has_fbo)
	//	{
	//		camera_attach_fbo(entity, fbo_width, fbo_height, fbo_has_depth_tex,	fbo_has_render_tex, entity->camera.resizeable);
	//	}

	//	camera_update_proj(entity);
	//	camera_update_view(entity);

	//}
	//break;
	//case ET_LIGHT:
	//{
	//	if(hashmap_value_exists(object->data, "light_type"))  entity->light.type = hashmap_int_get(object->data, "type");
	//	if(hashmap_value_exists(object->data, "outer_angle")) entity->light.outer_angle = hashmap_float_get(object->data, "outer_angle");
	//	if(hashmap_value_exists(object->data, "inner_angle")) entity->light.inner_angle = hashmap_float_get(object->data, "inner_angle");
	//	if(hashmap_value_exists(object->data, "falloff"))     entity->light.falloff = hashmap_float_get(object->data, "falloff");
	//	if(hashmap_value_exists(object->data, "intensity"))   entity->light.intensity = hashmap_float_get(object->data, "intensity");
	//	if(hashmap_value_exists(object->data, "depth_bias"))  entity->light.depth_bias = hashmap_float_get(object->data, "depth_bias");
	//	if(hashmap_value_exists(object->data, "color"))       entity->light.color = hashmap_vec3_get(object->data, "color");
	//	if(hashmap_value_exists(object->data, "cast_shadow")) entity->light.cast_shadow = hashmap_bool_get(object->data, "cast_shadow");
	//	if(hashmap_value_exists(object->data, "pcf_enabled")) entity->light.pcf_enabled = hashmap_bool_get(object->data, "pcf_enabled");
	//	if(hashmap_value_exists(object->data, "radius"))      entity->light.radius = hashmap_int_get(object->data, "radius");
	//	light_add(entity);
	//}
	//break;
	//case ET_SOUND_SOURCE:
	//{
	//	struct Sound_Source* sound_source = &entity->sound_source;
	//	sound_source->type                = ST_WAV;
	//	sound_source->playing             = false;
	//	sound_source->loop                = false;
	//	sound_source->source_instance     = 0;
	//	sound_source->min_distance        = 1.f;
	//	sound_source->max_distance        = 1000.f;
	//	sound_source->volume              = 1.f;
	//	sound_source->rolloff_factor      = 1.f;
	//	sound_source->attenuation_type    = SA_EXPONENTIAL;
	//	sound_source->source_buffer              = NULL;
	//	sound_source->source_filename     = NULL;

	//	if(hashmap_value_exists(object->data, "playing"))                sound_source->playing          = hashmap_bool_get(object->data, "playing");
	//	if(hashmap_value_exists(object->data, "loop"))                   sound_source->loop             = hashmap_bool_get(object->data, "loop");
	//	if(hashmap_value_exists(object->data, "sound_min_distance"))     sound_source->min_distance     = hashmap_float_get(object->data, "sound_min_distance");
	//	if(hashmap_value_exists(object->data, "sound_max_distance"))     sound_source->max_distance     = hashmap_float_get(object->data, "sound_max_distance");
	//	if(hashmap_value_exists(object->data, "volume"))                 sound_source->volume           = hashmap_float_get(object->data, "volume");
	//	if(hashmap_value_exists(object->data, "rolloff_factor"))         sound_source->rolloff_factor   = hashmap_float_get(object->data, "rolloff_factor");
	//	if(hashmap_value_exists(object->data, "source_filename"))        sound_source->source_filename  = str_new(hashmap_str_get(object->data, "source_filename"));
	//	if(hashmap_value_exists(object->data, "sound_type"))             sound_source->type             = hashmap_int_get(object->data, "sound_type");
	//	if(hashmap_value_exists(object->data, "sound_attenuation_type")) sound_source->attenuation_type = hashmap_int_get(object->data, "sound_attenuation_type");
	//	if(sound_source->source_filename)
	//	{
	//		sound_source->source_buffer = platform->sound.source_create(sound_source->source_filename, sound_source->type);
	//		if(sound_source->source_buffer)
	//		{
	//			sound_source->source_instance = platform->sound.source_instance_create(sound_source->source_buffer, true);

	//			vec3 abs_pos = {0.f, 0.f,  0.f};
	//			vec3 abs_fwd = {0.f, 0.f, -1.f};
	//			vec3 abs_up  = {0.f, 1.f, 0.f};
	//			transform_get_absolute_position(entity, &abs_pos);
	//			transform_get_absolute_forward(entity, &abs_fwd);
	//			transform_get_absolute_up(entity, &abs_up);
	//			platform->sound.source_instance_update_position(entity->sound_source.source_instance, abs_pos.x, abs_pos.y, abs_pos.z);

	//			platform->sound.source_instance_loop_set(sound_source->source_instance, sound_source->loop);
	//			platform->sound.source_instance_min_max_distance_set(sound_source->source_instance, sound_source->min_distance, sound_source->max_distance);
	//			platform->sound.source_instance_attenuation_set(sound_source->source_instance, sound_source->attenuation_type, sound_source->rolloff_factor);
	//			platform->sound.source_instance_volume_set(sound_source->source_instance, sound_source->volume);

	//			platform->sound.update_3d();
	//			if(sound_source->playing) platform->sound.source_instance_play(sound_source->source_instance);
	//		}
	//		else
	//		{
	//			log_error("Failed to create sound source from '%s'", sound_source->source_filename);
	//			free(sound_source->source_filename);
	//			sound_source->source_filename = NULL;
	//		}
	//	}
	//	else
	//	{
	//		log_error("entity:read", "No filename provided for sound source for entity '%s'", entity->name);
	//	}
	//}
	//break;
	//case ET_PLAYER:
	//{

	//}
	//break;
	//case ET_STATIC_MODEL:
	//{
	//	char* geometry_name = NULL;
	//	char* material_name = NULL;
	//	if(hashmap_value_exists(object->data, "geometry")) geometry_name = hashmap_str_get(object->data, "geometry");
	//	if(hashmap_value_exists(object->data, "material")) material_name = hashmap_str_get(object->data, "material");
	//	model_create(entity, geometry_name, material_name);
	//}
	//break;
	//case ET_ROOT:
	//{
	//	scene_root_set(entity);
	//}
	//break;
	//default:
	//	log_warning("Unhandled Entity type '%d' detected", entity->type);
	//	break;
	//}

	//if(entity->is_listener)
	//{
	//	vec3 abs_pos = {0.f, 0.f,  0.f};
	//	vec3 abs_fwd = {0.f, 0.f, -1.f};
	//	vec3 abs_up  = {0.f, 1.f, 0.f};
	//	transform_get_absolute_position(entity, &abs_pos);
	//	transform_get_absolute_forward(entity, &abs_fwd);
	//	transform_get_absolute_up(entity, &abs_up);

	//	platform->sound.listener_update(abs_pos.x, abs_pos.y, abs_pos.z,
	//									abs_fwd.x, abs_fwd.y, abs_fwd.z,
	//									abs_up.x,  abs_up.y,  abs_up.z);
	//}

	//return entity;
}

bool entity_load(const char* filename, int directory_type)
{
    FILE* entity_file = io_file_open(directory_type, filename, "rb");
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
	case ET_DEFAULT:      typename = "Default";      break;
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

void entity_rigidbody_on_move(Rigidbody body)
{
	struct Entity* entity = physics_body_data_get(body);
	vec3 pos = {0.f};
	quat rot = {0.f};

	physics_body_position_get(body, &pos.x, &pos.y, &pos.z);
	physics_body_rotation_get(body, &rot.x, &rot.y, &rot.z, &rot.w);

	quat_assign(&entity->transform.rotation, &rot);
	transform_set_position(entity, &pos);
	entity->transform.sync_physics = false;
}

void entity_rigidbody_on_collision(Rigidbody body_A, Rigidbody body_B)
{
	struct Entity* ent_A = NULL;
	struct Entity* ent_B = NULL;

	if(body_A) 
	{
		ent_A = physics_body_data_get(body_A);
	}

	if(body_B) 
	{
		ent_B = physics_body_data_get(body_B);
	}

	//if(ent_A && ent_A->collision.on_collision)
	//{
	//	ent_A->collision.on_collision(ent_A, ent_B ? ent_B : NULL, body_A, body_B ? body_B : NULL);
	//}

	//if(ent_B && ent_B->collision.on_collision)
	//{
	//	ent_B->collision.on_collision(ent_B, ent_A ? ent_A : NULL, body_B, body_A ? body_A : NULL);
	//}

	if(ent_A && ent_B)
	{
		//log_message("Entity %s collided with Entity %s", ent_A->name, ent_B->name);
	}
}

void entity_rigidbody_set(struct Entity * entity, struct Collision* collision, Rigidbody body)
{
	assert(entity && body);

	//Remove previous rigidbody if there is any
	if(collision->rigidbody || collision->collision_shape)
	{
		if(collision->rigidbody)
		{
			physics_body_remove(collision->rigidbody);
		}
		else if(collision->collision_shape)
		{
			physics_cs_remove(collision->collision_shape);
		}
		collision->rigidbody = NULL;
		collision->collision_shape = NULL;
	}

	collision->rigidbody       = body;
	collision->collision_shape = physics_body_cs_get(body);

	vec3 abs_pos = {0.f, 0.f,  0.f};
	quat abs_rot = {0.f, 0.f, 0.f, 1.f};
	transform_get_absolute_position(entity, &abs_pos);
	transform_get_absolute_rot(entity, &abs_rot);

	physics_body_rotation_set(body, abs_rot.x, abs_rot.y, abs_rot.z, abs_rot.w);
	physics_body_position_set(body, abs_pos.x, abs_pos.y, abs_pos.z);
	physics_body_data_set(body, entity);
}

void entity_collision_shape_set(struct Entity* entity, struct Collision* collision, Collision_Shape shape)
{
	assert(entity && shape);

	if(collision->rigidbody || collision->collision_shape)
	{
		if(collision->rigidbody)
		{
			physics_body_remove(collision->rigidbody);
		}
		else if(collision->collision_shape)
		{
			physics_cs_remove(collision->collision_shape);
		}
		collision->rigidbody = NULL;
		collision->collision_shape = NULL;
	}

	collision->collision_shape = shape;
	physics_cs_data_set(shape, entity);
}

void entity_rename(struct Entity* entity, const char* new_name)
{
	assert(entity);
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
	snprintf(entity->name, MAX_ENTITY_NAME_LEN, new_name);
}
