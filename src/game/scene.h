#ifndef SCENE_H
#define SCENE_H

#include "entity.h"
#include "renderer.h"

#define MAX_ENTITIES             32
#define MAX_LIGHTS               30
#define MAX_CAMERAS              2
#define MAX_STATIC_MESHES        1024
#define MAX_SOUND_SOURCES        128
#define MAX_ENTITY_ARCHETYPES    32
#define MAX_ENEMIES              64

struct Ray;
struct Raycast_Result;

struct Scene
{
	char                   filename[MAX_FILENAME_LEN];
    struct Entity          root_entity;
    struct Player          player;
    struct Entity          entities[MAX_ENTITIES];
    struct Static_Mesh     static_meshes[MAX_STATIC_MESHES];
    struct Camera          cameras[MAX_CAMERAS];
    struct Light           lights[MAX_LIGHTS];
    struct Sound_Source    sound_sources[MAX_SOUND_SOURCES];
	struct Enemy           enemies[MAX_ENEMIES];
	char                   entity_archetypes[MAX_ENTITY_ARCHETYPES][MAX_FILENAME_LEN];
    int                    active_camera_index;
};

void scene_init(struct Scene* scene);
bool scene_load(struct Scene* scene, const char* filename, int directory_type);
bool scene_save(struct Scene* scene, const char* filename, int directory_type);
void scene_destroy(struct Scene* scene);
void scene_update(struct Scene* scene, float dt);
void scene_post_update(struct Scene* scene);

struct Entity*       scene_entity_duplicate(struct Scene* scene, struct Entity* entity);
struct Entity*       scene_entity_create(struct Scene* scene, const char* name, struct Entity* parent);
struct Light*        scene_light_create(struct Scene* scene, const char* name, struct Entity* parent, int light_type);
struct Camera*       scene_camera_create(struct Scene* scene, const char* name, struct Entity* parent, int width, int height);
struct Static_Mesh*  scene_static_mesh_create(struct Scene* scene, const char* name, struct Entity* parent, const char* geometry_name, int material_type);
struct Sound_Source* scene_sound_source_create(struct Scene* scene, const char* name, struct Entity* parent, const char* filename, int type, bool loop, bool play);
struct Enemy*        scene_enemy_create(struct Scene* scene, const char* name, struct Entity* parent, int type);

void scene_entity_base_remove(struct Scene* scene, struct Entity* entity);
void scene_light_remove(struct Scene* scene, struct Light* light);
void scene_camera_remove(struct Scene* scene, struct Camera* camera);
void scene_static_mesh_remove(struct Scene* scene, struct Static_Mesh* mesh);
void scene_sound_source_remove(struct Scene* scene, struct Sound_Source* source);
void scene_enemy_remove(struct Scene* scene, struct Enemy* enemy);

void*                scene_find(struct Scene* scene, const char* name); // Looks in all entity type arrays and returns the first one found. Result should be cast back to expected type
struct Entity*       scene_entity_find(struct Scene* scene, const char* name);
struct Light*        scene_light_find(struct Scene* scene, const char* name);
struct Camera*       scene_camera_find(struct Scene* scene, const char* name);
struct Static_Mesh*  scene_static_mesh_find(struct Scene* scene, const char* name);
struct Sound_Source* scene_sound_source_find(struct Scene* scene, const char* name);
struct Entity*       scene_base_entity_get(struct Scene* scene, int id, int type);
struct Enemy*        scene_enemy_get(struct Scene* scene, const char* name);

void scene_entity_parent_set(struct Scene* scene, struct Entity* entity, struct Entity* parent);
void scene_entity_parent_reset(struct Scene* scene, struct Entity* entity); // Sets root entity as parent
int  scene_entity_archetype_add(struct Scene* scene, const char* filename);

void           scene_ray_intersect(struct Scene* scene, struct Ray* ray, struct Raycast_Result* out_results, int ray_mask);
struct Entity* scene_ray_intersect_closest(struct Scene* scene, struct Ray* ray, int ray_mask);
float          scene_entity_distance(struct Scene* scene, struct Entity* entity1, struct Entity* entity2);

#endif
