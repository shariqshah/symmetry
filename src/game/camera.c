#include "camera.h"
#include "entity.h"
#include "transform.h"
#include "entity.h"
#include "../common/array.h"
#include "framebuffer.h"
#include "texture.h"
#include "game.h"
#include "bounding_volumes.h"

#include "../common/utils.h"
#include "../common/log.h"
#include "gl_load.h"
#include "../system/platform.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

static void update_frustum(struct Camera* camera);

void camera_reset(struct Camera* camera)
{
	if(camera->fbo != -1)        framebuffer_remove(camera->fbo);
	if(camera->render_tex != -1) texture_remove(camera->render_tex);
	if(camera->depth_tex != -1)  texture_remove(camera->depth_tex);
	camera->fbo = camera->render_tex = camera->depth_tex = -1;
	camera->ortho = camera->resizeable = false;
	camera->fov = camera->aspect_ratio = camera->nearz = camera->farz = 0.f;
	mat4_identity(&camera->view_mat);
	mat4_identity(&camera->proj_mat);
	mat4_identity(&camera->view_proj_mat);
	for(int i = 0; i < FP_NUM_PLANES; i++)
		vec4_fill(&camera->frustum[i], 0.f, 0.f, 0.f, 0.f);
	vec4_fill(&camera->clear_color, 0.f, 1.f, 0.f, 1.0);
}

void camera_init(struct Camera* camera, int width, int height)
{
	camera->base.type = ET_CAMERA;
	camera->fbo = -1;
	camera->render_tex = -1;
	camera->depth_tex = -1;
	camera->farz = 1000.f;
	camera->nearz = 0.1f;
	camera->fov = 60.f;
	camera->ortho = false;
	camera->resizeable = true;
	camera->zoom = 1.f;
	float aspect_ratio = (float)width / (float)height;
	camera->aspect_ratio = aspect_ratio <= 0.f ? (4.f / 3.f) : aspect_ratio;
	mat4_identity(&camera->view_mat);
	mat4_identity(&camera->proj_mat);
	mat4_identity(&camera->view_proj_mat);
	for(int i = 0; i < FP_NUM_PLANES; i++)
		vec4_fill(&camera->frustum[i], 0.f, 0.f, 0.f, 0.f);
	camera_update_view(camera);
	camera_update_proj(camera);
	vec4_fill(&camera->clear_color, 1.f, 1.f, 1.f, 1.f);
}

void camera_update_view_proj(struct Camera* camera)
{
	mat4_identity(&camera->view_proj_mat);
	mat4_mul(&camera->view_proj_mat, &camera->proj_mat, &camera->view_mat);
	update_frustum(camera);
}

void camera_update_view(struct Camera* camera)
{
	vec3 lookat = { 0.f, 0.f, 0.f };
	vec3 up = { 0.f, 0.f, 0.f };
	vec3 position = { 0.f, 0.f, 0.f };
	transform_get_absolute_lookat(&camera->base, &lookat);
	transform_get_absolute_up(&camera->base, &up);
	transform_get_absolute_position(&camera->base, &position);
	mat4_lookat(&camera->view_mat, &position, &lookat, &up);
	camera_update_view_proj(camera);
}

void camera_update_proj(struct Camera* camera)
{
	if(!camera->ortho)
	{
		mat4_perspective(&camera->proj_mat,
			camera->fov / camera->zoom,
			camera->aspect_ratio,
			camera->nearz,
			camera->farz);
	}
	else
	{
		int width, height;
		struct Game_State* game_state = game_state_get();
		window_get_size(game_state->window, &width, &height);
		mat4_ortho(&camera->proj_mat,
			       -width  / camera->zoom,
			        width  / camera->zoom,
			        height / camera->zoom,
			       -height / camera->zoom,
			        camera->nearz,
			        camera->farz);
	}
	camera_update_view_proj(camera);
}

