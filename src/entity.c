#include "entity.h"
#include "array.h"
#include "log.h"
#include "string_utils.h"
#include "transform.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static struct Entity* entity_list;
static int* empty_indices;


void entity_init(void)
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
		switch((enum Component)i)
		{
		case C_TRANSFORM: transform_remove(entity->components[i]); break;
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
		entity->components[i] = -1;
	}
	entity->node = -1;
	free(entity->name);
	free(entity->tag);
	entity->name = entity->tag = NULL;
	array_push(empty_indices, index, int);
}

struct Entity* entity_create(const char* name, const char* tag)
{
	struct Entity* new_entity = NULL;
	int index = -1;
	
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		array_pop(empty_indices);
		new_entity = &entity_list[index];
	}
	else
	{
		new_entity = array_grow(entity_list, struct Entity);
		new_entity->name = new_entity->tag = NULL;
		index = array_len(entity_list) - 1;
	}
	
	if(new_entity->name) free(new_entity->name);
	if(new_entity->tag)  free(new_entity->tag);

	new_entity->name = name ? str_new(name) : str_new("DEFAULT_NAME");
	new_entity->tag = tag ? str_new(tag) : str_new("DEFAULT_TAG");
	new_entity->node = index;
	new_entity->components[C_TRANSFORM] = transform_create(new_entity->node);
	
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
		if(!entity->name)
			continue;
		if(strcmp(curr_ent->name, name) == 0)
		{
			entity = curr_ent;
			break;
		}
	}
	return entity; 
}
int entity_component_remove(struct Entity* entity, enum Component component)
{
	int success = 1;
	assert(entity);
    switch(component)
	{
	case C_TRANSFORM:
		log_error("entity:remove", "Cannot remove Tranform component");
		success = 0;
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
	case C_TRANSFORM: comp_obj = transform_get(entity->components[C_TRANSFORM]); break;
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
		log_error("entity:add_component", "Entity already has Transform component");
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
