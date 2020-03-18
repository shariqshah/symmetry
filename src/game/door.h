#ifndef DOOR_H
#define DOOR_H

struct Door;
struct Static_Mesh;
struct Sound_Source;
struct Parser_Object;
struct Entity;

enum Door_State
{
	DOOR_CLOSED = 0,
	DOOR_OPEN,
	DOOR_CLOSING,
	DOOR_OPENING
};

void         door_init(struct Door* door, int mask);
void         door_reset(struct Door* door);
void         door_update(struct Door* door, struct Scene* scene, float dt);
struct Door* door_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity);
void         door_write(struct Door* door, struct Hashmap* entity_data);

#endif