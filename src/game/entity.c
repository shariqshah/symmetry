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
#include "scene.h"
#include "game.h"
#include "texture.h"
#include "enemy.h"
#include "event.h"
#include "sound_source.h"
#include "door.h"
#include "pickup.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <float.h>

#define MAX_ENTITY_PROP_NAME_LEN 128
#define MAX_ENTITY_PROP_LEN      256
#define MAX_LINE_LEN             512

void entity_init(struct Entity* entity, const char* name, struct Entity* parent)
{
	assert(entity);

	strncpy(entity->name, name ? name : "DEFAULT_ENTITY_NAME", MAX_ENTITY_NAME_LEN);
	entity->name[MAX_ENTITY_NAME_LEN - 1] = '\0';
	entity->type                     = ET_DEFAULT;
	entity->archetype_index          = -1;
	entity->flags                    = EF_NONE;
	entity_bounding_box_reset(entity, false);
	entity->derived_bounding_box.min = (vec3){ -0.5f, -0.5f, -0.5f };
	entity->derived_bounding_box.max = (vec3){  0.5f,  0.5f,  0.5f };
	transform_init(entity, parent);
}

void entity_reset(struct Entity* entity, int id)
{
	assert(entity);
	entity->id                       = id;
	entity->type                     = ET_DEFAULT;
	entity->archetype_index          = -1;
	entity->flags                    = EF_NONE;
	entity_bounding_box_reset(entity, false);
	entity->derived_bounding_box.min = (vec3){ -0.5f, -0.5f, -0.5f };
	entity->derived_bounding_box.max = (vec3){  0.5f,  0.5f,  0.5f };
	transform_destroy(entity);
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
}

void entity_update_derived_bounding_box(struct Entity* entity)
{
	struct Bounding_Box* derived_box = &entity->derived_bounding_box;
	vec3_fill(&derived_box->min, FLT_MAX, FLT_MAX, FLT_MAX);
	vec3_fill(&derived_box->max, -FLT_MAX, -FLT_MAX, -FLT_MAX);
	vec3 box_vertices[8];
	bv_bounding_box_vertices_get(&entity->bounding_box, box_vertices);

	for(int i = 0; i < 8; i++)
	{
		vec3 transformed_vertex = { 0.f, 0.f, 0.f };
		vec3_mul_mat4(&transformed_vertex, &box_vertices[i], &entity->transform.trans_mat);
		if(transformed_vertex.x < derived_box->min.x) derived_box->min.x = transformed_vertex.x;
		if(transformed_vertex.y < derived_box->min.y) derived_box->min.y = transformed_vertex.y;
		if(transformed_vertex.z < derived_box->min.z) derived_box->min.z = transformed_vertex.z;

		if(transformed_vertex.x > derived_box->max.x) derived_box->max.x = transformed_vertex.x;
		if(transformed_vertex.y > derived_box->max.y) derived_box->max.y = transformed_vertex.y;
		if(transformed_vertex.z > derived_box->max.z) derived_box->max.z = transformed_vertex.z;
	}
}

void entity_bounding_box_reset(struct Entity* entity, bool update_derived)
{
	if(entity->type == ET_STATIC_MESH)
	{
		struct Static_Mesh* mesh = (struct Static_Mesh*)entity;
		struct Geometry* geom = geom_get(mesh->model.geometry_index);
		vec3_assign(&entity->bounding_box.min, &geom->bounding_box.min);
		vec3_assign(&entity->bounding_box.max, &geom->bounding_box.max);
	}
	else
	{
		vec3_fill(&entity->bounding_box.min, -0.5f, -0.5f, -0.5f);
		vec3_fill(&entity->bounding_box.max,  0.5f,  0.5f,  0.5f);
	}
	if(update_derived) entity_update_derived_bounding_box(entity);
     
}

