#include "scene.h"
#include "../common/array.h"
#include "entity.h"
#include "../common/log.h"
#include "transform.h"
#include "camera.h"
#include "../common/parser.h"
#include "model.h"
#include "light.h"
#include "player.h"
#include "game.h"
#include "bounding_volumes.h"
#include "geometry.h"
#include "editor.h"
#include "../system/sound.h"
#include "../system/physics.h"
#include "../system/platform.h"
#include "../common/hashmap.h"
#include "renderer.h"
#include "sound_source.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static void scene_write_entity_entry(struct Scene* scene, struct Entity* entity, struct Parser* parser);
static void scene_write_entity_list(struct Scene* scene, int entity_type, struct Parser* parser);

void scene_init(struct Scene* scene)
{
	assert(scene);
	struct Game_State* game_state = game_state_get();

	strncpy(scene->filename, "UNNAMED_SCENE", MAX_FILENAME_LEN);
	//Initialize the root entity
	entity_init(&scene->root_entity, "ROOT_ENTITY", NULL);
	scene->root_entity.flags |= EF_ACTIVE;
	scene->root_entity.id = 0;
	scene->root_entity.type = ET_ROOT;

	for(int i = 0; i < MAX_ENTITIES; i++) entity_reset(&scene->entities[i], i);
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		entity_reset(&scene->lights[i], i);
		scene->lights[i].type = ET_LIGHT;
	}
	
	for(int i = 0; i < MAX_STATIC_MESHES; i++)
	{
		entity_reset(&scene->static_meshes[i], i);
		struct Static_Mesh* mesh = &scene->static_meshes[i];
		mesh->collision.collision_shape = NULL;
		mesh->collision.rigidbody = NULL;
		mesh->collision.on_collision = NULL;
		mesh->model.geometry_index = -1;
		mesh->model.material = NULL;
	}

	for(int i = 0; i < MAX_SOUND_SOURCES; i++) entity_reset(&scene->sound_sources[i], i);
	int width = 1280, height = 720;
	window_get_drawable_size(game_state->window, &width, &height);

	for(int i = 0; i < MAX_CAMERAS; i++)
	{
		entity_init(&scene->cameras[i], NULL, &scene->root_entity);
		camera_init(&scene->cameras[i], width, height);
		scene->cameras[i].base.flags &= ~EF_ACTIVE;
		scene->cameras[i].base.id = i;
	}

	for(int i = 0; i < MAX_ENTITY_ARCHETYPES; i++)
		memset(&scene->entity_archetypes[i][0], '\0', MAX_FILENAME_LEN);

	player_init(&scene->player, scene);
	editor_camera_init(game_state->editor, game_state->cvars);
	editor_init_entities(game_state->editor);

	scene->active_camera_index = game_state_get()->game_mode == GAME_MODE_GAME ? CAM_GAME : CAM_EDITOR;
}

