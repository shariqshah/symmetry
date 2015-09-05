#ifndef entity_H
#define entity_H

#include "components.h"

struct Entity
{
	int   node;
	char* name;
	char* tag;
	int   components[MAX_COMPONENTS];
	int   parent;
	int*  children;
};

void           entity_init(void);
void           entity_cleanup(void);
void           entity_remove(int index);
struct Entity* entity_create(const char* name, const char* tag);
struct Entity* entity_get(int index);
struct Entity* entity_find(const char* name);
struct Entity* entity_get_all(void);
int            entity_component_remove(struct Entity* entity, enum Component component);
void*          entity_component_get(struct Entity* entity, enum Component component);
void*          entity_component_add(struct Entity* entity, enum Component component, ...);
int            entity_has_component(struct Entity* entity, enum Component component);
void           entity_sync_components(struct Entity* entity);

#endif
