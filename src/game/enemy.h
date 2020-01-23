#ifndef ENEMY_H
#define ENEMY_H

struct Enemy;
struct Scene;
struct Parser_Object;
struct Entity;
struct Hashmap;

void          enemy_init(struct Enemy* enemy, int type);
void          enemy_update_physics(struct Enemy* enemy, struct Scene* scene, float dt);
void          enemy_update(struct Enemy* enemy, struct Scene* scene, float dt);
void          enemy_reset(struct Enemy* enemy);
struct Enemy* enemy_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity);
void          enemy_write(struct Enemy* enemy, struct Hashmap* entity_data);
void          enemy_weapon_sound_set(struct Enemy* enemy, const char* sound_filename, int type);
void          enemy_static_mesh_set(struct Enemy* enemy, const char* geometry_filename, int material_type);

#endif