bool scene_load(struct Scene* scene, const char* filename, int directory_type)
{
	char prefixed_filename[MAX_FILENAME_LEN + 16];
	snprintf(prefixed_filename, MAX_FILENAME_LEN + 16, "scenes/%s.symtres", filename);
    FILE* scene_file = io_file_open(directory_type, prefixed_filename, "rb");
	if(!scene_file)
	{
		log_error("scene:load", "Failed to open scene file %s for reading");
		return false;
	}

	// Load scene config and apply renderer settings
	struct Parser* parsed_file = parser_load_objects(scene_file, prefixed_filename);

	if(!parsed_file)
	{
		log_error("scene:load", "Failed to parse file '%s' for loading scene", prefixed_filename);
		fclose(scene_file);
		return false;
	}

	if(array_len(parsed_file->objects) == 0)
	{
		log_error("scene:load", "No objects found in file %s", prefixed_filename);
		parser_free(parsed_file);
		fclose(scene_file);
		return false;
	}

	//Clear previous scene and re-initialize
	scene_destroy(scene);
	scene_post_update(scene);
	scene_init(scene);

	int num_objects_loaded = 0;
	for(int i = 0; i < array_len(parsed_file->objects); i++)
	{
		struct Parser_Object* object = &parsed_file->objects[i];
		switch(object->type)
		{
		case PO_SCENE_CONFIG:
		{
			struct Hashmap* scene_data = object->data;
			struct Render_Settings* render_settings = &game_state_get()->renderer->settings;

			if(hashmap_value_exists(scene_data, "fog_type"))           render_settings->fog.mode           = hashmap_int_get(scene_data, "fog_type");
			if(hashmap_value_exists(scene_data, "fog_density"))        render_settings->fog.density        = hashmap_float_get(scene_data, "fog_density");
			if(hashmap_value_exists(scene_data, "fog_start_distance")) render_settings->fog.start_dist     = hashmap_float_get(scene_data, "fog_start_distance");
			if(hashmap_value_exists(scene_data, "fog_max_distance"))   render_settings->fog.max_dist       = hashmap_float_get(scene_data, "fog_max_distance");
			if(hashmap_value_exists(scene_data, "fog_color"))          render_settings->fog.color          = hashmap_vec3_get(scene_data, "fog_color");
			if(hashmap_value_exists(scene_data, "ambient_light"))      render_settings->ambient_light      = hashmap_vec3_get(scene_data, "ambient_light");
			if(hashmap_value_exists(scene_data, "debug_draw_color"))   render_settings->debug_draw_color   = hashmap_vec4_get(scene_data, "debug_draw_color");
			if(hashmap_value_exists(scene_data, "debug_draw_enabled")) render_settings->debug_draw_enabled = hashmap_bool_get(scene_data, "debug_draw_enabled");
			if(hashmap_value_exists(scene_data, "debug_draw_mode"))    render_settings->debug_draw_mode    = hashmap_int_get(scene_data, "debug_draw_mode");
			if(hashmap_value_exists(scene_data, "debug_draw_physics")) render_settings->debug_draw_physics = hashmap_bool_get(scene_data, "debug_draw_physics");
			num_objects_loaded++;
		}
		break;
		case PO_ENTITY:
		{
			if(entity_read(object, &scene->root_entity))
				num_objects_loaded++;
		}
		break;
		case PO_SCENE_ENTITY_ENTRY:
		{
			struct Hashmap* entity_entry_data = object->data;
			if(hashmap_value_exists(object->data, "filename"))
			{
				struct Entity* loaded_entity = entity_load(hashmap_str_get(entity_entry_data, "filename"), DIRT_INSTALL);
				if(loaded_entity)
				{
					vec3 position = { 0.f, 0.f, 0.f };
					quat rotation = { 0.f, 0.f, 0.f, 1.f };
					vec3 scale    = { 1.f, 1.f, 1.f };

					if(hashmap_value_exists(entity_entry_data, "position")) position = hashmap_vec3_get(entity_entry_data, "position");
					if(hashmap_value_exists(entity_entry_data, "rotation")) rotation = hashmap_quat_get(entity_entry_data, "rotation");
					if(hashmap_value_exists(entity_entry_data, "scale"))    scale    = hashmap_vec3_get(entity_entry_data, "scale");

					transform_set_position(loaded_entity, &position);
					transform_scale(loaded_entity, &scale);
					quat_assign(&loaded_entity->transform.rotation, &rotation);
					transform_update_transmat(loaded_entity);

					if(hashmap_value_exists(entity_entry_data, "name")) strncpy(loaded_entity->name, hashmap_str_get(entity_entry_data, "name"), MAX_ENTITY_NAME_LEN);
					num_objects_loaded++;
				}
			}
		}
		break;
		case PO_PLAYER:
		{
			struct Hashmap* player_data = object->data;
			vec3 position = { 0.f, 0.f, 0.f };
			quat rotation = { 0.f, 0.f, 0.f, 1.f };
			vec3 scale = { 1.f, 1.f, 1.f };

			if(hashmap_value_exists(player_data, "position")) position = hashmap_vec3_get(player_data, "position");
			if(hashmap_value_exists(player_data, "rotation")) rotation = hashmap_quat_get(player_data, "rotation");
			if(hashmap_value_exists(player_data, "scale"))    scale    = hashmap_vec3_get(player_data, "scale");

			struct Player* player = &scene->player;
			transform_set_position(player, &position);
			transform_scale(player, &scale);
			quat_assign(&player->base.transform.rotation, &rotation);
			transform_update_transmat(player);

			if(hashmap_value_exists(player_data, "camera_clear_color")) player->camera->clear_color = hashmap_vec4_get(player_data, "camera_clear_color");
			num_objects_loaded++;
		}
		break;
		default:
			log_warning("Unknown object type '%s' in scene file %s", parser_object_type_to_str(object->type), prefixed_filename);
			continue;
		}
	}

	parser_free(parsed_file);
	fclose(scene_file);
	strncpy(scene->filename, filename, MAX_FILENAME_LEN);
	return num_objects_loaded > 0 ? true : false;
}

