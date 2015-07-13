#ifndef entity_H
#define entity_H

#include "components.h"
#include "num_types.h"
#include <stdbool.h>

typedef int32 Node;

typedef struct
{
	Node  node;
	char* name;
	char* tag;
	int   components[MAX_COMPONENTS];
} Entity;

void    entity_initialize(void);
void    entity_cleanup(void);
void    entity_remove(int index);
Entity* entity_create(const char* name, const char* tag);
Entity* entity_get(int index);
Entity* entity_find(const char* name);
bool    entity_component_remove(Entity* entity, Component component);
void*   entity_component_get(Entity* entity, Component component);
void*   entity_component_add(Entity* entity, Component component);


#endif
