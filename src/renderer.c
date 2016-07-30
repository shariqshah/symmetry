#include "renderer.h"
#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "array.h"
#include "geometry.h"
#include "shader.h"
#include "num_types.h"
#include "window_system.h"
#include "light.h"
#include "entity.h"
#include "transform.h"

static int def_fbo = -1;
static int light_fbo = -1;
static int light_tex = -1;
static int def_albedo_tex = -1;
static int def_position_tex = -1;
static int def_normal_tex = -1;
static int def_uv_tex = -1;
static int def_depth_tex = -1;
static int quad_geo = -1;
static int fbo_shader = -1;
static int composition_shader = -1;

void on_framebuffer_size_change(GLFWwindow* window, int width, int height);

void renderer_init(GLFWwindow* window)
{
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glfwSetFramebufferSizeCallback(window, on_framebuffer_size_change);

	/* Quad geometry for final render */
	vec3* vertices = array_new(vec3);
	vec2* uvs = array_new(vec2);
	vec3* normals = array_new(vec3);
	uint* indices = array_new(uint);
	vec3 temp_v3; 
	vec2 temp_v2;
	/* Vertices */
	temp_v3.x = -1; temp_v3.y = -1; temp_v3.z = 0;	array_push(vertices, temp_v3, vec3);
	temp_v3.x =  1; temp_v3.y = -1; temp_v3.z = 0;	array_push(vertices, temp_v3, vec3);
	temp_v3.x =  1; temp_v3.y =  1; temp_v3.z = 0;	array_push(vertices, temp_v3, vec3);
	temp_v3.x = -1; temp_v3.y =  1; temp_v3.z = 0;	array_push(vertices, temp_v3, vec3);
	/* Normals */
	temp_v3.x = 0; temp_v3.y = 1; temp_v3.z = 0;	array_push(normals, temp_v3, vec3);
	temp_v3.x = 0; temp_v3.y = 1; temp_v3.z = 0;	array_push(normals, temp_v3, vec3);
	/* Uvs */
	temp_v2.x = 0; temp_v2.y = 0; array_push(uvs, temp_v2, vec2);
	temp_v2.x = 1; temp_v2.y = 0; array_push(uvs, temp_v2, vec2);
	temp_v2.x = 1; temp_v2.y = 1; array_push(uvs, temp_v2, vec2);
	temp_v2.x = 0; temp_v2.y = 1; array_push(uvs, temp_v2, vec2);
	/* Indices */
	array_push(indices, 0, uint); array_push(indices, 1, uint); array_push(indices, 2, uint);
	array_push(indices, 2, uint); array_push(indices, 3, uint); array_push(indices, 0, uint);
	
	quad_geo = geom_create("Quad", vertices, uvs, normals, indices, NULL);
	array_free(vertices);
	array_free(uvs);
	array_free(normals);
	array_free(indices);

/* Textues for default fbo */
	int width = -1, height = -1;
	window_get_size(&width, &height);
	def_albedo_tex = texture_create("def_albedo_texture",
									TU_DIFFUSE,
									width, height,
									GL_RGB,
									GL_RGB16F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_albedo_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_albedo_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_albedo_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	texture_set_param(def_albedo_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	def_position_tex = texture_create("def_position_texture",
									TU_SHADOWMAP1,
									width, height,
									GL_RGB,
									GL_RGB16F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_position_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_position_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_position_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(def_position_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	def_normal_tex = texture_create("def_normal_texture",
									TU_SHADOWMAP2,
									width, height,
									GL_RGB,
									GL_RGB16F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_normal_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_normal_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_normal_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(def_normal_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	def_uv_tex = texture_create("def_uv_texture",
									TU_SHADOWMAP3,
									width, height,
									GL_RGB,
									GL_RGB16F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_uv_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_uv_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_uv_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(def_uv_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	def_depth_tex = texture_create("def_depth_texture",
									TU_SHADOWMAP4,
									width, height,
									GL_DEPTH_COMPONENT,
									GL_DEPTH_COMPONENT32F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_depth_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_depth_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_depth_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(def_depth_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	texture_set_param(def_depth_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	texture_set_param(def_depth_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	def_fbo = framebuffer_create(width, height, 1, 1);
	framebuffer_set_texture(def_fbo, def_albedo_tex, GL_COLOR_ATTACHMENT0);
	/* framebuffer_set_texture(def_fbo, def_position_tex, GL_COLOR_ATTACHMENT0 + 1); */
	/* framebuffer_set_texture(def_fbo, def_normal_tex, GL_COLOR_ATTACHMENT0 + 2); */
	/* framebuffer_set_texture(def_fbo, def_uv_tex, GL_COLOR_ATTACHMENT0 + 3); */
	framebuffer_set_texture(def_fbo, def_depth_tex, GL_DEPTH_ATTACHMENT);


	light_tex = texture_create("light_tex",
							   TU_SHADOWMAP1,
							   width, height,
							   GL_RGB,
							   GL_RGB16F,
							   GL_FLOAT,
							   NULL);
	texture_set_param(light_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(light_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(light_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	texture_set_param(light_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	light_fbo = framebuffer_create(width, height, 0, 1);
	framebuffer_set_texture(light_fbo, light_tex, GL_COLOR_ATTACHMENT0);
	
	composition_shader = shader_create("fbo.vert", "fbo.frag");
	//fbo_shader = shader_create("deferred_light.vert", "deferred_light.frag");
}

void renderer_draw(void)
{
	struct Camera* camera_list = camera_get_all();
	for(int i = 0; i < array_len(camera_list); i++)
	{
		struct Camera* camera = &camera_list[i];
		if(camera->node < 0)
			continue;

		int fbo = camera->fbo == -1 ? def_fbo : camera->fbo;
		framebuffer_bind(fbo);
		{
			glViewport(0, 0, framebuffer_get_width(fbo), framebuffer_get_height(fbo));
			/* GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, */
			/* 						 GL_COLOR_ATTACHMENT1, */
			/* 						 GL_COLOR_ATTACHMENT2, */
			/* 						 GL_COLOR_ATTACHMENT3}; */
			/* glDrawBuffers(4, &draw_buffers[0]); */
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glClearColor(camera->clear_color.x,
						 camera->clear_color.y,
						 camera->clear_color.z,
						 camera->clear_color.w);
			glEnable(GL_CULL_FACE );
			glCullFace(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			model_render_all(camera);
		}
		framebuffer_unbind();
	}

	int width, height;
	window_get_size(&width, &height);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader_bind(composition_shader);

	//shader_set_uniform_int(composition_shader, "albedo_map", (GL_TEXTURE0 + TU_DIFFUSE) - GL_TEXTURE0);
	texture_bind(def_albedo_tex);
	
	geom_render(quad_geo);
	
	shader_unbind();
	
	texture_unbind(def_albedo_tex);
	
	/* int width, height; */
	/* window_get_size(&width, &height); */
	/* struct Camera* active_camera = camera_get_primary(); */
	/* struct Entity* camera_entity = entity_get(active_camera->node); */
	/* struct Transform* camera_tran = entity_component_get(camera_entity, C_TRANSFORM); */

	/* framebuffer_bind(light_fbo); */
	/* { */
	/* 	glViewport(0, 0, width, height); */
	/* 	glClearColor(0.f, 0.f, 0.f, 1.0f); */
	/* 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
	/* 	//glEnable(GL_BLEND); */
	/* 	//glEnable(GL_DEPTH_TEST ); */
	/* 	/\* glDisable(GL_DEPTH_TEST ); *\/ */
	/* 	/\* glEnable(GL_CULL_FACE ); *\/ */
	/* 	/\* glCullFace(GL_BACK); *\/ */
	/* 	/\* glDepthFunc(GL_LEQUAL); *\/ */
	/* 	/\* glBlendEquation(GL_FUNC_ADD); *\/ */
	/* 	/\* glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); *\/ */

	/* 	/\* uint gbuffer_fbo_handle = 0; *\/ */
	/* 	/\* gbuffer_fbo_handle = framebuffer_get_gl_handle(def_fbo); *\/ */
	/* 	/\* glBindFramebuffer(GL_READ_FRAMEBUFFER, gbuffer_fbo_handle); *\/ */
	/* 	/\* glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); *\/ */
	/* 	/\* glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST); *\/ */
	/* 	/\* glBindFramebuffer(GL_FRAMEBUFFER, 0); *\/ */


	/* 	shader_bind(fbo_shader); */

	/* 	vec3 camera_pos = {0, 0, 0}; */
	/* 	transform_get_absolute_pos(camera_tran, &camera_pos); */
	/* 	shader_set_uniform_vec3(fbo_shader, "camera_pos", &camera_pos); */

	/* 	shader_set_uniform_int(fbo_shader, "albedo_map", (GL_TEXTURE0 + TU_DIFFUSE) - GL_TEXTURE0); */
	/* 	texture_bind(def_albedo_tex); */

	/* 	shader_set_uniform_int(fbo_shader, "position_map", (GL_TEXTURE0 + TU_SHADOWMAP1) - GL_TEXTURE0); */
	/* 	texture_bind(def_position_tex); */
	
	/* 	shader_set_uniform_int(fbo_shader, "normal_map", (GL_TEXTURE0 + TU_SHADOWMAP2) - GL_TEXTURE0); */
	/* 	texture_bind(def_normal_tex); */

	/* 	/\* shader_set_uniform_int(fbo_shader, "depth_map", (GL_TEXTURE0 + TU_SHADOWMAP4) - GL_TEXTURE0); *\/ */
	/* 	/\* texture_bind(def_depth_tex); *\/ */

	/* 	/\* shader_set_uniform_int(fbo_shader, "uv_map", (GL_TEXTURE0 + TU_SHADOWMAP3) - GL_TEXTURE0); *\/ */
	/* 	/\* texture_bind(def_uv_tex); *\/ */

	/* 	struct Light* active_lights = light_get_all(); */
	/* 	int sphere_geo = geom_find("sphere.pamesh"); */
	/* 	mat4 mvp; */
	/* 	struct Transform sphere_tran; */

	/* 	vec2 screen_size = {width, height}; */
	/* 	shader_set_uniform_vec2(fbo_shader, "screen_size", &screen_size); */
	
	/* 	for(int i = 0; i < array_len(active_lights); i++) */
	/* 	{	 */
	/* 		if(i == 0) */
	/* 			glBlendFunc(GL_ZERO, GL_ONE); */
	/* 		else */
	/* 			glBlendFunc(GL_ONE, GL_ONE); */
		
	/* 		/\* Set light uniform *\/ */
	/* 		struct Light* light = light_get(i); /\* TODO: Remove this and do it properly *\/ */
	/* 		struct Entity* light_entity = entity_get(light->node); */
	/* 		struct Transform* transform = entity_component_get(light_entity, C_TRANSFORM); */
	/* 		vec3 light_pos = {0, 0, 0}; */
	/* 		transform_get_absolute_pos(transform, &light_pos); */
	/* 		shader_set_uniform_vec3(fbo_shader, "light.position", &light_pos); */
	/* 		shader_set_uniform_vec4(fbo_shader, "light.color", &light->color); */
	/* 		shader_set_uniform_float(fbo_shader, "light.outer_angle", light->outer_angle); */
	/* 		shader_set_uniform_float(fbo_shader, "light.inner_angle", light->inner_angle); */
	/* 		shader_set_uniform_float(fbo_shader, "light.falloff", light->falloff); */
	/* 		shader_set_uniform_float(fbo_shader, "light.intensity", light->intensity); */
	/* 		shader_set_uniform_int(fbo_shader, "light.type", light->type); */
	/* 		shader_set_uniform_int(fbo_shader, "light.radius", light->radius); */

	/* 		/\* vec2 planes = {active_camera->nearz, active_camera->farz}; *\/ */
	/* 		/\* //vec2 planes = {active_camera->farz, active_camera->nearz}; *\/ */
	/* 		/\* shader_set_uniform_vec2(fbo_shader, "planes", &planes); *\/ */
			
	/* 		mat4_identity(&mvp); */
	/* 		transform_set_position(&sphere_tran, &light_pos); */
	/* 		vec3 light_scale = {light->radius, light->radius, light->radius}; */
	/* 		transform_scale(&sphere_tran, &light_scale); */
	/* 		mat4_mul(&mvp, &active_camera->view_proj_mat, &sphere_tran.trans_mat); */
	/* 		shader_set_uniform_mat4(fbo_shader, "mvp", &mvp); */
	/* 		/\* mat4 inv_proj_mat; *\/ */
	/* 		/\* mat4_identity(&inv_proj_mat); *\/ */
	/* 		/\* mat4_inverse(&inv_proj_mat, &active_camera->view_proj_mat); *\/ */
	/* 		/\* shader_set_uniform_mat4(fbo_shader, "inv_proj_mat", &inv_proj_mat); *\/ */
	/* 		geom_render(quad_geo); */
	/* 		//geom_render(sphere_geo); */
	/* 	} */

	
	/* 	//struct Camera* primary_camera = camera_get_primary(); */
	/* 	//texture_bind(primary_camera->render_tex); */
	/* 	/\* glViewport(0, height / 2, width / 2, height / 2); *\/ */
	/* 	/\* texture_bind(def_albedo_tex); *\/ */
	/* 	/\* geom_render(quad_geo); *\/ */
	/* 	/\* texture_unbind(def_albedo_tex); *\/ */

	/* 	/\* glViewport(width / 2, height / 2, width / 2, height / 2); *\/ */
	/* 	/\* texture_bind(def_position_tex); *\/ */
	/* 	/\* geom_render(quad_geo); *\/ */
	/* 	/\* texture_unbind(def_position_tex); *\/ */

	/* 	/\* glViewport(0, 0, width / 2, height / 2); *\/ */
	/* 	/\* texture_bind(def_normal_tex); *\/ */
	/* 	/\* geom_render(quad_geo); *\/ */
	/* 	/\* texture_unbind(def_normal_tex); *\/ */

	/* 	/\* glViewport(width / 2, 0, width / 2, height / 2); *\/ */
	/* 	/\* texture_bind(def_depth_tex); *\/ */
	/* 	/\* geom_render(quad_geo); *\/ */
	/* 	/\* texture_unbind(def_depth_tex); *\/ */
	
	/* 	shader_unbind(); */

	
	
	/* 	texture_unbind(def_albedo_tex); */
	/* 	texture_unbind(def_position_tex); */
	/* 	texture_unbind(def_normal_tex); */
	/* 	texture_unbind(def_uv_tex); */
	/* 	glDisable(GL_BLEND); */
	/* } */
	/* framebuffer_unbind(); */
}

void renderer_cleanup(void)
{
	geom_remove(quad_geo);
	framebuffer_remove(def_fbo);
	texture_remove(def_albedo_tex);
	texture_remove(def_depth_tex);
}

void on_framebuffer_size_change(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	struct Camera* camera = camera_get(0);
	float aspect = (float)width / (float)height;
	camera->aspect_ratio = aspect > 0.f ? aspect : 4.f / 3.f;
	camera_update_proj(camera);
}

void renderer_set_clearcolor(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

int renderer_check_glerror(const char* context)
{
	int error = 1;
	GLenum error_code = glGetError();
	const char* errorString = "No Error";
	switch(error_code)
	{
	case GL_INVALID_OPERATION:
		errorString = "Invalid Operation";
		break;
	case GL_NO_ERROR:
		errorString = "No Error";
		break;
	case GL_INVALID_ENUM:
		errorString = "Invalid ENUM";
		break;
	case GL_INVALID_VALUE:
		errorString = "Invalid Value";
		break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		errorString = "Invalid FrameBuffer Operation";
		break;
	case GL_OUT_OF_MEMORY:
		errorString = "Out of Memory";
		break;
	case GL_STACK_UNDERFLOW:
		errorString = "Stack Underflow";
		break;
	case GL_STACK_OVERFLOW:
		errorString = "Stack Overflow";
		break;
	}

	if(error_code != GL_NO_ERROR)
		log_error(context, errorString);
	else
		error = 0;

	return error;
}
