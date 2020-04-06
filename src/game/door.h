#ifndef DOOR_H
#define DOOR_H

#include "../common/linmath.h"

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
	DOOR_OPENING,
	DOOR_STATE_MAX
};

extern vec4 KEY_INDICATOR_COLOR_RED;
extern vec4 KEY_INDICATOR_COLOR_GREEN;
extern vec4 KEY_INDICATOR_COLOR_BLUE;
extern vec4 KEY_INDICATOR_COLOR_DISABLED;

void         door_init(struct Door* door, int mask);
void         door_reset(struct Door* door);
void         door_update(struct Door* door, struct Scene* scene, float dt);
struct Door* door_read(struct Parser_Object* object, const char* name, struct Entity* parent_entity);
void         door_write(struct Door* door, struct Hashmap* entity_data);
void         door_update_key_indicator_materials(struct Door* door);

#endif