bool entity_write(struct Entity* entity, struct Parser_Object* object, bool write_transform)
{
	if(!object)
	{
		log_error("entity:write", "Invalid object");
		return false;
	}

	struct Scene* scene = game_state_get()->scene;

	/* First write all properties common to all entity types */
	struct Hashmap* entity_data = object->data;

	hashmap_str_set(entity_data, "name", entity->name);
	hashmap_int_set(entity_data, "type", entity->type);
	if(entity->archetype_index != -1) hashmap_str_set(entity_data, "archetype", scene->entity_archetypes[entity->archetype_index]);

	/* Transform */
	hashmap_vec3_set(entity_data, "position", &entity->transform.position);
	hashmap_vec3_set(entity_data, "scale", &entity->transform.scale);
	hashmap_quat_set(entity_data, "rotation", &entity->transform.rotation);

	if(entity->type != ET_STATIC_MESH)
	{
		hashmap_vec3_set(entity_data, "bounding_box_min", &entity->bounding_box.min);
		hashmap_vec3_set(entity_data, "bounding_box_max", &entity->bounding_box.max);
	}

	uint flags = entity->flags & ~(EF_SELECTED_IN_EDITOR | EF_MARKED_FOR_DELETION); // Unset flags only used during run-time
	hashmap_uint_set(entity_data, "flags", flags);

	switch(entity->type)
	{
	case ET_CAMERA:
	{
		struct Camera* camera = (struct Camera*)entity;
		hashmap_bool_set(entity_data, "ortho", camera->ortho);
		hashmap_bool_set(entity_data, "resizeable", camera->resizeable);
		hashmap_float_set(entity_data, "fov", camera->fov);
		hashmap_float_set(entity_data, "zoom", camera->zoom);
		hashmap_float_set(entity_data, "nearz", camera->nearz);
		hashmap_float_set(entity_data, "farz", camera->farz);
		hashmap_vec4_set(entity_data, "clear_color", &camera->clear_color);
		if(camera->fbo != -1)
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
	}
	break;
	case ET_STATIC_MESH:
	{
		struct Static_Mesh* mesh = (struct Static_Mesh*)entity;
		struct Geometry* geom = geom_get(mesh->model.geometry_index);
		hashmap_int_set(entity_data, "material", mesh->model.material->type);

		//Set material model params for this particular mesh
		struct Model* model = &mesh->model;
		switch(model->material->type)
		{
		case MAT_BLINN:
			hashmap_vec4_set(entity_data, "diffuse_color", &model->material_params[MMP_DIFFUSE_COL].val_vec4);
			hashmap_str_set(entity_data, "diffuse_texture", model->material_params[MMP_DIFFUSE_TEX].val_int == -1 ? "default.tga" : texture_get_name(model->material_params[MMP_DIFFUSE_TEX].val_int));
			hashmap_float_set(entity_data, "diffuse", model->material_params[MMP_DIFFUSE].val_float);
			hashmap_float_set(entity_data, "specular", model->material_params[MMP_SPECULAR].val_float);
			hashmap_float_set(entity_data, "specular_strength", model->material_params[MMP_SPECULAR_STRENGTH].val_float);
			hashmap_vec2_set(entity_data, "uv_scale", &model->material_params[MMP_UV_SCALE].val_vec2);
			break;
		case MAT_UNSHADED:
			hashmap_vec4_set(entity_data, "diffuse_color", &model->material_params[MMP_DIFFUSE_COL].val_vec4);
			hashmap_str_set(entity_data, "diffuse_texture", model->material_params[MMP_DIFFUSE_TEX].val_int == -1 ? "default.tga" : texture_get_name(model->material_params[MMP_DIFFUSE_TEX].val_int));
			hashmap_vec2_set(entity_data, "uv_scale", &model->material_params[MMP_UV_SCALE].val_vec2);
			break;
		};

		hashmap_str_set(entity_data, "geometry", geom->filename);
	}
	break;
	case ET_LIGHT:
	{
		struct Light* light = (struct Light*)entity;
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
	}
	break;
	case ET_SOUND_SOURCE:
	{
		struct Sound_Source* sound_source = (struct Sound_Source*)entity;
		hashmap_str_set(entity_data, "source_filename", sound_source->source_buffer->filename);
		hashmap_bool_set(entity_data, "paused", sound_source_is_paused(game_state_get()->sound, sound_source));
		hashmap_int_set(entity_data, "sound_type", sound_source->type);
		hashmap_bool_set(entity_data, "loop", sound_source->loop);
		hashmap_float_set(entity_data, "volume", sound_source->volume);
		hashmap_float_set(entity_data, "sound_min_distance", sound_source->min_distance);
		hashmap_float_set(entity_data, "sound_max_distance", sound_source->max_distance);
		hashmap_float_set(entity_data, "rolloff_factor", sound_source->rolloff_factor);
		hashmap_int_set(entity_data, "sound_attenuation_type", sound_source->attenuation_type);
	}
	break;
	case ET_ENEMY:
	{
		struct Enemy* enemy = (struct Enemy*)entity;
		enemy_write(enemy, entity_data);
	}
	break;
	case ET_TRIGGER:
	{
		struct Trigger* trigger = (struct Trigger*)entity;
		hashmap_int_set(entity_data, "trigger_type", trigger->type);
		hashmap_int_set(entity_data, "trigger_mask", trigger->trigger_mask);
	}
	break;
	case ET_DOOR:
	{
		struct Door* door = (struct Door*)entity;
		door_write(door, entity_data);
	}
	break;
	case ET_PICKUP:
	{
		struct Pickup* pickup = (struct Pickup*)entity;
		pickup_write(pickup, entity_data);
	}
	break;
	};

	return true;
}