bool scene_save(struct Scene* scene, const char* filename, int directory_type)
{
	char prefixed_filename[MAX_FILENAME_LEN + 16];
	snprintf(prefixed_filename, MAX_FILENAME_LEN + 16, "scenes/%s.symtres", filename);
    FILE* scene_file = io_file_open(directory_type, prefixed_filename, "w");
	if(!scene_file)
	{
		log_error("scene:save", "Failed to open scene file %s for writing");
		return false;
	}

    struct Parser* parser = parser_new();

	// Scene configuration
    struct Parser_Object* scene_config_object = parser_object_new(parser, PO_SCENE_CONFIG);
	struct Render_Settings* render_settings = &game_state_get()->renderer->settings;
	struct Hashmap* scene_data = scene_config_object->data;
	hashmap_int_set(scene_data, "fog_type", render_settings->fog.mode);
	hashmap_float_set(scene_data, "fog_density", render_settings->fog.density);
	hashmap_float_set(scene_data, "fog_start_distance", render_settings->fog.start_dist);
	hashmap_float_set(scene_data, "fog_max_distance", render_settings->fog.max_dist);
	hashmap_vec3_set(scene_data, "fog_color", &render_settings->fog.color);
	hashmap_vec3_set(scene_data, "ambient_light", &render_settings->ambient_light);
	hashmap_vec4_set(scene_data, "debug_draw_color", &render_settings->debug_draw_color);
	hashmap_bool_set(scene_data, "debug_draw_enabled", render_settings->debug_draw_enabled);
	hashmap_int_set(scene_data, "debug_draw_mode", render_settings->debug_draw_mode);
	hashmap_bool_set(scene_data, "debug_draw_physics", render_settings->debug_draw_physics);

	// Player
	struct Parser_Object* player_object = parser_object_new(parser, PO_PLAYER);
	entity_write(&scene->player, player_object, true);
	hashmap_vec4_set(player_object->data, "camera_clear_color", &scene->player.camera->clear_color);

	scene_write_entity_list(scene, ET_DEFAULT, parser);
	scene_write_entity_list(scene, ET_LIGHT, parser);
	scene_write_entity_list(scene, ET_STATIC_MESH, parser);
	scene_write_entity_list(scene, ET_CAMERA, parser);
	scene_write_entity_list(scene, ET_SOUND_SOURCE, parser);

    if(parser_write_objects(parser, scene_file, prefixed_filename))
        log_message("Scene saved to %s", prefixed_filename);

    parser_free(parser);
	fclose(scene_file);
	return true;
}

void scene_write_entity_list(struct Scene* scene, int entity_type, struct Parser* parser)
{
	int max_length = 0;
	size_t stride = 0;
	struct Entity* entity = NULL;

	switch(entity_type)
	{
	case ET_DEFAULT:
		max_length = MAX_ENTITIES;
		entity = &scene->entities[0];
		stride = sizeof(struct Entity);
		break;
	case ET_LIGHT:
		max_length = MAX_LIGHTS;
		entity = &scene->lights[0].base;
		stride = sizeof(struct Light);
		break;
	case ET_STATIC_MESH:
		max_length = MAX_STATIC_MESHES;
		entity = &scene->static_meshes[0].base;
		stride = sizeof(struct Static_Mesh);
		break;
	case ET_CAMERA:
		max_length = MAX_CAMERAS;
		entity = &scene->cameras[0].base;
		stride = sizeof(struct Camera);
		break;
	case ET_SOUND_SOURCE:
		max_length = MAX_SOUND_SOURCES;
		entity = &scene->sound_sources[0].base;
		stride = sizeof(struct Sound_Source);
		break;
	default: return;
	}

	size_t count = 0;
	while(count < max_length)
	{
		//((char*)entity) += stride * count;
		
		if(!(entity->flags & EF_TRANSIENT) && (entity->flags & EF_ACTIVE))
		{
			if(entity->archetype_index != -1)
			{
				scene_write_entity_entry(scene, entity, parser);
			}
			else
			{
				struct Parser_Object* object = parser_object_new(parser, PO_ENTITY);
				if(!entity_write(entity, object, true))
					log_error("scene:save", "Failed to save entity : %s", entity->name);
			}
		}
		count++;
		((char*)entity) += stride;
	}
}

void scene_write_entity_entry(struct Scene* scene, struct Entity* entity, struct Parser* parser)
{
	// For entities with archetypes, we only write the name of the archetype to load 
	// them from and their transformation info
	struct Parser_Object* object = parser_object_new(parser, PO_SCENE_ENTITY_ENTRY);
	hashmap_str_set(object->data, "filename", &scene->entity_archetypes[entity->archetype_index][0]);
	hashmap_str_set(object->data, "name", entity->name);
	hashmap_vec3_set(object->data, "position", &entity->transform.position);
	hashmap_vec3_set(object->data, "scale", &entity->transform.scale);
	hashmap_quat_set(object->data, "rotation", &entity->transform.rotation);
}

void scene_destroy(struct Scene* scene)
{
	assert(scene);

	for(int i = 0; i < MAX_ENTITIES; i++)          scene_entity_base_remove(scene, &scene->entities[i]);
	for(int i = 0; i < MAX_CAMERAS; i++)           scene_camera_remove(scene, &scene->cameras[i]);
	for(int i = 0; i < MAX_LIGHTS; i++)            scene_light_remove(scene, &scene->lights[i]);
	for(int i = 0; i < MAX_STATIC_MESHES; i++)     scene_static_mesh_remove(scene, &scene->static_meshes[i]);
	for(int i = 0; i < MAX_SOUND_SOURCES; i++)     scene_sound_source_remove(scene, &scene->sound_sources[i]);
	for(int i = 0; i < MAX_ENTITY_ARCHETYPES; i++) memset(&scene->entity_archetypes[i][0], '\0', MAX_FILENAME_LEN);
	player_destroy(&scene->player);
	entity_reset(&scene->root_entity, 0);
	scene->root_entity.flags &= ~EF_ACTIVE;
}

