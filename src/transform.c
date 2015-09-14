#include "transform.h"
#include "log.h"
#include "array.h"
#include "entity.h"
#include "utils.h"
#include <assert.h>

static struct Transform* transform_list;
static int* empty_indices;

void transform_init(void)
{
	transform_list = array_new(struct Transform);
	empty_indices = array_new(int);
}

void transform_cleanup(void)
{
	array_free(transform_list);
	array_free(empty_indices);
}

int transform_create(int node)
{
	int index = -1;
	if(node > -1)
	{
		struct Transform* new_transform = NULL;
		if(array_len(empty_indices) > 0)
		{
			index = *array_get_last(empty_indices, int);
			array_pop(empty_indices);
			new_transform = &transform_list[index];
		}
		else
		{
			new_transform = array_grow(transform_list, struct Transform);
			index = array_len(transform_list) - 1;
		}
		new_transform->node = node;
		vec3_fill(&new_transform->position, 0.f, 0.f, 0.f);
		vec3_fill(&new_transform->scale, 1.f, 1.f, 1.f);
		quat_identity(&new_transform->rotation);
		transform_update_transmat(new_transform);
	}
	return index;
}

void transform_translate(struct Transform* transform, vec3* amount, enum Transform_Space space)
{
	if(space == TS_LOCAL)
	{
		quat_mul_vec3(amount, &transform->rotation, amount);
	}
	else if(space == TS_PARENT)
	{
		struct Entity* parent = entity_get_parent(transform->node);
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		quat_mul_vec3(amount, &parent_tran->rotation, amount);
	}
	vec3_add(&transform->position, &transform->position, amount);
	transform_update_transmat(transform);
}
void transform_rotate(struct Transform* transform,
					  vec3*  axis,
					  float angle,
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
	
	struct Entity* entity = entity_get(transform->node);
	struct Entity* parent = entity_get(entity->parent);
	if(parent)
	{
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		mat4_mul(&transform->trans_mat, &transform->trans_mat, &parent_tran->trans_mat);
	}

	/* Update all children */
	if(array_len(entity->children) > 0)
	{
		for(int i = 0; i < array_len(entity->children); i++)
		{
			struct Entity* child = entity_get(entity->children[i]);
			struct Transform* child_tran = entity_component_get(child, C_TRANSFORM);
			transform_update_transmat(child_tran);
		}
	}
	
	entity_sync_components(entity);
}

struct Transform* transform_get(int index)
{
	assert(index > -1 && index < array_len(transform_list));
	return &transform_list[index];
}

void transform_remove(int index)
{
	assert(index > -1 && index < array_len(transform_list));
	transform_list[index].node = -1;
	array_push(empty_indices, index, int);
}

void transform_set_position(struct Transform* transform, vec3* new_position)
{
	vec3_assign(&transform->position, new_position);
	transform_update_transmat(transform);
}

void transform_get_absolute_pos(struct Transform* transform, vec3* res)
{
	struct Entity* entity = entity_get(transform->node);
	struct Entity* parent = entity_get(entity->parent);
	if(parent)
	{
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		transform_get_absolute_pos(parent_tran, res);
	}
	vec3_add(res, res, &transform->position);
}

void transform_get_absolute_rot(struct Transform* transform, quat* res)
{
	struct Entity* entity = entity_get(transform->node);
	struct Entity* parent = entity_get(entity->parent);
	if(parent)
	{
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		transform_get_absolute_rot(parent_tran, res);
	}
	quat_mul(res, res, &transform->rotation);
}