bool entity_save(struct Entity* entity, const char* filename, int directory_type)
{
	char prefixed_filename[MAX_FILENAME_LEN + 16];
	snprintf(prefixed_filename, MAX_FILENAME_LEN + 16, "entities/%s.symtres", filename);
    FILE* entity_file = io_file_open(directory_type, prefixed_filename, "w");
	if(!entity_file)
	{
		log_error("entity:save", "Failed to open entity file %s for writing");
		return false;
	}

    struct Parser* parser = parser_new();
    struct Parser_Object* object = parser_object_new(parser, PO_ENTITY);
    if(!entity_write(entity, object, false))
    {
        log_error("entity:save", "Failed to save entity : %s to file : %s", entity->name, prefixed_filename);
		parser_free(parser);
        fclose(entity_file);
        return false;
    }

	// See if the entity has any children, if it does,
	// write the entity first then, write all its children
	int num_children = array_len(entity->transform.children);
	for (int i = 0; i < num_children; i++)
	{
		struct Parser_Object* child_object = parser_object_new(parser, PO_ENTITY);
		struct Entity* child_entity = entity->transform.children[i];
		if(!entity_write(child_entity, child_object, true))
		{
			log_error("entity:save", "Failed to write child entity : %s for parent entity : %s to file : %s", entity->name, child_entity->name, prefixed_filename);
			parser_free(parser);
			fclose(entity_file);
			return false;
		}
	}

    if(parser_write_objects(parser, entity_file, prefixed_filename))
        log_message("Entity %s saved to %s", entity->name, prefixed_filename);

	//Update the entity's archetype index to the one we just saved
	entity->archetype_index = scene_entity_archetype_add(game_state_get()->scene, filename);
    parser_free(parser);
	fclose(entity_file);
	return true;
}

struct Entity* entity_read(struct Parser_Object* object, struct Entity* parent_entity)
{
	assert(object);

	struct Scene* scene = game_state_get()->scene;
	if(object->type != PO_ENTITY)
	{
		log_error("entity:read", "Invalid object type");
		return NULL;
	}

	const char* name = hashmap_str_get(object->data, "name");
	int type = hashmap_int_get(object->data, "type");
	if(!name)
	{
		log_error("entity:read", "No entity name provided");
		return NULL;
	}

	if(type < 0 || type >= ET_MAX)
	{
		log_error("entity:read", "Invalid entity type");
		return NULL;
	}

