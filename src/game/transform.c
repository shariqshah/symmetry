#include "transform.h"
#include "../common/log.h"
#include "../common/array.h"
#include "entity.h"
#include "../common/utils.h"

#include <assert.h>
#include <string.h>

void transform_init(struct Entity* entity, struct Entity* parent)
{
	struct Transform* transform = &entity->transform;
	vec3_fill(&transform->position, 0.f, 0.f, 0.f);
	vec3_fill(&transform->scale, 1.f, 1.f, 1.f);
	quat_identity(&transform->rotation);
	mat4_identity(&transform->trans_mat);
	transform->children = array_new(struct Entity*);
	transform->parent   = NULL;
	if(parent)
		transform_parent_set(entity, parent, false);
	transform_update_transmat(entity);
}

void transform_child_add(struct Entity* parent, struct Entity* child, bool update_transmat)
{
	struct Transform* parent_transform = &parent->transform;
	struct Transform* child_transform  = &child->transform;

    /* Check if already added */
	for(int i = 0; i < array_len(parent_transform->children); i++)
	{
		if(parent_transform->children[i] == child)
		{
			log_warning("Parent : %s already has a child named %s", parent->name, parent_transform->children[i]->name);
			return;
		}
	}
	
	struct Entity** new_child_loc = array_grow(parent_transform->children, struct Entity*);
	*new_child_loc = child;
	child_transform->parent = parent;
	if(update_transmat) transform_update_transmat(child);
}

bool transform_child_remove(struct Entity* parent, struct Entity* child)
{
	bool success = false;
	struct Transform* parent_transform = &parent->transform;
	for(int i = 0; i < array_len(parent_transform->children); i++)
	{
		if(parent_transform->children[i] == child)
		{
			array_remove_at(parent_transform->children, i);
			child->transform.parent = NULL;
			success = true;
			return success;
		};
	}
	return success;
}

void transform_parent_set(struct Entity* child, struct Entity* parent, bool update_transmat)
{
	if(child->transform.parent == NULL)
	{
		transform_child_add(parent, child, false);
	}
	else
	{
		transform_child_remove(parent, child);
		transform_child_add(parent, child, false);
	}

	if(update_transmat) transform_update_transmat(child);
}

void transform_copy(struct Entity* copy_to, struct Entity* copy_from, bool copy_parent)
{
	/* Bear in mind that we can't copy the children because that would mean
	   those children have more than one parent and we don't support that.*/
	struct Entity* current_parent = copy_to->transform.parent;
	struct Entity** current_children = copy_to->transform.children;

	memcpy(&copy_to->transform, &copy_from->transform, sizeof(struct Transform));
	if(copy_parent)
	{
		transform_child_remove(current_parent, copy_to);
		transform_parent_set(copy_to, copy_from->transform.parent, true);
	}
	else
	{
		copy_to->transform.parent = current_parent;
	}

	copy_to->transform.is_modified = true;
	copy_to->transform.children = current_children;
}

void transform_translate(struct Entity* entity, vec3* amount, enum Transform_Space space)
{
	struct Transform* transform = &entity->transform;
	vec3 translation_amount;
	vec3_assign(&translation_amount, amount);
	if(space == TS_LOCAL)
	{
		quat_mul_vec3(&translation_amount, &transform->rotation, &translation_amount);
	}
	else if(space == TS_PARENT)
	{
		struct Entity* parent = transform->parent;
		if(parent)
		{
			struct Transform* parent_tran = &parent->transform;
			quat_mul_vec3(&translation_amount, &parent_tran->rotation, &translation_amount);
		}
	}
	vec3_add(&transform->position, &transform->position, &translation_amount);
	transform_update_transmat(entity);
}

void transform_rotate(struct Entity*       entity,
					  vec3*                axis,
					  float                angle,
					  enum Transform_Space space)
{
	struct Transform* transform = &entity->transform;
	quat new_rot;
	quat_identity(&new_rot);
	quat_axis_angle(&new_rot, axis, angle);

	if(space == TS_LOCAL)
		quat_mul(&transform->rotation, &transform->rotation, &new_rot);
	else
		quat_mul(&transform->rotation, &new_rot, &transform->rotation);
	transform_update_transmat(entity);
}

void transform_scale(struct Entity* entity, vec3* scale)
{
	struct Transform* transform = &entity->transform;
	vec3_assign(&transform->scale, scale);
	transform_update_transmat(entity);
}

void transform_get_forward(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	vec3_fill(res, 0.f, 0.f, -1.f);
	quat_get_forward_rh(res, &transform->rotation);
}

void transform_get_lookat(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	transform_get_forward(entity, res);
	vec3_add(res, &transform->position, res);
}

void transform_get_absolute_forward(struct Entity* entity, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(entity, &abs_rot);
	quat_get_forward_rh(res, &abs_rot);
}

