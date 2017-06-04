#include "renderer.h"
#include "gl_load.h"

#include "log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "array.h"
#include "shader.h"
#include "num_types.h"
#include "light.h"
#include "entity.h"
#include "transform.h"
#include "game.h"
#include "gui.h"
#include "config_vars.h"
#include "hashmap.h"
#include "geometry.h"

static int def_fbo            = -1;
static int def_albedo_tex     = -1;
static int def_depth_tex      = -1;
static int quad_geo           = -1;
static int composition_shader = -1;
static int debug_shader       = -1;

void on_framebuffer_size_change(int width, int height);

void renderer_init(void)
{
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	platform_windowresize_callback_set(on_framebuffer_size_change);
	gui_init();
	
	/* Quad geometry for final render */
	vec3* vertices = array_new(vec3);
	vec2* uvs      = array_new(vec2);
	vec3* normals  = array_new(vec3);
	uint* indices  = array_new(uint);
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

	int width = -1, height = -1;
	struct Game_State* game_state = game_state_get();
	window_get_size(game_state->window, &width, &height);
	def_albedo_tex = texture_create("def_albedo_texture",
									TU_DIFFUSE,
									width, height,
									GL_RGB,
									GL_RGB16F,
									GL_FLOAT,
									NULL);
	texture_set_param(def_albedo_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	texture_set_param(def_albedo_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	texture_set_param(def_albedo_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	texture_set_param(def_albedo_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
	texture_set_param(def_depth_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	texture_set_param(def_depth_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	def_fbo = framebuffer_create(width, height, 1, 0, 1);
	framebuffer_set_texture(def_fbo, def_albedo_tex, FA_COLOR_ATTACHMENT0);
	framebuffer_set_texture(def_fbo, def_depth_tex, FA_DEPTH_ATTACHMENT);
	composition_shader = shader_create("fbo.vert", "fbo.frag");
	debug_shader       = shader_create("debug.vert", "debug.frag");
}

void renderer_draw(struct Entity* active_viewer)
{
	struct Entity* entity_list = entity_get_all();
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* viewer = &entity_list[i];
		if(entity_list[i].type != ET_CAMERA) continue;
		struct Camera* camera = &viewer->camera;

		/* if(camera->fbo == -1) continue; */
		int fbo = camera->fbo == -1 ? def_fbo : camera->fbo;
		framebuffer_bind(fbo);
		{
			glViewport(0, 0, framebuffer_get_width(fbo), framebuffer_get_height(fbo));
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			glClearColor(camera->clear_color.x,
						 camera->clear_color.y,
						 camera->clear_color.z,
						 camera->clear_color.w);
			glEnable(GL_CULL_FACE );
			glCullFace(GL_BACK);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			model_render_all(viewer, GDM_TRIANGLES);
		}
		framebuffer_unbind();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}
	
	/* struct Camera* camera = camera_get_primary(); */
	/* { */
	/* 	int width, height; */
	/* 	struct Game_State* game_state = game_state_get(); */
	/* 	window_get_size(game_state->window, &width, &height); */
	/* 	glViewport(0, 0, width, height); */
	/* 	glEnable(GL_DEPTH_TEST); */
	/* 	glDepthFunc(GL_LEQUAL); */
	/* 	glClearColor(camera->clear_color.x, */
	/* 				 camera->clear_color.y, */
	/* 				 camera->clear_color.z, */
	/* 				 camera->clear_color.w); */
	/* 	glEnable(GL_CULL_FACE ); */
	/* 	glCullFace(GL_BACK); */
	/* 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); */
	/* 	model_render_all(camera); */
	/* } */

	int width, height;
	struct Game_State* game_state = game_state_get();
	window_get_size(game_state->window, &width, &height);
	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	shader_bind(composition_shader);
	struct Camera* active_camera = &active_viewer->camera;
	int final_render_tex = active_camera->render_tex == -1 ? def_albedo_tex : active_camera->render_tex;
	texture_bind(final_render_tex);
	geom_render(quad_geo, GDM_TRIANGLES);
	texture_unbind(final_render_tex);
	shader_unbind();

	struct Hashmap* cvars = config_vars_get();
	//if(settings.debug_draw_enabled)
	if(hashmap_bool_get(cvars, "debug_draw_enabled"))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		//model_render_all_debug(active_viewer, debug_shader, settings.debug_draw_mode, &settings.debug_draw_color);
		vec4 debug_draw_color = hashmap_vec4_get(cvars, "debug_draw_color");
		model_render_all_debug(active_viewer, debug_shader, hashmap_int_get(cvars, "debug_draw_mode"), &debug_draw_color);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	gui_render(NK_ANTI_ALIASING_ON);
}

void renderer_cleanup(void)
{
	gui_cleanup();
	geom_remove(quad_geo);
	framebuffer_remove(def_fbo);
	texture_remove(def_albedo_tex);
	texture_remove(def_depth_tex);
}

void on_framebuffer_size_change(int width, int height)
{
	struct Entity* entity_list = entity_get_all();
	float aspect = (float)width / (float)height;
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* viewer = &entity_list[i];
		if(viewer->type != ET_CAMERA) continue;

		viewer->camera.aspect_ratio = aspect > 0.f ? aspect : 4.f / 3.f;
		camera_update_proj(viewer);
	}
	
	framebuffer_resize_all(width, height);
}

void renderer_clearcolor_set(float red, float green, float blue, float alpha)
{
	glClearColor(red, green, blue, alpha);
}

int renderer_check_glerror(const char* context)
{
	int error = 1;
	GLenum error_code = glGetError();
	const char* error_string = "No Error";
	switch(error_code)
	{
	case GL_INVALID_OPERATION: 			   error_string = "Invalid Operation"; 		       break;
	case GL_NO_ERROR:		   			   error_string = "No Error";		  		       break;
	case GL_INVALID_ENUM:	   			   error_string = "Invalid ENUM";	  		       break;
	case GL_INVALID_VALUE:	   			   error_string = "Invalid Value";	  		       break;
	case GL_INVALID_FRAMEBUFFER_OPERATION: error_string = "Invalid FrameBuffer Operation"; break;
	case GL_OUT_OF_MEMORY:		           error_string = "Out of Memory";		           break;
	}

	if(error_code != GL_NO_ERROR)
		log_error(context, error_string);
	else
		error = 0;

	return error;
}

void renderer_debug_draw_enabled(bool enabled)
{
	struct Hashmap* cvars = config_vars_get();
	hashmap_bool_set(cvars,  "debug_draw_enabled", enabled);
}

void renderer_settings_get(struct Render_Settings* settings)
{
	struct Hashmap* cvars = config_vars_get();
	settings->fog.mode               = hashmap_int_get(cvars,   "fog_mode");
	settings->fog.density            = hashmap_float_get(cvars, "fog_density");
	settings->fog.start_dist         = hashmap_float_get(cvars, "fog_start_dist");
	settings->fog.max_dist           = hashmap_float_get(cvars, "fog_max_dist");
	settings->fog.color              = hashmap_vec3_get(cvars,  "fog_color");
	settings->debug_draw_enabled     = hashmap_bool_get(cvars,  "debug_draw_enabled");
	settings->debug_draw_mode        = hashmap_int_get(cvars,   "debug_draw_mode");
	settings->debug_draw_color       = hashmap_vec4_get(cvars,  "debug_draw_color");
	settings->ambient_light          = hashmap_vec3_get(cvars,  "ambient_light");
}

void renderer_settings_set(const struct Render_Settings* settings)
{
	struct Hashmap* cvars = config_vars_get();
	hashmap_int_set(cvars,   "fog_mode",           settings->fog.mode);
	hashmap_float_set(cvars, "fog_density",        settings->fog.density);
	hashmap_float_set(cvars, "fog_start_dist",     settings->fog.start_dist);
	hashmap_float_set(cvars, "fog_max_dist",       settings->fog.max_dist);
	hashmap_bool_set(cvars,  "debug_draw_enabled", settings->debug_draw_enabled);
	hashmap_int_set(cvars,   "debug_draw_mode",    settings->debug_draw_mode);
	hashmap_vec4_set(cvars,  "debug_draw_color",   &settings->debug_draw_color);
	hashmap_vec3_set(cvars,  "ambient_light",      &settings->ambient_light);
}
