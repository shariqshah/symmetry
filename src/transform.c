#include "transform.h"
#include "log.h"
#include "array.h"
#include "entity.h"
#include "utils.h"
#include <assert.h>

void transform_create(struct Transform* transform, int parent_entity)
{
	assert(transform);
	vec3_fill(&transform->position, 0.f, 0.f, 0.f);
	vec3_fill(&transform->scale, 1.f, 1.f, 1.f);
	quat_identity(&transform->rotation);
	mat4_identity(&transform->trans_mat);
	transform->parent   = parent_entity;
	transform->children = array_new(int);
	transform_update_transmat(transform);
}

void transform_translate(struct Transform* transform, vec3* amount, enum Transform_Space space)
{
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
	transform_update_transmat(transform);
}

void transform_rotate(struct Transform*    transform,
					  vec3*                axis,
					  float                angle,
					  enum Transform_Space space)
{
	quat new_rot;
	quat_identity(&new_rot);
	quat_axis_angle(&new_rot, axis, TO_RADIANS(angle));

	if(space == TS_LOCAL)
		quat_mul(&transform->rotation, &transform->rotation, &new_rot);
	else
		quat_mul(&transform->rotation, &new_rot, &transform->rotation);
	transform_update_transmat(transform);
}

void transform_scale(struct Transform* transform, vec3* scale)
{
	vec3_assign(&transform->scale, scale);
	transform_update_transmat(transform);
}

void transform_get_forward(struct Transform* transform, vec3* res)
{
	vec3_fill(res, 0.f, 0.f, -1.f);
	quat_get_forward_rh(res, &transform->rotation);
}

void transform_get_lookat(struct Transform* transform, vec3* res)
{
	transform_get_forward(transform, res);
	vec3_add(res, &transform->position, res);
}

void transform_get_absolute_forward(struct Transform* transform, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(transform, &abs_rot);
	quat_get_forward_rh(res, &abs_rot);
}

void transform_get_absolute_lookat(struct Transform* transform, vec3* res)
{
	vec3 abs_position = {0.f, 0.f, 0.f};
	transform_get_absolute_pos(transform, &abs_position);
	transform_get_absolute_forward(transform, res);
	vec3_add(res, &abs_position, res);
}

void transform_get_up(struct Transform* transform, vec3* res)
{
	vec3_fill(res, 0.f, 1.f, 0.f);
	quat_get_up(res, &transform->rotation);
}

void transform_get_absolute_up(struct Transform* transform, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(transform, &abs_rot);
	quat_get_up(res, &abs_rot);
}

void transform_get_absolute_right(struct Transform* transform, vec3* res)
{
	quat abs_rot;
	quat_identity(&abs_rot);
	transform_get_absolute_rot(transform, &abs_rot);
	quat_get_right(res, &abs_rot);
}


void transform_get_right(struct Transform* transform, vec3* res)
{
	vec3_fill(res, 1.f, 0.f, 0.f);
	quat_get_right(res, &transform->rotation);
}

void transform_update_transmat(struct Transform* transform)
{
	static mat4 scale, translation, rotation;
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
			struct Entity*    child      = entity_get(transform->children[i]);
			struct Transform* child_tran = &child->transform;
			transform_update_transmat(child_tran);
		}
	}
	transform->is_modified = true;
}

void transform_destroy(struct Transform* transform)
{
	assert(transform);
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

void transform_set_position(struct Transform* transform, vec3* new_position)
{
	vec3_assign(&transform->position, new_position);
	transform_update_transmat(transform);
}

void transform_get_absolute_pos(struct Transform* transform, vec3* res)
{
	struct Entity* parent = entity_get(transform->parent);
	if(parent)
	{
		struct Transform* parent_tran = &parent->transform;
		transform_get_absolute_pos(parent_tran, res);
	}
	vec3_add(res, res, &transform->position);
}

void transform_get_absolute_scale(struct Transform* transform, vec3* res)
{
	struct Entity* parent = entity_get(transform->parent);
	if(parent)
	{
		struct Transform* parent_tran = &parent->transform;
		transform_get_absolute_scale(parent_tran, res);
	}
	vec3_add(res, res, &transform->scale);
}

void transform_get_absolute_rot(struct Transform* transform, quat* res)
{
	struct Entity* parent = entity_get(transform->parent);
	if(parent)
	{
		struct Transform* parent_tran = &parent->transform;
		transform_get_absolute_rot(parent_tran, res);
	}
	quat_mul(res, res, &transform->rotation);
}