	struct Entity* new_entity = NULL;
	switch(type)
	{
	case ET_DEFAULT:
		new_entity = scene_entity_create(scene, name, parent_entity);
		break;
	case ET_CAMERA:
	{
		bool has_fbo = false;
		bool fbo_has_depth_tex = false;
		bool fbo_has_render_tex = false;
		int fbo_width = -1;
		int fbo_height = -1;
		struct Camera* camera = scene_camera_create(scene, name, parent_entity, 320, 240);
		if(!camera) return new_entity;
		if(hashmap_value_exists(object->data, "fov"))                camera->fov = hashmap_float_get(object->data, "fov");
		if(hashmap_value_exists(object->data, "resizeable"))         camera->resizeable = hashmap_bool_get(object->data, "resizeable");
		if(hashmap_value_exists(object->data, "zoom"))               camera->zoom = hashmap_float_get(object->data, "zoom");
		if(hashmap_value_exists(object->data, "nearz"))              camera->nearz = hashmap_float_get(object->data, "nearz");
		if(hashmap_value_exists(object->data, "farz"))               camera->farz = hashmap_float_get(object->data, "farz");
		if(hashmap_value_exists(object->data, "ortho"))              camera->ortho = hashmap_bool_get(object->data, "ortho");
		if(hashmap_value_exists(object->data, "has_fbo"))            has_fbo = hashmap_bool_get(object->data, "has_fbo");
		if(hashmap_value_exists(object->data, "fbo_has_depth_tex"))  fbo_has_depth_tex = hashmap_bool_get(object->data, "fbo_has_depth_tex");
		if(hashmap_value_exists(object->data, "fbo_has_render_tex")) fbo_has_render_tex = hashmap_bool_get(object->data, "fbo_has_render_tex");
		if(hashmap_value_exists(object->data, "fbo_width"))          fbo_width = hashmap_int_get(object->data, "fbo_width");
		if(hashmap_value_exists(object->data, "fbo_height"))         fbo_height = hashmap_int_get(object->data, "fbo_height");
		if(hashmap_value_exists(object->data, "clear_color"))
		{
			vec4 color = hashmap_vec4_get(object->data, "clear_color");
			vec4_assign(&camera->clear_color, &color);
		}

		float aspect_ratio = (float)fbo_width / (float)fbo_height;
		camera->aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;

		camera->fbo = -1;
		camera->render_tex = -1;
		camera->depth_tex = -1;

		if(has_fbo)
		{
			camera_attach_fbo(camera, fbo_width, fbo_height, fbo_has_depth_tex,	fbo_has_render_tex, camera->resizeable);
		}

		camera_update_proj(camera);
		camera_update_view(camera);
		new_entity = &camera->base;
	}
	break;
	case ET_LIGHT:
	{
		struct Light* light = scene_light_create(scene, name, parent_entity, LT_POINT);
		if(!light)
			return new_entity;
		if(hashmap_value_exists(object->data, "light_type"))  light->type        = hashmap_int_get(object->data, "light_type");
		if(hashmap_value_exists(object->data, "outer_angle")) light->outer_angle = hashmap_float_get(object->data, "outer_angle");
		if(hashmap_value_exists(object->data, "inner_angle")) light->inner_angle = hashmap_float_get(object->data, "inner_angle");
		if(hashmap_value_exists(object->data, "falloff"))     light->falloff     = hashmap_float_get(object->data, "falloff");
		if(hashmap_value_exists(object->data, "intensity"))   light->intensity   = hashmap_float_get(object->data, "intensity");
		if(hashmap_value_exists(object->data, "depth_bias"))  light->depth_bias  = hashmap_float_get(object->data, "depth_bias");
		if(hashmap_value_exists(object->data, "color"))       light->color       = hashmap_vec3_get(object->data, "color");
		if(hashmap_value_exists(object->data, "cast_shadow")) light->cast_shadow = hashmap_bool_get(object->data, "cast_shadow");
		if(hashmap_value_exists(object->data, "pcf_enabled")) light->pcf_enabled = hashmap_bool_get(object->data, "pcf_enabled");
		if(hashmap_value_exists(object->data, "radius"))      light->radius      = hashmap_int_get(object->data, "radius");
		new_entity = &light->base;
	}
	break;
	case ET_SOUND_SOURCE:
	{
		struct Sound_Source* sound_source = scene_sound_source_create(scene, name, parent_entity, "sounds/teh_beatz.wav", ST_WAV, true, true);
		if(!sound_source)
			return new_entity;
		struct Sound_Source_Buffer* default_source_buffer = sound_source->source_buffer;
		uint default_source_instance = sound_source->source_instance;

		if(hashmap_value_exists(object->data, "loop"))                   sound_source->loop             = hashmap_bool_get(object->data,  "loop");
		if(hashmap_value_exists(object->data, "sound_min_distance"))     sound_source->min_distance     = hashmap_float_get(object->data, "sound_min_distance");
		if(hashmap_value_exists(object->data, "sound_max_distance"))     sound_source->max_distance     = hashmap_float_get(object->data, "sound_max_distance");
		if(hashmap_value_exists(object->data, "volume"))                 sound_source->volume           = hashmap_float_get(object->data, "volume");
		if(hashmap_value_exists(object->data, "rolloff_factor"))         sound_source->rolloff_factor   = hashmap_float_get(object->data, "rolloff_factor");
		if(hashmap_value_exists(object->data, "sound_type"))             sound_source->type             = hashmap_int_get(object->data,   "sound_type");
		if(hashmap_value_exists(object->data, "sound_attenuation_type")) sound_source->attenuation_type = hashmap_int_get(object->data,   "sound_attenuation_type");
		if(hashmap_value_exists(object->data, "source_filename"))
		{
			struct Sound* sound = game_state_get()->sound;
			
			sound_source->source_buffer = sound_source_buffer_create(sound, hashmap_str_get(object->data, "source_filename"), sound_source->type);
			if(sound_source->source_buffer)
			{
				bool paused = hashmap_value_exists(object->data, "paused") ? hashmap_bool_get(object->data, "paused") : false;
				if(!paused)
				{
					sound_source->source_instance = sound_source_instance_create(sound, sound_source->source_buffer, true);

					vec3 abs_pos = { 0.f, 0.f,  0.f };
					transform_get_absolute_position(sound_source, &abs_pos);
					sound_source_instance_update_position(sound, sound_source->source_instance, abs_pos);

					sound_source_instance_loop_set(sound, sound_source->source_instance, sound_source->loop);
					sound_source_instance_min_max_distance_set(sound, sound_source->source_instance, sound_source->min_distance, sound_source->max_distance);
					sound_source_instance_attenuation_set(sound, sound_source->source_instance, sound_source->attenuation_type, sound_source->rolloff_factor);
					sound_source_instance_volume_set(sound, sound_source->source_instance, sound_source->volume);

					sound_update_3d(sound);
					sound_source_instance_play(sound, sound_source->source_instance);
				}

				//Stop the default sound source from playing now that we have loaded the actual buffer
				sound_source_instance_destroy(sound, default_source_instance);
			}
			else
			{
				log_error("Failed to create sound source from '%s'", hashmap_str_get(object->data, "source_filename"));
			}
		}
		else
		{
			log_error("entity:read", "No filename provided for sound source for entity '%s'", name);
		}
		new_entity = &sound_source->base;
	}
	break;
	case ET_STATIC_MESH:
	{
		const char* geometry_name = NULL;
		int material_type = MAT_UNSHADED;
		if(hashmap_value_exists(object->data, "geometry")) geometry_name = hashmap_str_get(object->data, "geometry");
		if(hashmap_value_exists(object->data, "material")) material_type = hashmap_int_get(object->data, "material");
		struct Static_Mesh* mesh = scene_static_mesh_create(scene, name, parent_entity, geometry_name, material_type);
		if(!mesh)
			return new_entity;
		new_entity = &mesh->base;

		//Set material model params for this particular mesh
		struct Model* model = &mesh->model;
		switch(model->material->type)
		{
		case MAT_BLINN:
			if(hashmap_value_exists(object->data, "diffuse_texture"))
			{
				const char* texture_name = hashmap_str_get(object->data, "diffuse_texture");
				model->material_params[MMP_DIFFUSE_TEX].val_int = texture_create_from_file(texture_name, TU_DIFFUSE);
			}
			if(hashmap_value_exists(object->data, "diffuse_color"))     model->material_params[MMP_DIFFUSE_COL].val_vec4        = hashmap_vec4_get(object->data, "diffuse_color");
			if(hashmap_value_exists(object->data, "diffuse"))           model->material_params[MMP_DIFFUSE].val_float           = hashmap_float_get(object->data, "diffuse");
			if(hashmap_value_exists(object->data, "specular"))          model->material_params[MMP_SPECULAR].val_float          = hashmap_float_get(object->data, "specular");
			if(hashmap_value_exists(object->data, "specular_strength")) model->material_params[MMP_SPECULAR_STRENGTH].val_float = hashmap_float_get(object->data, "specular_strength");
			if(hashmap_value_exists(object->data, "uv_scale"))          model->material_params[MMP_UV_SCALE].val_vec2           = hashmap_vec2_get(object->data, "uv_scale");
			break;
		case MAT_UNSHADED:
			if(hashmap_value_exists(object->data, "diffuse_color")) model->material_params[MMP_DIFFUSE_COL].val_vec4 = hashmap_vec4_get(object->data, "diffuse_color");
			if(hashmap_value_exists(object->data, "diffuse_texture"))
			{
				const char* texture_name = hashmap_str_get(object->data, "diffuse_texture");
				model->material_params[MMP_DIFFUSE_TEX].val_int = texture_create_from_file(texture_name, TU_DIFFUSE);
			}
			if(hashmap_value_exists(object->data, "uv_scale")) model->material_params[MMP_UV_SCALE].val_vec2 = hashmap_vec2_get(object->data, "uv_scale");
			break;
		};
	}
	break;
	case ET_ENEMY:
	{
		new_entity = &enemy_read(object, name, parent_entity)->base;
		if(!new_entity)
			return new_entity;
	}
	break;
	case ET_TRIGGER:
	{
		int type          = hashmap_value_exists(object->data, "trigger_type") ? hashmap_int_get(object->data, "trigger_type") : TRIG_TOGGLE;
		int mask          = hashmap_value_exists(object->data, "trigger_mask") ? hashmap_int_get(object->data, "trigger_mask") : TRIGM_ALL;
		struct Trigger* trigger = scene_trigger_create(scene, name, parent_entity, type, mask);
		if(!trigger)
			return new_entity;
		else
			new_entity = &trigger->base;
	}
	break;
	case ET_DOOR:
	{
		new_entity = &door_read(object, name, parent_entity)->base;
		if(!new_entity)
			return new_entity;
	}
	break;
	case ET_PICKUP:
	{
		new_entity = &pickup_read(object, name, parent_entity)->base;
		if(!new_entity)
			return new_entity;
	}
	break;
	default:
		log_warning("Unhandled Entity type '%d' detected", type);
		return new_entity;
		break;
	}
	
