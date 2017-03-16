#include "renderer.h"
#include "gl_load.h"

#include "log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "array.h"
#include "geometry.h"
#include "shader.h"
#include "num_types.h"
#include "light.h"
#include "entity.h"
#include "transform.h"
#include "game.h"
#include "gui.h"

static int def_fbo = -1;
static int def_albedo_tex = -1;
static int def_depth_tex = -1;
static int quad_geo = -1;
static int composition_shader = -1;
static struct Render_Settings settings;

#define MAX_GUI_VERTEX_MEMORY  512 * 1024
#define MAX_GUI_ELEMENT_MEMORY 128 * 1024

void on_framebuffer_size_change(int width, int height);

void renderer_init(void)
{
	glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	platform_windowresize_callback_set(on_framebuffer_size_change);
	
	settings.fog.mode = FM_EXPONENTIAL;
	settings.fog.density = 0.01f;
	settings.fog.start_dist = 50.f;
	settings.fog.max_dist = 150.f;
	vec3_fill(&settings.fog.color, 60.f/255.f, 60.f/255.f, 75.f/255.f);
	vec3_fill(&settings.ambient_light, 0.1f, 0.1f, 0.12f);
	settings.max_gui_vertex_memory  = MAX_GUI_VERTEX_MEMORY;
	settings.max_gui_element_memory = MAX_GUI_ELEMENT_MEMORY;

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
	/* framebuffer_set_texture(def_fbo, def_depth_tex, GL_DEPTH_ATTACHMENT); */
	composition_shader = shader_create("fbo.vert", "fbo.frag");
}

void renderer_draw(void)
{
	struct Camera* camera_list = camera_get_all();
	for(int i = 0; i < array_len(camera_list); i++)
	{
		struct Camera* camera = &camera_list[i];
		if(camera->node < 0)
			continue;

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
			model_render_all(camera);
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
	struct Camera* active_camera = camera_get_primary();
	int final_render_tex = active_camera->render_tex == -1 ? def_albedo_tex : active_camera->render_tex;
	texture_bind(final_render_tex);
	geom_render(quad_geo);
	texture_unbind(final_render_tex);
	shader_unbind();

	gui_render(NK_ANTI_ALIASING_ON, settings.max_gui_vertex_memory, settings.max_gui_element_memory);
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
	glViewport(0, 0, width, height);
	struct Camera* camera = camera_get(0); /* TODO: This is an ugly hack, remove it as soon as possible */
	float aspect = (float)width / (float)height;
	camera->aspect_ratio = aspect > 0.f ? aspect : 4.f / 3.f;
	camera_update_proj(camera);
	//framebuffer_resize(def_fbo, width, height);
	framebuffer_resize_all(width, height);
}

void renderer_set_clearcolor(float red, float green, float blue, float alpha)
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

struct Render_Settings* renderer_get_settings(void)
{
	return &settings;
}
