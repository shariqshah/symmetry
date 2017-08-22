#include "scene.h"
#include "array.h"
#include "entity.h"
#include "log.h"
#include "transform.h"
#include "common.h"

#include <assert.h>
#include <string.h>

static int root_node = -1;

void scene_init(void)
{
	/* Add root node to scene */
	struct Entity* root = entity_create("ROOT", ET_ROOT, -1);
	root_node = root->id;
}

struct Entity* scene_add_new(const char* name, const int type)
{
	return scene_add_as_child(name, type, root_node);
}

struct Entity* scene_add_as_child(const char* name, const int type, int parent_id)
{
	assert(parent_id > -1);
	return entity_create(name, type, parent_id);
}

void scene_remove(struct Entity* entity)
{
	assert(entity);
	for(int i = 0; i < array_len(entity->transform.children); i++)
	{
		struct Entity* child = entity_get(entity->transform.children[i]);
		scene_remove(child);
	}
	entity_remove(entity->id);
}

void scene_reset_parent(struct Entity* entity, struct Entity* new_parent)
{
	assert(entity && new_parent);
	struct Entity* curr_parent = entity_get(entity->transform.parent);
	if(curr_parent)
	{
		/* find the index that the entity is at in the cuurent parent's
		   children array and remove it from there. Then set the new_parent
		   as the entity's parent */
		int index = -1;
		for(int i = 0; i < array_len(curr_parent->transform.children); i++)
		{
			if(curr_parent->transform.children[i] == entity->id)
			{
				index = i;
				break;
			}
		}

		if(index > -1)
		{
			array_remove_at(curr_parent->transform.children, index);
			entity->transform.parent = new_parent->id;
			array_push(new_parent, entity->id, int);
			transform_update_transmat(entity);
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
		if(entity_list[i].id != -1)
			entity_remove(i);
	}
	entity_post_update();
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
	for(int i = 0; i < array_len(parent->transform.children); i++)
	{
		struct Entity* curr_child = entity_get(parent->transform.children[i]);
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
	if(entity->transform.parent != -1)
		parent = entity_get(entity->transform.parent);
	return parent;
}

bool scene_load(const char* filename, int directory_type)
{
	bool success = false;
    FILE* entity_file = platform->file.open(directory_type, filename, "r");
	if(!entity_file)
	{
		log_error("scene:load", "Failed to open scenefile %s for writing", filename);
		return NULL;
	}

    struct Entity* root = entity_get(root_node);

	return success;
}

bool scene_save(const char* filename, int directory_type)
{
	bool success = false;
	return success;
}