	vec3 position = { 0.f, 0.f, 0.f };
	quat rotation = { 0.f, 0.f, 0.f, 1.f };
	vec3 scale    = { 1.f, 1.f, 1.f };

	if(hashmap_value_exists(object->data, "position")) position = hashmap_vec3_get(object->data, "position");
	if(hashmap_value_exists(object->data, "rotation")) rotation = hashmap_quat_get(object->data, "rotation");
	if(hashmap_value_exists(object->data, "scale"))    scale = hashmap_vec3_get(object->data, "scale");

	transform_set_position(new_entity, &position);
	transform_scale(new_entity, &scale);
	quat_mul(&new_entity->transform.rotation, &new_entity->transform.rotation, &rotation);

	if(new_entity->type != ET_STATIC_MESH)
	{
		if(hashmap_value_exists(object->data, "bounding_box_min")) new_entity->bounding_box.min = hashmap_vec3_get(object->data, "bounding_box_min");
		if(hashmap_value_exists(object->data, "bounding_box_max")) new_entity->bounding_box.max = hashmap_vec3_get(object->data, "bounding_box_max");
	}

	if(hashmap_value_exists(object->data, "flags")) new_entity->flags = hashmap_uint_get(object->data, "flags");

	transform_update_transmat(new_entity);
	if(hashmap_value_exists(object->data, "archetype")) new_entity->archetype_index = scene_entity_archetype_add(scene, hashmap_str_get(object->data, "archetype"));

