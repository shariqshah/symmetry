#ifndef ENEMY_H
#define ENEMY_H

struct Enemy;
struct Scene;
struct Parser_Object;
struct Entity;
struct Hashmap;

void          enemy_init(struct Enemy* enemy, int type);
void          enemy_update(struct Enemy* enemy, struct Scene* scene, float dt);
void          enemy_reset(struct Enemy* enemy);
struct Enemy* enemy_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity);
void          enemy_write(struct Enemy* enemy, struct Hashmap* entity_data);

#endif