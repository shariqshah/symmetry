#include "transform.h"
#include "log.h"
#include "array.h"
#include "entity.h"
#include "utils.h"
#include <assert.h>

void transform_create(struct Entity* entity, int parent_entity)
{
	struct Transform* transform = &entity->transform;
	vec3_fill(&transform->position, 0.f, 0.f, 0.f);
	vec3_fill(&transform->scale, 1.f, 1.f, 1.f);
	quat_identity(&transform->rotation);
	mat4_identity(&transform->trans_mat);
	transform->parent   = parent_entity;
	transform->children = array_new(int);
	transform_update_transmat(entity);
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
		struct Entity* parent = entity_get_parent(transform->parent);
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
	quat_axis_angle(&new_rot, axis, TO_RADIANS(angle));

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
	transform_get_absolute_pos(entity, &abs_position);
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
	
	struct Entity* parent = entity_get(transform->parent);
	if(parent)
	{
		struct Transform* parent_tran = &parent->transform;
		mat4_mul(&transform->trans_mat, &transform->trans_mat, &parent_tran->trans_mat);
	}

	/* Update all children */
	int children = array_len(transform->children);
	if(children > 0)
	{
		for(int i = 0; i < children; i++)
		{
			struct Entity* child = entity_get(transform->children[i]);
			transform_update_transmat(child);
		}
	}
	transform->is_modified = true;
}

void transform_destroy(struct Entity* entity)
{
	struct Transform* transform = &entity->transform;
	int children = array_len(transform->children);
	if(children > 0)
	{
		for(int i = 0; i < children; i++)
		{
			struct Entity* child = entity_get(transform->children[i]);
			child->marked_for_deletion = true;
		}
	}
	
	/* Remove transform */
	array_free(transform->children);
	vec3_fill(&transform->position, 0.f, 0.f, 0.f);
	vec3_fill(&transform->scale, 1.f, 1.f, 1.f);
	quat_identity(&transform->rotation);
	mat4_identity(&transform->trans_mat);
	transform->parent = -1;
	transform->is_modified = false;
}

void transform_set_position(struct Entity* entity, vec3* new_position)
{
	struct Transform* transform = &entity->transform;
	vec3_assign(&transform->position, new_position);
	transform_update_transmat(entity);
}

void transform_get_absolute_pos(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	struct Entity*    parent    = entity_get(transform->parent);
	if(parent)
		transform_get_absolute_pos(parent, res);
	vec3_add(res, res, &transform->position);
}

void transform_get_absolute_scale(struct Entity* entity, vec3* res)
{
	struct Transform* transform = &entity->transform;
	struct Entity*    parent    = entity_get(transform->parent);
	if(parent)
		transform_get_absolute_scale(parent, res);
	vec3_add(res, res, &transform->scale);
}

void transform_get_absolute_rot(struct Entity* entity, quat* res)
{
	struct Transform* transform = &entity->transform;
	struct Entity*    parent    = entity_get(transform->parent);
	if(parent)
		transform_get_absolute_rot(parent, res);
	quat_mul(res, res, &transform->rotation);
}