	return new_entity;
}

struct Entity* entity_load(const char* filename, int directory_type, bool send_on_load_event)
{
	char prefixed_filename[MAX_FILENAME_LEN + 16];
	snprintf(prefixed_filename, MAX_FILENAME_LEN + 16, "entities/%s.symtres", filename);
    FILE* entity_file = io_file_open(directory_type, prefixed_filename, "rb");
	if(!entity_file)
	{
		log_error("entity:load", "Failed to open entity file %s for reading", prefixed_filename);
		return false;
	}

	struct Parser* parsed_file = parser_load_objects(entity_file, prefixed_filename);
	struct Entity* new_entity = false;

	if(!parsed_file)
	{
		log_error("entity:load", "Failed to parse file '%s' for entity definition", prefixed_filename);
		fclose(entity_file);
		return false;
	}

	if(array_len(parsed_file->objects) == 0)
	{
		log_error("entity:load", "No objects found in file %s", prefixed_filename);
		parser_free(parsed_file);
		fclose(entity_file);
		return false;
	}

	int num_entites_loaded = 0;
	struct Entity* parent_entity = NULL;
	for(int i = 0; i < array_len(parsed_file->objects); i++)
	{
		struct Parser_Object* object = &parsed_file->objects[i];
		if(object->type != PO_ENTITY) continue;

		new_entity = entity_read(object, parent_entity);
		if(new_entity)
		{
			if(i == 0)
			{
				new_entity->archetype_index = scene_entity_archetype_add(game_state_get()->scene, filename);
				parent_entity = new_entity;
			}
			// We don't want this entity to be saved when we're saving the scene because 
			// this entity and its parent will be loaded from this same file next time the scene is loaded
			if(i != 0)
				new_entity->flags |= EF_TRANSIENT;
			num_entites_loaded++;
			log_message("Entity %s loaded from %s", new_entity->name, prefixed_filename);
		}
		else
		{
			log_error("entity:load", "Failed to load entity from %s", prefixed_filename);
		}
	}	