void camera_attach_fbo(struct Camera* camera,
	int            width,
	int            height,
	bool           has_depth,
	bool           has_color,
	bool           resizeable)
{
	assert(width > 0 && height > 0);
	if(camera->fbo != -1)
	{
		log_error("camera:attach_fbo", "Camera already has fbo attached!");
		return;
	}
	camera->fbo = framebuffer_create(width, height, 0, 1, resizeable);
	if(camera->fbo > -1)
	{
		if(has_color)
		{
			camera->render_tex = texture_create(NULL,
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
			framebuffer_texture_set(camera->fbo, camera->render_tex, FA_COLOR_ATTACHMENT0);
		}

		if(has_depth)
		{
			camera->depth_tex = texture_create(NULL,
				TU_SHADOWMAP1,
				width, height,
				GL_DEPTH_COMPONENT,
				GL_DEPTH_COMPONENT,
				GL_FLOAT,
				NULL);
			texture_set_param(camera->depth_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			texture_set_param(camera->depth_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			texture_set_param(camera->depth_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture_set_param(camera->depth_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			texture_set_param(camera->depth_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			texture_set_param(camera->depth_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			framebuffer_texture_set(camera->fbo, camera->depth_tex, FA_DEPTH_ATTACHMENT);
		}
	}
	else
	{
		log_error("camera:attach_fbo", "Framebuffer not attached to camera!");
	}
}

static void update_frustum(struct Camera* camera)
{
	assert(camera);
	float* mvp = &camera->view_proj_mat.mat[0];

	camera->frustum[FP_LEFT].x = mvp[3] + mvp[0];
	camera->frustum[FP_LEFT].y = mvp[7] + mvp[4];
	camera->frustum[FP_LEFT].z = mvp[11] + mvp[8];
	camera->frustum[FP_LEFT].w = mvp[15] + mvp[12];

	camera->frustum[FP_RIGHT].x = mvp[3] - mvp[0];
	camera->frustum[FP_RIGHT].y = mvp[7] - mvp[4];
	camera->frustum[FP_RIGHT].z = mvp[11] - mvp[8];
	camera->frustum[FP_RIGHT].w = mvp[15] - mvp[12];

	camera->frustum[FP_BOTTOM].x = mvp[3] + mvp[1];
	camera->frustum[FP_BOTTOM].y = mvp[7] + mvp[5];
	camera->frustum[FP_BOTTOM].z = mvp[11] + mvp[9];
	camera->frustum[FP_BOTTOM].w = mvp[15] + mvp[13];

	camera->frustum[FP_TOP].x = mvp[3] - mvp[1];
	camera->frustum[FP_TOP].y = mvp[7] - mvp[5];
	camera->frustum[FP_TOP].z = mvp[11] - mvp[9];
	camera->frustum[FP_TOP].w = mvp[15] - mvp[13];

	camera->frustum[FP_NEAR].x = mvp[3] + mvp[2];
	camera->frustum[FP_NEAR].y = mvp[7] + mvp[6];
	camera->frustum[FP_NEAR].z = mvp[11] + mvp[10];
	camera->frustum[FP_NEAR].w = mvp[15] + mvp[14];

	camera->frustum[FP_FAR].x = mvp[3] - mvp[2];
	camera->frustum[FP_FAR].y = mvp[7] - mvp[6];
	camera->frustum[FP_FAR].z = mvp[11] - mvp[10];
	camera->frustum[FP_FAR].w = mvp[15] - mvp[14];

	for(int i = 0; i < FP_NUM_PLANES; i++)
	{
		//vec3 plane_xyz = { camera->frustum[i].x, camera->frustum[i].y, camera->frustum[i].z };
		//float length = fabsf(vec3_len(&plane_xyz));
		//vec4_scale(&camera->frustum[i], &camera->frustum[i], (1.f / length));
		vec4_norm(&camera->frustum[i], &camera->frustum[i]);
	}
}

struct Ray camera_screen_coord_to_ray(struct Camera* camera, int mouse_x, int mouse_y)
{
	struct Ray ray;

	int win_width = 0, win_height = 0;
	struct Game_State* game_state = game_state_get();
	window_get_drawable_size(game_state->window, &win_width, &win_height);

	float normalized_x = (2.f * (float)mouse_x) / (float)win_width - 1.f;
	float normalized_y = 1.f - (2.f * (float)mouse_y) / (float)win_height;

	vec3 near_point = { 0.f };
	vec3 far_point = { 0.f };

	mat4 inverse_view_proj_mat;
	mat4_identity(&inverse_view_proj_mat);
	mat4_inverse(&inverse_view_proj_mat, &camera->view_proj_mat);

	//Project the near point
	quat rot_near = { normalized_x, normalized_y, 0.f, 1.f };
	quat_mul_mat4(&rot_near, &rot_near, &inverse_view_proj_mat);
	near_point.x = rot_near.x / rot_near.w;
	near_point.y = rot_near.y / rot_near.w;
	near_point.z = rot_near.z / rot_near.w;

	//Project far point
	quat rot_far = { normalized_x, normalized_y, 1.f, 1.f };
	quat_mul_mat4(&rot_far, &rot_far, &inverse_view_proj_mat);
	far_point.x = rot_far.x / rot_far.w;
	far_point.y = rot_far.y / rot_far.w;
	far_point.z = rot_far.z / rot_far.w;

	vec3_sub(&ray.direction, &far_point, &near_point);
	vec3_norm(&ray.direction, &ray.direction);

	transform_get_absolute_position(&camera->base, &ray.origin);

	return ray;
}
