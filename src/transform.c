#include "transform.h"
#include "array.h"
#include <assert.h>

struct Transform* transform_list;
int* empty_indices;

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
		quat_identity(new_transform->rotation);
		transform_update_transmat(new_transform);
	}
	return index;
}

void transform_translate(struct Transform* transform, vec3 amount, enum Transform_Space space)
{
	if(space == TS_LOCAL)
		quat_mul_vec3(amount, transform->rotation, amount);	
	vec3_add(transform->position, transform->position, amount);
	transform_update_transmat(transform);
}
void transform_rotate(struct Transform* transform,
					  vec3  axis,
					  float angle,
					  enum Transform_Space space)
{
	quat new_rot;
	quat_identity(new_rot);
	quat_rotate(new_rot, angle, axis);
	quat_norm(new_rot, new_rot);
	if(space == TS_LOCAL)
		quat_mul(transform->rotation, transform->rotation, new_rot);
	else if(space == TS_WORLD)
		quat_mul(transform->rotation, new_rot, transform->rotation);
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
	res[0] = 0; res[1] = 0; res[2] = 1;
	quat_mul_vec3(res, transform->rotation, res);
}

void transform_get_up(struct Transform* transform, vec3 res)
{
	res[0] = 0; res[1] = 1; res[2] = 0;
	quat_mul_vec3(res, transform->rotation, res);
}

void transform_get_right(struct Transform* transform, vec3 res)
{
	res[0] = 1; res[1] = 0; res[2] = 0;
	quat_mul_vec3(res, transform->rotation, res);
}


void transform_update_transmat(struct Transform* transform)
{
	mat4 scale, rot, tran;
	mat4_scale_aniso(scale, scale, transform->scale[0], transform->scale[1], transform->scale[2]);
	mat4_from_quat(rot, transform->rotation);
	mat4_translate(tran, transform->position[0], transform->position[1], transform->position[2]);
	mat4_mul(transform->trans_mat, transform->trans_mat, scale);
	mat4_mul(transform->trans_mat, transform->trans_mat, rot);
	mat4_mul(transform->trans_mat, transform->trans_mat, tran);
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