void transform_get_absolute_lookat(struct Entity* entity, vec3* res)
{
	vec3 abs_position = {0.f, 0.f, 0.f};
	transform_get_absolute_position(entity, &abs_position);
	transform_get_absolute_forward(entity, res);
	vec3_add(res, &abs_position, res);
}

void transform_get_up(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	vec3_fill(res, 0.f, 1.f, 0.f);
	quat_get_up(res, &transform->rotation);
}

void transform_get_absolute_up(struct Entity* entity, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(entity, &abs_rot);
	quat_get_up(res, &abs_rot);
}

void transform_get_absolute_right(struct Entity* entity, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(entity, &abs_rot);
	quat_get_right(res, &abs_rot);
}


void transform_get_right(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	vec3_fill(res, 1.f, 0.f, 0.f);
	quat_get_right(res, &transform->rotation);
}

void transform_update_transmat(struct Entity* entity)
{
	static mat4 scale, translation, rotation;
	struct Transform* transform = &entity->transform;
	mat4_identity(&scale);
	mat4_identity(&translation);
	mat4_identity(&rotation);
	mat4_identity(&transform->trans_mat);
	mat4_scale(&scale, transform->scale.x, transform->scale.y, transform->scale.z);
	mat4_translate(&translation, transform->position.x, transform->position.y, transform->position.z);
	mat4_from_quat(&rotation, &transform->rotation);

	mat4_mul(&transform->trans_mat, &transform->trans_mat, &translation);
	mat4_mul(&transform->trans_mat, &transform->trans_mat, &rotation);
	mat4_mul(&transform->trans_mat, &transform->trans_mat, &scale);
	
	if(transform->parent)
	{
		struct Transform* parent_tran = &transform->parent->transform;
		mat4_mul(&transform->trans_mat, &parent_tran->trans_mat, &transform->trans_mat);
	}

	/* Update all children */
	int children = array_len(transform->children);
	if(children > 0)
	{
		for(int i = 0; i < children; i++)
			transform_update_transmat(entity->transform.children[i]);
	}
	transform->is_modified = true;
	if(entity->type == ET_STATIC_MESH) entity->transform.sync_physics = true; // Find a better way to handle this
}

void transform_destroy(struct Entity* entity)
{
	struct Transform* transform = &entity->transform;
	// Remove children
	int children = array_len(transform->children);
	if(children > 0)
	{
		for(int i = 0; i < children; i++)
		{
			struct Entity* child = transform->children[i];
			child->flags |= EF_MARKED_FOR_DELETION;
		}
	}

	// Remove this entity from parent's children
	if(entity->transform.parent)
		transform_child_remove(entity->transform.parent, entity);
		
	/* Remove transform */
	array_free(transform->children);
	vec3_fill(&transform->position, 0.f, 0.f, 0.f);
	vec3_fill(&transform->scale, 1.f, 1.f, 1.f);
	quat_identity(&transform->rotation);
	mat4_identity(&transform->trans_mat);
	transform->parent = NULL;
	transform->is_modified = false;
}

void transform_set_position(struct Entity* entity, vec3* new_position)
{
	struct Transform* transform = &entity->transform;
	vec3_assign(&transform->position, new_position);
	transform_update_transmat(entity);
}

void transform_get_absolute_position(struct Entity* entity, vec3* res)
{
	vec3_assign(res, &entity->transform.position);
	bool done = false;
	struct Entity* parent = entity->transform.parent;
	while(!done)
	{
		if(!parent)
		{
			done = true;
			break;
		}
		vec3_add(res, res, &parent->transform.position);
		parent = parent->transform.parent;
	}
}

void transform_get_absolute_scale(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	vec3_assign(res, &transform->scale);
	bool done = false;
	struct Entity* parent = transform->parent;
	while(!done)
	{
		if(!parent)
		{
			done = true;
			break;
		}
		vec3_mul(res, res, &parent->transform.scale);
		parent = parent->transform.parent;
	}
	
}

void transform_get_absolute_rot(struct Entity* entity, quat* res)
{
	quat_assign(res, &entity->transform.rotation);
	bool done = false;
	struct Entity* parent = entity->transform.parent;
	while(!done)
	{
		if(!parent)
		{
			done = true;
			break;
		}
		quat_mul(res, res, &parent->transform.rotation);
		parent = parent->transform.parent;
	}
}

void transform_reset(struct Entity* entity)
{
	/* Does remove/change parent or children, only modifies that transformation data */
	vec3_fill(&entity->transform.position, 0.f, 0.f, 0.f);
	vec3_fill(&entity->transform.scale, 0.f, 0.f, 0.f);
	quat_identity(&entity->transform.rotation);
	transform_update_transmat(entity);
}
