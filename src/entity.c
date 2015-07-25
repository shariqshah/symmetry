#include "entity.h"
#include "array.h"
#include "log.h"
#include "string_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct Entity* entity_list;
static int* empty_indices;


void entity_initialize(void)
{
	entity_list = array_new(struct Entity);
	empty_indices = array_new(int);
}

void entity_cleanup(void)
{
	for(int i = 0; i < array_len(entity_list); i++)
		entity_remove(i);

	array_free(entity_list);
	array_free(empty_indices);
}

void entity_remove(int index)
{
	struct Entity* entity = &entity_list[index];

	for(int i = 0; i < MAX_COMPONENTS; i++)
	{
		enum Component component = entity->components[i];
		switch(component)
		{
		case C_TRANSFORM:
			break;
		case C_MODEL:
			break;
		case C_RIGIDBODY:
			break;
		case C_CAMERA:
			break;
		default:
			/* Only called for MAX_COMPONENTS, do nothing */
			break;
		}
	}
}

struct Entity* entity_create(const char* name, const char* tag)
{
	struct Entity* new_entity = NULL;
	int index = -1;
	int empty_len = array_len(empty_indices);
	if(empty_len > 0)
	{
		index = empty_indices[empty_len - 1];
		array_pop(empty_indices);
		new_entity = &entity_list[index];
	}
	else
	{
		new_entity = array_grow(entity_list, struct Entity);
		index = array_len(entity_list) - 1;
	}
	
	if(new_entity->name) free(new_entity->name);
	if(new_entity->name) free(new_entity->tag);

	new_entity->name = name ? str_new(name) : str_new("DEFAULT_NAME");
	new_entity->tag = tag ? str_new(tag) : str_new("DEFAULT_TAG");
	new_entity->node = index;

	/* TODO: Add transform here by default maybe? */
	
	return new_entity;
	   
}

struct Entity* entity_get(int index)
{
	struct Entity* entity = NULL;
	if(index >= 0 && index < array_len(entity_list))
		entity = &entity_list[index];
	else
		log_error("entity:get", "Invalid index '%d'", index);
	return entity;
}

struct Entity* entity_find(const char* name)
{
	/* Bruteforce search all entities and return the first match */
	struct Entity* entity = NULL;
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* curr_ent = &entity_list[i];
		if(strcmp(curr_ent->name, name) == 0)
		{
			entity = curr_ent;
			break;
		}
	}
	
	return entity; 
}
bool entity_component_remove(struct Entity* entity, enum Component component)
{
	bool success = true;
	assert(entity);
    switch(component)
	{
	case C_TRANSFORM:
		break;
	case C_MODEL:
		break;
	case C_RIGIDBODY:
		break;
	case C_CAMERA:
		break;
	default:
		log_error("entity:component_remove", "Invalid component type");
		break;
	}
	if(success) entity->components[component] = -1;
	
	return success;
}
void* entity_component_get(struct Entity* entity, enum Component component)
{
	void* comp_obj = NULL;
	assert(entity);
	switch(component)
	{
	case C_TRANSFORM:
		break;
	case C_MODEL:
		break;
	case C_RIGIDBODY:
		break;
	case C_CAMERA:
		break;
	default:
		log_error("entity:component_get", "Invalid component type");
		break;
	}
	return comp_obj;
}

void* entity_component_add(struct Entity* entity, enum Component component)
{
	void* new_comp = NULL;
	assert(entity);
	switch(component)
	{
	case C_TRANSFORM:
		break;
	case C_MODEL:
		break;
	case C_RIGIDBODY:
		break;
	case C_CAMERA:
		break;
	default:
		log_error("entity:component_add", "Invalid component type");
		break;
	}
	return new_comp;
}
