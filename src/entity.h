#ifndef entity_H
#define entity_H

#include "components.h"

struct Entity
{
	int   node;
	char* name;
	char* tag;
	int   components[NUM_COMPONENTS];
	int   parent;
	int*  children;
	int   is_listener;
};

void           entity_init(void);
void           entity_cleanup(void);
void           entity_remove(int index);
struct Entity* entity_create(const char* name, const char* tag);
struct Entity* entity_get(int index);
struct Entity* entity_find(const char* name);
struct Entity* entity_get_all(void);
struct Entity* entity_get_parent(int node);
int            entity_component_remove(struct Entity* entity, enum Component component);
void*          entity_component_get(struct Entity* entity, enum Component component);
void*          entity_component_add(struct Entity* entity, enum Component component, ...);
int            entity_has_component(struct Entity* entity, enum Component component);
void           entity_sync_components(struct Entity* entity);

#endif
