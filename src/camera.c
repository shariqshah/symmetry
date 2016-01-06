#include "GLFW/glfw3.h"
#include "camera.h"
#include "entity.h"
#include "transform.h"
#include "array.h"
#include "framebuffer.h"
#include "texture.h"
#include "bounding_volumes.h"

#include "utils.h"
#include "log.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static struct Camera* camera_list;
static int* empty_indices;
static int primary_camera_index;
static void update_frustum(struct Camera* camera);

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
	primary_camera_index = -1;
}

void camera_remove(int index)
{
	if(index > -1 && index < array_len(camera_list))
	{
		struct Camera* camera = &camera_list[index];
		if(camera->fbo != -1) framebuffer_remove(camera->fbo);
		if(camera->render_tex != -1) texture_remove(camera->render_tex);
		if(camera->depth_tex != -1) texture_remove(camera->depth_tex);
		camera->fbo = camera->render_tex = camera->depth_tex = camera->node = -1;
		array_push(empty_indices, index, int);
	}
}

void camera_cleanup(void)
{
	for(int i = 0; i < array_len(camera_list); i++)
		if(camera_list[i].node != -1) camera_remove(i);
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
	new_camera->fbo = -1;
	new_camera->render_tex = -1;
	new_camera->depth_tex = -1;
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
	vec4_fill(&new_camera->clear_color, 1.f, 1.f, 1.f, 1.f);
	return index;
}

void camera_update_view_proj(struct Camera* camera)
{
	mat4_mul(&camera->view_proj_mat, &camera->proj_mat, &camera->view_mat);
	update_frustum(camera);
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

void camera_attach_fbo(struct Camera* camera, int width, int height, int has_depth, int has_color)
{
	assert(width > 0 && height > 0 && camera);
	if(camera->fbo != -1)
	{
		log_error("camera:attach_fbo", "Camera already has fbo attached!");
		return;
	}
	camera->fbo = framebuffer_create(width, height, has_depth, has_color);
	if(camera->fbo > -1)
	{
		char tex_name[128];
		snprintf(tex_name, 128, "cam_render_tex_%d", camera->node);
		camera->render_tex = texture_create(tex_name,
											TU_DIFFUSE,
											width, height,
											GL_RGBA,
											GL_RGBA8,
											GL_UNSIGNED_BYTE,
											NULL);
		texture_set_param(camera->render_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		texture_set_param(camera->render_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		texture_set_param(camera->render_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture_set_param(camera->render_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		memset(tex_name, '\0', 128);
		snprintf(tex_name, 128, "cam_depth_tex_%d", camera->node);
		camera->depth_tex = texture_create(tex_name,
										   TU_SHADOWMAP1,
										   width, height,
										   GL_DEPTH_COMPONENT,
										   GL_DEPTH_COMPONENT,
										   GL_UNSIGNED_BYTE,
										   NULL);
		texture_set_param(camera->depth_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		texture_set_param(camera->depth_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		texture_set_param(camera->depth_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		texture_set_param(camera->depth_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		texture_set_param(camera->depth_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
		texture_set_param(camera->depth_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		camera->fbo = framebuffer_create(width, height, 0, 1);
		framebuffer_set_texture(camera->fbo, camera->render_tex, GL_COLOR_ATTACHMENT0);
		framebuffer_set_texture(camera->fbo, camera->depth_tex, GL_DEPTH_ATTACHMENT);
	}
	else
	{
		log_error("camera:attach_fbo", "Framebuffer not attached to camera!");
	}
}

struct Camera* camera_get_all(void)
{
	return camera_list;
}

void camera_set_primary_viewer(struct Camera* camera)
{
	assert(camera);
	if(camera->node == -1)
	{
		log_error("camera:set_primary_viewer", "Invalid camera!");
	}
	else
	{
		/* locate the index of this camera */
		for(int i = 0; i < array_len(camera_list); i++)
		{
			if(camera_list[i].node == camera->node)
			{
				primary_camera_index = i;
				break;
			}
			
		}
	}
}

struct Camera* camera_get_primary(void)
{
	struct Camera* primary_camera = NULL;
	if(primary_camera_index != -1)
		primary_camera = &camera_list[primary_camera_index];
	return primary_camera;
}

static void update_frustum(struct Camera* camera)
{
	assert(camera);
	float* mvp = &camera->view_proj_mat.mat[0];

	camera->frustum[FP_LEFT].x   = mvp[3]  + mvp[0];
	camera->frustum[FP_LEFT].y   = mvp[7]  + mvp[4];
	camera->frustum[FP_LEFT].z   = mvp[11] + mvp[2];
	camera->frustum[FP_LEFT].w   = mvp[15] + mvp[12];
 
	camera->frustum[FP_RIGHT].x  = mvp[3]  - mvp[0];
	camera->frustum[FP_RIGHT].y  = mvp[7]  - mvp[4];
	camera->frustum[FP_RIGHT].z  = mvp[11] - mvp[8];
	camera->frustum[FP_RIGHT].w  = mvp[15] - mvp[12];
       
	camera->frustum[FP_BOTTOM].x = mvp[3]  + mvp[1];
	camera->frustum[FP_BOTTOM].y = mvp[11] + mvp[5];
	camera->frustum[FP_BOTTOM].z = mvp[11] + mvp[9];
	camera->frustum[FP_BOTTOM].w = mvp[15] + mvp[13];
 
	camera->frustum[FP_TOP].x    = mvp[3]  - mvp[1];
	camera->frustum[FP_TOP].y    = mvp[7]  - mvp[5];
	camera->frustum[FP_TOP].z    = mvp[11] - mvp[9];
	camera->frustum[FP_TOP].w    = mvp[15] - mvp[13];
 
	camera->frustum[FP_NEAR].x   = mvp[3]  + mvp[2];
	camera->frustum[FP_NEAR].y   = mvp[7]  + mvp[6];
	camera->frustum[FP_NEAR].z   = mvp[11] + mvp[10];
	camera->frustum[FP_NEAR].w   = mvp[15] + mvp[14];
       
	camera->frustum[FP_FAR].x    = mvp[3]  - mvp[2];
	camera->frustum[FP_FAR].y    = mvp[7]  - mvp[6];
	camera->frustum[FP_FAR].z    = mvp[11] - mvp[10];
	camera->frustum[FP_FAR].w    = mvp[15] - mvp[14];

	for(int i = 0; i < 6; i++)
	{
		vec3 plane_xyz = {camera->frustum[i].x, camera->frustum[i].y, camera->frustum[i].z};
		float length = vec3_len(&plane_xyz);
		vec4_scale(&camera->frustum[i], &camera->frustum[i], (1.f / length));
	}
}