void scene_update(struct Scene* scene, float dt)
{
	if(game_state_get()->game_mode == GAME_MODE_GAME) player_update(&scene->player, scene, dt);
}

void scene_post_update(struct Scene* scene)
{
	assert(scene);
	struct Sound* sound = game_state_get()->sound;

	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		struct Entity* entity = &scene->entities[i];
		if(!(entity->flags & EF_ACTIVE)) continue;

		if(entity->flags & EF_MARKED_FOR_DELETION)
		{
			scene_entity_base_remove(scene, entity);
			continue;
		}

		if(entity->transform.is_modified) entity->transform.is_modified = false;
	}

	for(int i = 0; i < MAX_CAMERAS; i++)
	{
		struct Camera* camera = &scene->cameras[i];
		if(!(camera->base.flags & EF_ACTIVE)) continue;

		if(camera->base.flags & EF_MARKED_FOR_DELETION)
		{
			scene_camera_remove(scene, camera);
			continue;
		}

		if(camera->base.transform.is_modified)
		{
			camera_update_view(camera);
			camera->base.transform.is_modified = false;
		}
	}

	for(int i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		struct Sound_Source* sound_source = &scene->sound_sources[i];
		if(!(sound_source->base.flags & EF_ACTIVE)) continue;

		if(sound_source->base.flags & EF_MARKED_FOR_DELETION)
		{
			scene_sound_source_remove(scene, sound_source);
			continue;
		}

		if(sound_source->base.transform.is_modified)
		{
			sound_source_update(sound, sound_source);
			sound_source->base.transform.is_modified = false;
		}
	}

	for(int i = 0; i < MAX_STATIC_MESHES; i++)
	{
		struct Static_Mesh* static_mesh = &scene->static_meshes[i];
		if(!(static_mesh->base.flags & EF_ACTIVE)) continue;

		if(static_mesh->base.flags & EF_MARKED_FOR_DELETION)
		{
			scene_static_mesh_remove(scene, static_mesh);
			continue;
		}

		if(static_mesh->base.transform.is_modified)
		{
			if(static_mesh->collision.rigidbody && static_mesh->base.transform.sync_physics)
			{
				quat abs_rot = { 0.f, 0.f, 0.f, 1.f };
				vec3 abs_pos = { 0.f, 0.f,  0.f };
				transform_get_absolute_rot(&static_mesh->base, &abs_rot);
				transform_get_absolute_position(&static_mesh->base, &abs_pos);
				physics_body_rotation_set(static_mesh->collision.rigidbody, abs_rot.x, abs_rot.y, abs_rot.z, abs_rot.w);
				physics_body_position_set(static_mesh->collision.rigidbody, abs_pos.x, abs_pos.y, abs_pos.z);
			}
			static_mesh->base.transform.sync_physics = false;
			static_mesh->base.transform.is_modified = false;
		}
	}

	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		struct Light* light = &scene->lights[i];
		if(!(light->base.flags & EF_ACTIVE)) continue;

		if(light->base.flags & EF_MARKED_FOR_DELETION)
		{
			scene_light_remove(scene, light);
			continue;
		}

		if(light->base.transform.is_modified) light->base.transform.is_modified = false;
	}

	if(scene->player.base.transform.is_modified)
	{
		scene->player.base.transform.is_modified = false;
	}
}

struct Entity* scene_entity_create(struct Scene* scene, const char* name, struct Entity* parent)
{
	assert(scene);

	struct Entity* new_entity = NULL;
	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		struct Entity* entity = &scene->entities[i];
		if(!(entity->flags & EF_ACTIVE))
		{
			new_entity = entity;
			break;
		}
	}

	if(new_entity)
	{
		if(!parent)
			parent = &scene->root_entity;
		entity_init(new_entity, name, parent);
	}
	else
	{
		log_error("scene:entity_create", "Max entity limit reached!");
	}

	return new_entity;
}

struct Light* scene_light_create(struct Scene* scene, const char* name, struct Entity* parent, int light_type)
{
	assert(scene);
	struct Light* new_light = NULL;
	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		struct Light* light = &scene->lights[i];
		if(!(light->base.flags & EF_ACTIVE))
		{
			new_light = light;
			break;
		}
	}

	if(new_light)
	{
		entity_init(&new_light->base, name, parent ? parent : &scene->root_entity);
		new_light->base.type = ET_LIGHT;
		light_init(new_light, light_type);
	}
	else
	{
		log_error("scene:light_create", "Max light limit reached!");
	}

	return new_light;
}

struct Camera* scene_camera_create(struct Scene* scene, const char* name, struct Entity* parent, int width, int height)
{
	assert(scene);
	struct Camera* new_camera = NULL;
	for(int i = 0; i < MAX_CAMERAS; i++)
	{
		struct Camera* camera = &scene->cameras[i];
		if(!(camera->base.flags & EF_ACTIVE))
		{
			new_camera = camera;
			break;
		}
	}

