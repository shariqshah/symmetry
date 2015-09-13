#include "camera.h"
#include "entity.h"
#include "transform.h"
#include "array.h"

#include "utils.h"
#include "log.h"

static struct Camera* camera_list;
static int* empty_indices;

struct Camera* camera_get(int index)
{
	struct Camera* camera = NULL;
	if(index > -1 && index < array_len(camera_list))
		camera = &camera_list[index];

	return camera;
}

void camera_init(void)
{
	camera_list = array_new(struct Camera);
	empty_indices = array_new(int);
}

void camera_remove(int index)
{
	if(index > -1 && index < array_len(camera_list))
	{
		camera_list->node = -1;
		array_push(empty_indices, index, int);
	}
}

void camera_cleanup(void)
{
	array_free(camera_list);
	array_free(empty_indices);
}

int camera_create(int node, int width, int height)
{
	int index = -1;
	struct Camera* new_camera = NULL;
	if(array_len(empty_indices) > 0)
	{
		index = *array_get_last(empty_indices, int);
		new_camera = &camera_list[index];
		array_pop(empty_indices);
	}
	else
	{
		new_camera = array_grow(camera_list, struct Camera);
		index = array_len(camera_list) - 1;
	}

	new_camera->node = node;
	new_camera->farz = 1000.f;
	new_camera->nearz = 0.1f;
	new_camera->fov = 60.f;
	float aspect_ratio = (float)width / (float)height;
	new_camera->aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;
	new_camera->ortho = 0;
	mat4_identity(&new_camera->view_mat);
	mat4_identity(&new_camera->proj_mat);
	mat4_identity(&new_camera->view_proj_mat);
	camera_update_view(new_camera);
	camera_update_proj(new_camera);

	return index;
}

void camera_update_view_proj(struct Camera* camera)
{
	mat4_mul(&camera->view_proj_mat, &camera->proj_mat, &camera->view_mat);
}

void camera_update_view(struct Camera* camera)
{
	struct Entity* entity = entity_get(camera->node);
	struct Transform* transform = entity_component_get(entity, C_TRANSFORM);
	vec3 lookat = {0.f, 0.f, 0.f};
	vec3 up = {0.f, 0.f, 0.f};
	vec3 position = {0.f, 0.f, 0.f};
	transform_get_absolute_lookat(transform, &lookat);
	transform_get_absolute_up(transform, &up);
	transform_get_absolute_pos(transform, &position);
	mat4_lookat(&camera->view_mat, &position, &lookat, &up);
	camera_update_view_proj(camera);
}

void camera_update_proj(struct Camera* camera)
{
	if(!camera->ortho)
	{
		mat4_perspective(&camera->proj_mat,
						 camera->fov,
						 camera->aspect_ratio,
						 camera->nearz,
						 camera->farz);
	}
	else
	{
		mat4_ortho(&camera->proj_mat, -1, 1, -1, 1, camera->nearz, camera->farz);
	}
	camera_update_view_proj(camera);
}
