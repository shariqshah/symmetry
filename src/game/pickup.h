#ifndef PICKUP_H
#define PICKUP_H

struct Pickup;
struct Entity;
struct Parser_Object;
struct Hashmap;

void           pickup_init(struct Pickup* pickup, int type);
void           pickup_reset(struct Pickup* pickup);
struct Pickup* pickup_read(struct Parser_Object* parser_object, const char* name, struct Entity* parent_entity);
void           pickup_write(struct Pickup* pickup, struct Hashmap* pickup_data);
void           pickup_update(struct Pickup* pickup, float dt);

#endif