	if(new_camera)
	{
		entity_init(&new_camera->base, name, parent ? parent : &scene->root_entity);
		new_camera->base.type = ET_CAMERA;
		camera_init(new_camera, width, height);
	}
	else
	{
		log_error("scene:camera_create", "Max camera limit reached!");
	}

	return new_camera;
}

struct Static_Mesh* scene_static_mesh_create(struct Scene* scene, const char* name, struct Entity* parent, const char* geometry_name, int material_type)
{
	assert(scene);
	struct Static_Mesh* new_static_mesh = NULL;
	for(int i = 0; i < MAX_STATIC_MESHES; i++)
	{
		struct Static_Mesh* static_mesh = &scene->static_meshes[i];
		if(!(static_mesh->base.flags & EF_ACTIVE))
		{
			new_static_mesh = static_mesh;
			break;
		}
	}

	if(new_static_mesh)
	{
		entity_init(&new_static_mesh->base, name, parent ? parent : &scene->root_entity);
		new_static_mesh->base.type = ET_STATIC_MESH;
		model_init(&new_static_mesh->model, new_static_mesh, geometry_name, material_type);
		vec3_assign(&new_static_mesh->base.bounding_box.min, &geom_get(new_static_mesh->model.geometry_index)->bounding_box.min);
		vec3_assign(&new_static_mesh->base.bounding_box.max, &geom_get(new_static_mesh->model.geometry_index)->bounding_box.max);
		entity_update_derived_bounding_box(new_static_mesh);
	}
	else
	{
		log_error("scene:model_create", "Max model limit reached!");
	}

	return new_static_mesh;
}

struct Sound_Source* scene_sound_source_create(struct Scene* scene, const char* name, struct Entity* parent, const char* filename, int type, bool loop, bool play)
{
	assert(scene && filename);
	struct Sound* sound = game_state_get()->sound;
	struct Sound_Source* new_sound_source = NULL;
	for(int i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		struct Sound_Source* sound_source = &scene->sound_sources[i];
		if(!(sound_source->base.flags & EF_ACTIVE))
		{
			new_sound_source = sound_source;
			break;
		}
	}

	if(new_sound_source)
	{
		entity_init(&new_sound_source->base, name, parent ? parent : &scene->root_entity);
		new_sound_source->base.type = ET_SOUND_SOURCE;
		struct Entity* entity = &new_sound_source->base;

		new_sound_source->source_buffer = sound_source_create(sound, filename, type);
		if(!new_sound_source->source_buffer)
		{
			log_error("scene:sound_source_create", "Failed to load file '%s' to provide sound source for entity %s", filename, entity->name);
			new_sound_source->source_instance = 0;
			return new_sound_source;
		}

		new_sound_source->source_instance = sound_source_instance_create(sound, new_sound_source->source_buffer, true);
		vec3 abs_pos = { 0.f, 0.f,  0.f };
		transform_get_absolute_position(entity, &abs_pos);
		sound_source_instance_update_position(sound, new_sound_source->source_instance, abs_pos);

		new_sound_source->loop = loop;
		new_sound_source->min_distance = 0.f;
		new_sound_source->max_distance = 10.f;
		new_sound_source->playing = play;
		new_sound_source->attenuation_type = SA_LINEAR;
		new_sound_source->rolloff_factor = 0.95f;
		new_sound_source->volume = 1.f;
		new_sound_source->type = type;

		sound_source_instance_loop_set(sound, new_sound_source->source_instance, new_sound_source->loop);
		sound_source_instance_min_max_distance_set(sound, new_sound_source->source_instance, new_sound_source->min_distance, new_sound_source->max_distance);
		sound_source_instance_attenuation_set(sound, new_sound_source->source_instance, new_sound_source->attenuation_type, new_sound_source->rolloff_factor);
		sound_source_instance_volume_set(sound, new_sound_source->source_instance, new_sound_source->volume);

		sound_update_3d(sound);
		if(new_sound_source->playing) sound_source_instance_play(sound, new_sound_source->source_instance);
	}
	else
	{
		log_error("scene:sound_source_create", "Max sound source limit reached!");
	}

	return new_sound_source;
}

void scene_entity_base_remove(struct Scene* scene, struct Entity* entity)
{
	assert(scene && entity && entity->id >= 0);

	if(!(entity->flags & EF_ACTIVE)) return;

	transform_destroy(entity);
	entity->flags = EF_NONE;
	memset(entity->name, '\0', MAX_ENTITY_NAME_LEN);
}

void scene_light_remove(struct Scene* scene, struct Light* light)
{
	assert(scene && light);
	scene_entity_base_remove(scene, &light->base);
	light->valid = false;
}

void scene_camera_remove(struct Scene* scene, struct Camera* camera)
{
	assert(scene && camera);
	camera_reset(camera);
	scene_entity_base_remove(scene, &camera->base);
}

void scene_static_mesh_remove(struct Scene* scene, struct Static_Mesh* mesh)
{
	assert(scene && mesh);

	mesh->collision.on_collision = NULL;
	if(mesh->collision.collision_shape) physics_cs_remove(mesh->collision.collision_shape);
	if(mesh->collision.rigidbody) physics_body_remove(mesh->collision.rigidbody);

	model_reset(&mesh->model, mesh);
	scene_entity_base_remove(scene, &mesh->base);
}

