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

#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void scene_write_entity_entry(struct Scene* scene, struct Entity* entity, struct Parser* parser);
static void scene_write_entity_list(struct Scene* scene, int entity_type, struct Parser* parser);

void scene_init(struct Scene* scene)
{
	assert(scene);
	struct Game_State* game_state = game_state_get();

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
	editor_init_camera(game_state->editor, game_state->cvars);

	scene->active_camera_index = game_state_get()->game_mode == GAME_MODE_GAME ? CAM_GAME : CAM_EDITOR;
}

bool scene_load(struct Scene* scene, const char* filename, int directory_type)
{
    FILE* scene_file = io_file_open(directory_type, filename, "rb");
	if(!scene_file)
	{
		log_error("scene:load", "Failed to open scene file %s for reading");
		return false;
	}

	// Load scene config and apply renderer settings
	struct Parser* parsed_file = parser_load_objects(scene_file, filename);

	if(!parsed_file)
	{
		log_error("scene:load", "Failed to parse file '%s' for loading scene", filename);
		fclose(scene_file);
		return false;
	}

	if(array_len(parsed_file->objects) == 0)
	{
		log_error("scene:load", "No objects found in file %s", filename);
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
			num_objects_loaded++;
		}
		break;
		default:
			log_warning("Unknown object type '%s' in scene file %s", parser_object_type_to_str(object->type), filename);
			continue;
		}
	}

	parser_free(parsed_file);
	fclose(scene_file);
	return num_objects_loaded > 0 ? true : false;
}

bool scene_save(struct Scene* scene, const char* filename, int directory_type)
{
    FILE* scene_file = io_file_open(directory_type, filename, "w");
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

	scene_write_entity_list(scene, ET_DEFAULT, parser);
	scene_write_entity_list(scene, ET_LIGHT, parser);
	scene_write_entity_list(scene, ET_STATIC_MESH, parser);
	scene_write_entity_list(scene, ET_CAMERA, parser);
	scene_write_entity_list(scene, ET_SOUND_SOURCE, parser);

    if(parser_write_objects(parser, scene_file, filename))
        log_message("Scene saved to %s", filename);

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
		((char*)entity) += stride;
		
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
			vec3 abs_pos = { 0.f, 0.f,  0.f };
			transform_get_absolute_position(&sound_source->base, &abs_pos);
			sound_source_instance_update_position(sound, sound_source->source_instance, abs_pos);
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

	for(int i = 0; i < MAX_ENTITIES; i++)
	{
		struct Entity* entity = &scene->entities[i];
		if(!(entity->flags & EF_ACTIVE)) continue;
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
		// TODO: handle creating collision mesh for the model at creation
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
			log_error("entity:sync_sound_params", "Failed to load file '%s' to provide sound source for entity %s", filename, entity->name);
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
	assert(scene && entity && parent);
	transform_parent_set(entity, parent, true);
}

void scene_ray_intersect(struct Scene* scene, struct Ray* ray, struct Raycast_Result* out_results)
{
	assert(out_results);

	memset(&out_results[0], '\0', sizeof(struct Entity*) * MAX_RAYCAST_ENTITIES_INTERSECT);
	out_results->num_entities_intersected = 0;

	for(int i = 0; i < MAX_STATIC_MESHES; i++)
	{
		struct Static_Mesh* mesh = &scene->static_meshes[i];
		if(!(mesh->base.flags & EF_ACTIVE)) continue;
		vec3 abs_pos = { 0.f };
		transform_get_absolute_position(mesh, &abs_pos);

		struct Geometry* geometry = geom_get(mesh->model.geometry_index);
		if(bv_intersect_sphere_ray(&geometry->bounding_sphere, &abs_pos, ray))
		{
			out_results->entities_intersected[out_results->num_entities_intersected] = &mesh->base;
			out_results->num_entities_intersected++;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool scene_load_(const char* filename, int directory_type)
{
	//   FILE* entity_file = platform->file.open(directory_type, filename, "r");
	//if(!entity_file)
	//{
	//       log_error("scene:load", "Failed to open scenefile %s for reading", filename);
	//       return false;
	//}

	//int count = 0;
	//int eof_char = -1;
	//while(!feof(entity_file))
	//{
	//	if(eof_char != -1) ungetc(eof_char, entity_file);
	//	struct Entity* new_entity = NULL;
	//	new_entity = entity_read(entity_file);
	//	if(!new_entity)
	//	{
	//		log_error("scene:load", "Error reading entity");
	//	}
	//	else
	//	{
	//		log_message("Loaded %s", new_entity->name);
	//		count++;
	//	}
	//	eof_char = fgetc(entity_file);
	//	/* To check end of file, we  get the next character and before beginning
	//	   loop we check if eof occured, feof only returns true if the last read
	//	   was an eof. If it wasn't eof then we return the character back to the
	//	   stream and carry on. */
	//}

	//log_message("%d entites loaded from %s", count, filename);
	//fclose(entity_file);
	return entity_load(filename, directory_type);
}

bool scene_save_(const char* filename, int directory_type)
{
	bool success = false;
	/*FILE* scene_file = platform->file.open(directory_type, filename, "w");
	  if(!scene_file)
	  {
	  log_error("scene:save", "Failed to create scenefile %s for writing", filename);
	  return false;
	  }

	  struct Parser* parser = parser_new();
	  if(!parser)
	  {
	  log_error("scene:save", "Could not create Parser");
	  fclose(scene_file);
	  return false;
	  }

	  int* entities_to_write = array_new(int);
	  array_push(entities_to_write, root_node, int);

	  bool done = false;
	  int count = 0;
	  while(!done)
	  {
	  struct Entity* entity = entity_get(entities_to_write[0]);
	  struct Parser_Object* object = parser_object_new(parser, PO_ENTITY);
	  if(!object)
	  {
	  log_error("scene:save", "Failed to create parser object for %s", entity->name);
	  continue;
	  }

	  if(!entity_write(entity, object))
	  {
	  log_error("scene:save", "Failed to write '%s' into parser object", entity->name);
	  continue;
	  }

	  log_message("Entity '%s' written to file", entity->name);
	  count++;
	  for(int i = 0; i < array_len(entity->transform.children); i++)
	  array_push(entities_to_write, entity->transform.children[i], int);

	  array_remove_at(entities_to_write, 0);
	  if(array_len(entities_to_write) == 0) done = true;
	  }

	  if(parser_write_objects(parser, scene_file, filename))
	  {
	  log_message("%d entities written to %s", count, filename);
	  }
	  else
	  {
	  log_error("scene:save", "Failed to write scene to %s", filename);
	  success = false;
	  }

	  array_free(entities_to_write);
	  parser_free(parser);
	  fclose(scene_file);*/

	return success;
}
