#ifndef SCENE_H
#define SCENE_H

#include "entity.h"
#include "renderer.h"

#define MAX_ENTITIES      1024
#define MAX_LIGHTS        30
#define MAX_CAMERAS       2
#define MAX_STATIC_MESHES 1024
#define MAX_SOUND_SOURCES 128

struct Ray;
struct Raycast_Result;

struct Scene
{
    struct Render_Settings renderer_profile;
    struct Entity          root_entity;
    struct Player          player;
    struct Entity          entities[MAX_ENTITIES];
    struct Static_Mesh     static_meshes[MAX_STATIC_MESHES];
    struct Camera          cameras[MAX_CAMERAS];
    struct Light           lights[MAX_LIGHTS];
    struct Sound_Source    sound_sources[MAX_SOUND_SOURCES];
    int                    active_camera_index;
};

void scene_init(struct Scene* scene);
bool scene_load(struct Scene* scene, const char* filename, int dir_type);
bool scene_save(struct Scene* scene, const char* filename, int dir_type);
void scene_destroy(struct Scene* scene);
void scene_update(struct Scene* scene, float dt);
void scene_post_update(struct Scene* scene);

struct Entity*       scene_entity_create(struct Scene* scene, const char* name, struct Entity* parent);
struct Light*        scene_light_create(struct Scene* scene, const char* name, struct Entity* parent, int light_type);
struct Camera*       scene_camera_create(struct Scene* scene, const char* name, struct Entity* parent, int width, int height);
struct Static_Mesh*  scene_static_mesh_create(struct Scene* scene, const char* name, struct Entity* parent, const char* geometry_name, int material_type);
struct Sound_Source* scene_sound_source_create(struct Scene* scene, const char* name, struct Entity* parent, const char* filename, int type, bool loop, bool play);

void scene_entity_base_remove(struct Scene* scene, struct Entity* entity);
void scene_light_remove(struct Scene* scene, struct Light* light);
void scene_camera_remove(struct Scene* scene, struct Camera* camera);
void scene_static_mesh_remove(struct Scene* scene, struct Static_Mesh* mesh);
void scene_sound_source_remove(struct Scene* scene, struct Sound_Source* source);

void*                scene_find(struct Scene* scene, const char* name); // Looks in all entity type arrays and returns the first one found. Result should be cast back to expected type
struct Entity*       scene_entity_find(struct Scene* scene, const char* name);
struct Light*        scene_light_find(struct Scene* scene, const char* name);
struct Camera*       scene_camera_find(struct Scene* scene, const char* name);
struct Static_Mesh*  scene_static_mesh_find(struct Scene* scene, const char* name);
struct Sound_Source* scene_sound_source_find(struct Scene* scene, const char* name);
struct Entity*       scene_base_entity_get(struct Scene* scene, int id, int type);

void scene_entity_parent_set(struct Scene* scene, struct Entity* entity, struct Entity* parent);
void scene_entity_parent_reset(struct Scene* scene, struct Entity* entity); // Sets root entity as parent

void scene_ray_intersect(struct Scene* scene, struct Ray* ray, struct Raycast_Result* out_results);

//
//void           scene_init(void);
//void           scene_remove(struct Entity* entity);
//void           scene_reset_parent(struct Entity* entity, struct Entity* new_parent);
//void           scene_cleanup(void);
//struct Entity* scene_add_new(const char* name, const int type); /* Add as child of Root */
//struct Entity* scene_add_as_child(const char* name, const int type, int parent);
//struct Entity* scene_find(const char* name);
//struct Entity* scene_get_root(void);
//void           scene_root_set(struct Entity* entity);
//struct Entity* scene_get_child_by_name(struct Entity* parent, const char* name);
//struct Entity* scene_get_parent(struct Entity* entity);
//bool           scene_load(const char* filename, int directory_type);
//bool           scene_save(const char* filename, int directory_type);


#endif