void scene_sound_source_remove(struct Scene* scene, struct Sound_Source* source)
{
	assert(scene && source);

	sound_source_instance_destroy(game_state_get()->sound, source->source_instance);
	source->source_instance = 0;
	scene_entity_base_remove(scene, &source->base);
}

struct Entity* scene_entity_find(struct Scene* scene, const char* name)
{
	assert(scene && name);
	struct Entity* entity = NULL;

	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		if(strncmp(name, scene->entities[i].name, MAX_ENTITY_NAME_LEN) == 0)
		{
			entity = &scene->entities[i];
			break;
		}
	}

	return entity;
}

struct Light* scene_light_find(struct Scene* scene, const char* name)
{
	assert(scene && name);
	struct Light* light = NULL;

	for(int i = 0; i < MAX_LIGHTS; i++)
	{
		if(strncmp(name, scene->lights[i].base.name, MAX_ENTITY_NAME_LEN) == 0)
		{
			light = &scene->lights[i];
			break;
		}
	}

	return light;
}

struct Camera* scene_camera_find(struct Scene* scene, const char* name)
{
	assert(scene && name);
	struct Camera* camera = NULL;

	for(int i = 0; i < MAX_CAMERAS; i++)
	{
		if(strncmp(name, scene->cameras[i].base.name, MAX_ENTITY_NAME_LEN) == 0)
		{
			camera = &scene->cameras[i];
			break;
		}
	}

	return camera;
}

struct Static_Mesh* scene_static_mesh_find(struct Scene* scene, const char* name)
{
	assert(scene && name);
	struct Static_Mesh* static_mesh = NULL;

	for(int i = 0; i < MAX_STATIC_MESHES; i++)
	{
		if(strncmp(name, scene->static_meshes[i].base.name, MAX_ENTITY_NAME_LEN) == 0)
		{
			static_mesh = &scene->static_meshes[i];
			break;
		}
	}

	return static_mesh;
}

struct Sound_Source* scene_sound_source_find(struct Scene* scene, const char* name)
{
	assert(scene && name);
	struct Sound_Source* sound_source = NULL;

	for(int i = 0; i < MAX_SOUND_SOURCES; i++)
	{
		if(strncmp(name, scene->sound_sources[i].base.name, MAX_ENTITY_NAME_LEN) == 0)
		{
			sound_source = &scene->sound_sources[i];
			break;
		}
	}

	return sound_source;
}

struct Entity* scene_base_entity_get(struct Scene* scene, int id, int type)
{
	assert(scene && id != -1 && type < ET_MAX);

	struct Entity* entity = NULL;

	switch(type)
	{
	case ET_DEFAULT:      entity = &scene->entities[id];      break;
	case ET_CAMERA:       entity = &scene->cameras[id];       break;
	case ET_LIGHT:        entity = &scene->lights[id];        break;
	case ET_STATIC_MESH:  entity = &scene->static_meshes[id]; break;
	case ET_SOUND_SOURCE: entity = &scene->sound_sources[id]; break;
	case ET_PLAYER:       entity = &scene->player;            break;
	case ET_ROOT:         entity = &scene->root_entity;       break;
	}

	return entity;
}

void* scene_find(struct Scene* scene, const char* name)
{
	void* entity = NULL;

	entity = scene_entity_find(scene, name);
	if(entity) return entity;

	entity = scene_light_find(scene, name);
	if(entity) return entity;

	entity = scene_camera_find(scene, name);
	if(entity) return entity;

	entity = scene_static_mesh_find(scene, name);
	if(entity) return entity;

	entity = scene_sound_source_find(scene, name);
	if(entity) return entity;

	return entity;
}

void scene_entity_parent_reset(struct Scene* scene, struct Entity* entity)
{
	assert(scene && entity);
	transform_parent_set(entity, &scene->root_entity, true);
}

void scene_entity_parent_set(struct Scene* scene, struct Entity* entity, struct Entity* parent)
{
	assert(scene && entity && parent && entity != parent);
	transform_parent_set(entity, parent, true);
}

