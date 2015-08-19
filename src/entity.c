#include "entity.h"
#include "array.h"
#include "log.h"
#include "string_utils.h"
#include "transform.h"
#include "camera.h"
#include "model.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

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
		if(i == C_TRANSFORM)
		{
			transform_remove(entity->components[i]);
			entity->components[i] = -1;
		}
		else
		{
			entity_component_remove(entity, i);
		}
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
	for(int i = 0; i < MAX_COMPONENTS; i++)
		new_entity->components[i] = -1;
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
	int comp_index = entity->components[component];
	switch(component)
	{
	case C_TRANSFORM: log_error("entity:remove_component", "Cannot remove TRANSFORM"); break;
	case C_MODEL:     if(comp_index != -1) model_remove(comp_index); break;
	case C_CAMERA:    if(comp_index != -1) camera_remove(comp_index); break;
	case C_RIGIDBODY:
		break;
	default:
		/* Only called for MAX_COMPONENTS, do nothing */
		break;
	}
	entity->components[component] = -1;
	if(success) entity->components[component] = -1;
	
	return success;
}

void* entity_component_get(struct Entity* entity, enum Component component)
{
	void* comp_obj = NULL;
	assert(entity);
	int comp_index = entity->components[component];
	if(comp_index != -1)
	{
		switch(component)
		{
		case C_TRANSFORM: comp_obj = transform_get(comp_index); break;
		case C_MODEL:     comp_obj = model_get(comp_index); break;
		case C_CAMERA:    comp_obj = camera_get(comp_index); break;
		case C_RIGIDBODY:
			break;
		default: log_error("entity:component_get", "Invalid component type"); break;
		}
	}
	else
	{
		log_error("entity:component_get", "Entity '%s' does not have component %s",
				  entity->name,
				  comp_to_str(component));
	}
	return comp_obj;
}

void* entity_component_add(struct Entity* entity, enum Component component, ...)
{
	void* new_comp = NULL;
	assert(entity);
	va_list args;
	va_start(args, component);
	int new_comp_index = -1;
	switch(component)
	{
	case C_TRANSFORM:
		log_error("entity:add_component", "Entity already has Transform component");
	case C_MODEL:
	{
		const char* filename = va_arg(args, const char*);
		new_comp_index = model_create(entity->node, filename);
		new_comp = model_get(new_comp_index);
	}
	break;
	case C_CAMERA:
	{
		int width = va_arg(args, int);
		int height = va_arg(args, int);
		new_comp_index = camera_create(entity->node, width, height);
		new_comp = camera_get(new_comp_index);
	}
	break;
	case C_RIGIDBODY:
		break;
	default:
		log_error("entity:component_add", "Invalid component type");
		break;
	}

	if(new_comp_index == -1)
	{
		log_error("entity:component_add", "%s component not added to %s",
									   comp_to_str(component),
									   entity->name);
	}
	else
	{
		entity->components[component] = new_comp_index;
		log_message("%s component added to %s", comp_to_str(component), entity->name);
	}
	va_end(args);
	
	return new_comp;
}

int entity_has_component(struct Entity* entity, enum Component component)
{
	int has_comp = 0;
	if(entity->components[component] != -1)
		has_comp = 1;
	return has_comp;
}

void entity_sync_components(struct Entity* entity)
{
	if(entity_has_component(entity, C_CAMERA))
	{
		struct Camera* camera = entity_component_get(entity, C_CAMERA);
		camera_update_view(camera);
	}
}
