#include "scene.h"
#include "array.h"
#include "entity.h"
#include "log.h"
#include "transform.h"

#include <assert.h>
#include <string.h>

static int root_node = -1;

void scene_init(void)
{
	/* Add root node to scene */
	struct Entity* root = entity_create("ROOT", NULL);
	root_node = root->node;
}

struct Entity* scene_add_new(const char* name, const char* tag)
{
	return scene_add_as_child(name, tag, root_node);
}

struct Entity* scene_add_as_child(const char* name, const char* tag, int parent_node)
{
	assert(parent_node > -1);
	struct Entity* new_entity = NULL;
	new_entity = entity_create(name, tag);
	struct Entity* parent = entity_get(parent_node);
	new_entity->parent = parent->node;
	array_push(parent->children, new_entity->node, int);
	struct Transform* new_ent_tran = entity_component_get(new_entity, C_TRANSFORM);
	transform_update_transmat(new_ent_tran);
	return new_entity;
}

void scene_remove(struct Entity* entity)
{
	assert(entity);
	for(int i = 0; i < array_len(entity->children); i++)
	{
		struct Entity* child = entity_get(entity->children[i]);
		scene_remove(child);
	}
	entity_remove(entity->node);
}

void scene_reset_parent(struct Entity* entity, struct Entity* new_parent)
{
	assert(entity && new_parent);
	struct Entity* curr_parent = entity_get(entity->parent);
	if(curr_parent)
	{
		/* find the index that the entity is at in the cuurent parent's
		   children array and remove it from there. Then set the new_parent
		   as the entity's parent */
		int index = -1;
		for(int i = 0; i < array_len(curr_parent->children); i++)
		{
			if(curr_parent->children[i] == entity->node)
			{
				index = i;
				break;
			}
		}

		if(index > -1)
		{
			array_remove_at(curr_parent->children, index);
			entity->parent = new_parent->node;
			array_push(new_parent, entity->node, int);
			struct Transform* entity_tran = entity_component_get(entity, C_TRANSFORM);
			transform_update_transmat(entity_tran);
		}
		else
		{
			log_error("scene:reset_parent", "Entity %s not found in it's parent '%s''s children array");
		}
	}
	else
	{
		log_error("scene:reset_parent", "Cannot change parent of ROOT entity");
	}
}

void scene_cleanup(void)
{
	struct Entity* entity_list = entity_get_all();
	for(int i = 0; i < array_len(entity_list); i++)
	{
		if(entity_list[i].node != -1)
			entity_remove(i);
	}
}

struct Entity* scene_find(const char* name)
{
	return entity_find(name);
}

struct Entity* scene_get_root(void)
{
	return entity_get(root_node);
}

struct Entity* scene_get_child_by_name(struct Entity* parent, const char* name)
{
	assert(parent);
	struct Entity* child = NULL;
	for(int i = 0; i < array_len(parent->children); i++)
	{
		struct Entity* curr_child = entity_get(parent->children[i]);
		if(strcmp(curr_child->name, name) == 0)
		{
			child = curr_child;
			break;
		}
	}
	return child;
}

struct Entity* scene_get_parent(struct Entity* entity)
{
	assert(entity);
	struct Entity* parent = NULL;
	if(entity->parent != -1)
		parent = entity_get(entity->parent);
	return parent;
}
