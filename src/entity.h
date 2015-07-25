#ifndef entity_H
#define entity_H

#include "components.h"
#include "num_types.h"

typedef int32 Node;

struct Entity
{
	Node  node;
	char* name;
	char* tag;
	int   components[MAX_COMPONENTS];
};

void           entity_init(void);
void           entity_cleanup(void);
void           entity_remove(int index);
struct Entity* entity_create(const char* name, const char* tag);
struct Entity* entity_get(int index);
struct Entity* entity_find(const char* name);
int            entity_component_remove(struct Entity* entity, enum Component component);
void*          entity_component_get(struct Entity* entity, enum Component component);
void*          entity_component_add(struct Entity* entity, enum Component component);


#endif