void scene_ray_intersect(struct Scene* scene, struct Ray* ray, struct Raycast_Result* out_results, int ray_mask)
{
	assert(out_results);

	memset(&out_results[0], '\0', sizeof(struct Entity*) * MAX_RAYCAST_ENTITIES_INTERSECT);
	out_results->num_entities_intersected = 0;

	for(int i = 0; i < ET_MAX; i++)
	{
		int max_length = 0;
		size_t stride = 0;
		struct Entity* entity = NULL;

		switch(i)
		{
		case ET_DEFAULT:
			if(!(ray_mask & ERM_DEFAULT)) continue;
			max_length = MAX_ENTITIES;
			entity = &scene->entities[0];
			stride = sizeof(struct Entity);
			break;
		case ET_LIGHT:
			if(!(ray_mask & ERM_LIGHT)) continue;
			max_length = MAX_LIGHTS;
			entity = &scene->lights[0].base;
			stride = sizeof(struct Light);
			break;
		case ET_STATIC_MESH:
			if(!(ray_mask & ERM_STATIC_MESH)) continue;
			max_length = MAX_STATIC_MESHES;
			entity = &scene->static_meshes[0].base;
			stride = sizeof(struct Static_Mesh);
			break;
		case ET_CAMERA:
			if(!(ray_mask & ERM_CAMERA)) continue;
			max_length = MAX_CAMERAS;
			entity = &scene->cameras[0].base;
			stride = sizeof(struct Camera);
			break;
		case ET_SOUND_SOURCE:
			if(!(ray_mask & ERM_SOUND_SOURCE)) continue;
			max_length = MAX_SOUND_SOURCES;
			entity = &scene->sound_sources[0].base;
			stride = sizeof(struct Sound_Source);
			break;
		case ET_PLAYER:
			if(!(ray_mask & ERM_PLAYER)) continue;
			max_length = 1;
			entity = &scene->player;
			stride = sizeof(struct Player);
			break;
		default: continue;
		}

		size_t count = 0;
		while(count < max_length)
		{
			if(entity->flags & EF_ACTIVE && !(entity->flags & EF_IGNORE_RAYCAST))
			{
				int result = bv_intersect_bounding_box_ray(&entity->derived_bounding_box, ray);
				if(result == IT_INTERSECT || result == IT_INSIDE)
				{
					out_results->entities_intersected[out_results->num_entities_intersected] = entity;
					out_results->num_entities_intersected++;
					if(out_results->num_entities_intersected >= MAX_RAYCAST_ENTITIES_INTERSECT)
					{
						//log_warning("Reached Max raycast limit");
						return;
					}
				}
			}

			count++;
			((char*)entity) += stride;
		}
	}
}

struct Entity* scene_ray_intersect_closest(struct Scene* scene, struct Ray* ray, int ray_mask)
{
	struct Entity* closest = NULL;
	float current_closest = 0.f;
	for(int i = 0; i < ET_MAX; i++)
	{
		int max_length = 0;
		size_t stride = 0;
		struct Entity* entity = NULL;

		switch(i)
		{
		case ET_DEFAULT:
			if(!(ray_mask & ERM_DEFAULT)) continue;
			max_length = MAX_ENTITIES;
			entity = &scene->entities[0];
			stride = sizeof(struct Entity);
			break;
		case ET_LIGHT:
			if(!(ray_mask & ERM_LIGHT)) continue;
			max_length = MAX_LIGHTS;
			entity = &scene->lights[0].base;
			stride = sizeof(struct Light);
			break;
		case ET_STATIC_MESH:
			if(!(ray_mask & ERM_STATIC_MESH)) continue;
			max_length = MAX_STATIC_MESHES;
			entity = &scene->static_meshes[0].base;
			stride = sizeof(struct Static_Mesh);
			break;
		case ET_CAMERA:
			if(!(ray_mask & ERM_CAMERA)) continue;
			max_length = MAX_CAMERAS;
			entity = &scene->cameras[0].base;
			stride = sizeof(struct Camera);
			break;
		case ET_SOUND_SOURCE:
			if(!(ray_mask & ERM_SOUND_SOURCE)) continue;
			max_length = MAX_SOUND_SOURCES;
			entity = &scene->sound_sources[0].base;
			stride = sizeof(struct Sound_Source);
			break;
		case ET_PLAYER:
			if(!(ray_mask & ERM_PLAYER)) continue;
			max_length = 1;
			entity = &scene->player;
			stride = sizeof(struct Player);
			break;
		default: continue;
		}

		size_t count = 0;
		while(count < max_length)
		{
			if(entity->flags & EF_ACTIVE && !(entity->flags & EF_IGNORE_RAYCAST))
			{
				float distance = bv_distance_ray_bounding_box(ray, &entity->derived_bounding_box);
				if(distance != INFINITY && distance >= 0.f)
				{
					bool assign = false;
					if(closest == NULL)
						assign = true;
					else if(distance > current_closest&&
							bv_intersect_bounding_box_ray(&entity->derived_bounding_box, ray) == IT_INSIDE &&
							bv_intersect_bounding_boxes(&entity->derived_bounding_box, &closest->derived_bounding_box) != IT_INSIDE)
						assign = true;
					else if(distance < current_closest)
						assign = true;

					if(assign)
					{
						current_closest = distance;
						closest = entity;
					}
				}
			}

			count++;
			((char*)entity) += stride;
		}
	}

	return closest;
}

float scene_entity_distance(struct Scene* scene, struct Entity* entity1, struct Entity* entity2)
{
	vec3 abs_pos1 = { 0.f, 0.f, 0.f };
	vec3 abs_pos2 = { 0.f, 0.f, 0.f };
	transform_get_absolute_position(entity1, &abs_pos1);
	transform_get_absolute_position(entity2, &abs_pos2);
	vec3 distance_vector = { 0.f, 0.f, 0.f };
	vec3_sub(&distance_vector, &abs_pos1, &abs_pos2);
	return vec3_len(&distance_vector);
}

