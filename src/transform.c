#include "transform.h"
#include "log.h"
#include "array.h"
#include "entity.h"
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
		new_transform->position[0] = new_transform->position[1] = new_transform->position[2] = 0;
		new_transform->scale[0] = new_transform->scale[1] = new_transform->scale[2] = 1;
		//mat4_identity(new_transform->rotation);
		quat_identity(new_transform->rotq);
		transform_update_transmat(new_transform);
	}
	return index;
}

void transform_translate(struct Transform* transform, vec3 amount, enum Transform_Space space)
{
	if(space == TS_LOCAL)
	{
		mat4 temp;
		mat4_identity(temp);
		mat4_from_quat(temp, transform->rotq);
		mat4_mul_vec3(amount, temp, amount);
		//mat4_mul_vec3(amount, transform->rotation, amount);
		//quat_mul_vec3(amount, transform->rotq, amount);
	}
	if(space == TS_PARENT)
	{
		struct Entity* parent = entity_get_parent(transform->node);
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		//mat4_mul_vec3(amount, parent_tran->rotation, amount);
		quat_mul_vec3(amount, parent_tran->rotq, amount);
	}
	vec3_add(transform->position, transform->position, amount);
	transform_update_transmat(transform);
}
void transform_rotate(struct Transform* transform,
					  vec3  axis,
					  float angle,
					  enum Transform_Space space)
{
	//mat4 new_rot;
	quat new_rotq;
	/* mat4_identity(new_rot); */
	/* mat4_rotate(new_rot, new_rot, axis[0], axis[1], axis[2], TO_RADIANS(angle)); */
	quat_identity(new_rotq);
	quat_rotate(new_rotq, TO_RADIANS(angle), axis);
	
	if(space == TS_LOCAL)
		quat_mul(transform->rotq, transform->rotq, new_rotq);
	else if(space == TS_WORLD)
		quat_mul(transform->rotq, new_rotq, transform->rotq);
	quat_norm(transform->rotq, transform->rotq);
	transform_update_transmat(transform);
}

void transform_scale(struct Transform* transform, vec3 scale)
{
	transform->scale[0] = scale[0];
	transform->scale[1] = scale[1];
	transform->scale[2] = scale[2];
	transform_update_transmat(transform);
}

void transform_get_forward(struct Transform* transform, vec3 res)
{
	res[0] = 0; res[1] = 0; res[2] = -1;
	//mat4_mul_vec3(res, transform->rotation, res);
	mat4 temp;
	mat4_identity(temp);
	mat4_from_quat(temp, transform->rotq);
	mat4_mul_vec3(res, temp, res);
	//quat_mul_vec3(res, transform->rotq, res);
}

void transform_get_lookat(struct Transform* transform, vec3 res)
{
	transform_get_forward(transform, res);
	vec3_add(res, transform->position, res);
}

void transform_get_absolute_forward(struct Transform* transform, vec3 res)
{
	res[0] = 0.f; res[0] = 0.f; res[0] = -1.f;
	//mat4_mul_vec3(res, transform->trans_mat, res);
	quat_mul_vec3(res, transform->rotq, res);
	vec3_norm(res, res);
}

void transform_get_absolute_lookat(struct Transform* transform, vec3 res)
{
	vec3 abs_position = {0.f};
	transform_get_absolute_pos(transform, abs_position);
	transform_get_absolute_forward(transform, res);
	vec3_add(res, abs_position, res);
}

void transform_get_up(struct Transform* transform, vec3 res)
{
	res[0] = 0; res[1] = 1; res[2] = 0;
	//mat4_mul_vec3(res, transform->rotation, res);
	mat4 temp;
	mat4_identity(temp);
	mat4_from_quat(temp, transform->rotq);
	mat4_mul_vec3(res, temp, res);
	//quat_mul_vec3(res, transform->rotq, res);
}

void transform_get_right(struct Transform* transform, vec3 res)
{
	res[0] = 1; res[1] = 0; res[2] = 0;
	//mat4_mul_vec3(res, transform->rotation, res);
	quat_mul_vec3(res, transform->rotq, res);
}

void transform_update_transmat(struct Transform* transform)
{
	static mat4 scale, translation, rot;
	mat4_identity(scale);
	mat4_identity(translation);
	mat4_identity(rot);
	mat4_identity(transform->trans_mat);
	mat4_scale_aniso(scale, scale, transform->scale[0], transform->scale[1], transform->scale[2]);
	mat4_translate(translation, transform->position[0], transform->position[1], transform->position[2]);
	mat4_from_quat(rot, transform->rotq);
	
	mat4_mul(transform->trans_mat, transform->trans_mat, translation);
	//mat4_mul(transform->trans_mat, transform->trans_mat, transform->rotation);
	mat4_mul(transform->trans_mat, transform->trans_mat, rot);
	mat4_mul(transform->trans_mat, transform->trans_mat, scale);
	
	struct Entity* entity = entity_get(transform->node);
	struct Entity* parent = entity_get(entity->parent);
	if(parent)
	{
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		mat4_mul(transform->trans_mat, transform->trans_mat, parent_tran->trans_mat);
		//mat4_mul(transform->trans_mat, parent_tran->trans_mat, transform->trans_mat);
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

void transform_set_position(struct Transform* transform, vec3 new_position)
{
	for(int i = 0; i < 3; i++)
		transform->position[i] = new_position[i];
	transform_update_transmat(transform);
}

void transform_get_absolute_pos(struct Transform* transform, vec3 res)
{
	struct Entity* entity = entity_get(transform->node);
	struct Entity* parent = entity_get(entity->parent);
	if(parent)
	{
		struct Transform* parent_tran = entity_component_get(parent, C_TRANSFORM);
		transform_get_absolute_pos(parent_tran, res);
	}
	vec3_add(res, res, transform->position);
}