	// Send simulated scene loaded event since this entity has been loaded while the scene has already been loaded
	if(send_on_load_event)
	{
		struct Game_State* game_state = game_state_get();
		struct Event_Manager* event_manager = game_state->event_manager;
		struct Event on_scene_loaded_event =
		{
			.type = EVT_SCENE_LOADED,
		};
		strncpy(on_scene_loaded_event.scene_load.filename, game_state->scene->filename, MAX_FILENAME_LEN);
		event_manager_send_event_entity(event_manager, &on_scene_loaded_event, parent_entity);
	}

	parser_free(parsed_file);
	fclose(entity_file);
	return parent_entity;
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
	case ET_ENEMY:        typename = "Enemy";        break;
	case ET_TRIGGER:      typename = "Trigger";      break;
	case ET_DOOR:         typename = "Door";         break;
	case ET_PICKUP:       typename = "Pickup";       break;
	default:              typename = "Unknown";      break;
	};
	return typename;
}

void entity_rename(struct Entity* entity, const char* new_name)
{
	assert(entity);
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
	snprintf(entity->name, MAX_ENTITY_NAME_LEN, new_name);
}

int entity_get_num_children_of_type(struct Entity* entity, int type, struct Entity** in_children, int max_children)
{
	assert(type < ET_MAX && max_children >= 1);

	int found_count = -1;
	for(int i = 0; i < array_len(entity->transform.children); i++)
	{
		struct Entity* child = entity->transform.children[i];
		if(!(child->flags & EF_MARKED_FOR_DELETION) && (child->flags & EF_ACTIVE) && child->type == type)
		{
			found_count++;
			if(found_count == max_children)
				break;
			in_children[found_count] = child;
		}
	}
	return found_count + 1;
}