int scene_entity_archetype_add(struct Scene* scene, const char* filename)
{
	// check if we have already added this archetype, if we have, return that index
	// otherwise add it and return the index
	int index = -1;
	for(int i = 0; i < MAX_ENTITY_ARCHETYPES; i++)
	{
		if(strncmp(filename, &scene->entity_archetypes[i][0], MAX_FILENAME_LEN) == 0)
		{
			index = i;
			break;
		}
	}

	if(index == -1)
	{
		for(int i = 0; i < MAX_ENTITY_ARCHETYPES; i++)
		{
			if(scene->entity_archetypes[i][0] == '\0')
			{
				strncpy(&scene->entity_archetypes[i][0], filename, MAX_FILENAME_LEN);
				index = i;
				break;
			}
		}
	}

	if(index == -1)
		log_warning("Out of archetype indices!");

	return index;
}

struct Entity* scene_entity_duplicate(struct Scene* scene, struct Entity* entity)
{
	assert(scene && entity);

	struct Entity* new_entity = NULL;
	if(entity->archetype_index != -1)
	{
		new_entity = entity_load(scene->entity_archetypes[entity->archetype_index], DIRT_INSTALL);
		if(new_entity) scene_entity_parent_set(scene, new_entity, entity->transform.parent);
		return new_entity;
	}

	switch(entity->type)
	{
	case ET_DEFAULT:
	{
		new_entity = scene_entity_create(scene, entity->name, entity->transform.parent);
	}
	break;
	case ET_LIGHT:
	{
		struct Light* light = (struct Light*)entity;
		struct Light* new_light = scene_light_create(scene, entity->name, entity->transform.parent, light->type);
		if(!new_light)
			return new_entity;
		new_light->inner_angle = light->inner_angle;
		new_light->outer_angle = light->outer_angle;
		new_light->falloff     = light->falloff;
		new_light->intensity   = light->intensity;
		new_light->cast_shadow = light->cast_shadow;
		new_light->pcf_enabled = light->pcf_enabled;
		new_light->valid       = light->valid;
		new_light->type        = light->type;
		new_light->radius      = light->radius;
		new_light->depth_bias  = light->depth_bias;
		vec3_assign(&new_light->color, &light->color);
		memcpy(new_light->shadow_map, light->shadow_map, sizeof(int) * 4); // Fix this when we implement shadow mapping
		new_entity = &new_light->base;
	}
	break;
	case ET_STATIC_MESH:
	{
		struct Static_Mesh* mesh = (struct Static_Mesh*)entity;
		struct Static_Mesh* new_mesh = scene_static_mesh_create(scene, entity->name, entity->transform.parent, geom_get(mesh->model.geometry_index)->filename, mesh->model.material->type);
		if(!new_mesh)
			return new_entity;
		memcpy(new_mesh->model.material_params, mesh->model.material_params, sizeof(struct Variant) * MMP_MAX);
		new_entity = &new_mesh->base;
		//Handle collision related information here!
	}
	break;
	case ET_SOUND_SOURCE:
	{
		struct Sound_Source* sound_source = (struct Sound_Source*)entity;
		struct Sound_Source* new_sound_source = scene_sound_source_create(scene, entity->name, entity->transform.parent, sound_source->source_buffer->filename, sound_source->type, sound_source->loop, sound_source->playing);
		if(!new_sound_source)
			return new_entity;
		new_sound_source->min_distance     = sound_source->min_distance;
		new_sound_source->max_distance     = sound_source->max_distance;
		new_sound_source->rolloff_factor   = sound_source->rolloff_factor;
		new_sound_source->volume           = sound_source->volume;
		new_sound_source->attenuation_type = sound_source->attenuation_type;

		struct Sound* sound = game_state_get()->sound;
		sound_source_instance_attenuation_set(sound, new_sound_source->source_instance, new_sound_source->attenuation_type, new_sound_source->rolloff_factor);
		sound_source_instance_min_max_distance_set(sound, new_sound_source->source_instance, new_sound_source->min_distance, new_sound_source->max_distance);
		sound_source_instance_volume_set(sound, new_sound_source->source_instance, new_sound_source->volume);
		new_entity = &new_sound_source->base;
	}
	break;
	default:
	{
		log_message("Cannot duplicate unsupported entity type: %s", entity_type_name_get(entity));
	}
	break;
	}

	transform_copy(new_entity, entity, false);

	for(int i = 0; i < array_len(entity->transform.children); i++)
	{
		struct Entity* child_entity = entity->transform.children[i];
		struct Entity* new_child_entity = scene_entity_duplicate(scene, child_entity);
		if(new_child_entity)
			scene_entity_parent_set(scene, new_child_entity, new_entity);
		else
			log_error("scene:entity_duplicate", "Failed to create child entity from %s", child_entity->name);
	}
	return new_entity;
}
