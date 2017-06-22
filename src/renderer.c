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
#include "material.h"
#include "editor.h"
#include "variant.h"

#include <string.h>
#include <stdio.h>

#define MAX_UNIFORM_NAME_LEN 64

/* TODO: Move all this into a struct called renderer state */
static int def_fbo            = -1;
static int def_albedo_tex     = -1;
static int def_depth_tex      = -1;
static int quad_geo           = -1;
static int composition_shader = -1;
static int debug_shader       = -1;

static int num_culled = 0, num_rendered = 0, num_indices = 0;
static int num_culled_slot = -1, num_rendered_slot = -1, num_indices_slot = -1;


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

	num_culled_slot   = editor_debugvar_slot_create("Culled Geom",   VT_INT);
	num_rendered_slot = editor_debugvar_slot_create("Rendered Geom", VT_INT);
	num_indices_slot  = editor_debugvar_slot_create("Total Indices", VT_INT);
}

void renderer_draw(struct Entity* active_viewer)
{
	/* Render each camera output into it's framebuffer or to the default framebuffer */
	struct Entity* entity_list = entity_get_all();
	for(int i = 0; i < array_len(entity_list); i++)
	{
		struct Entity* viewer = &entity_list[i];
		if(entity_list[i].type != ET_CAMERA) continue;
		struct Camera* camera = &viewer->camera;

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
			static mat4 mvp;
			struct Material* material_list = material_get_all_materials();
			for(int i = 0; i < array_len(material_list); i++)
			{
				/* for each material, get all the registered models and render them */
				struct Material* material = &material_list[i];
				if(!material->active || array_len(material->registered_models) == 0)
					continue;

				shader_bind(material->shader);
				renderer_check_glerror("model:render_all:shader_bind");

				if(material->lit)	/* Set light information */
				{
					int valid_light_count = 0;
					int* light_index_list = light_get_valid_indices(&valid_light_count);
					char uniform_name[MAX_UNIFORM_NAME_LEN];
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					for(int i = 0; i < valid_light_count; i++)
					{
						struct Entity* light_entity = entity_get(light_index_list[i]);
						struct Light*  light        = &light_entity->light; /* TODO: Cull lights according to camera frustum */
						vec3 light_pos = {0, 0, 0};
						transform_get_absolute_pos(light_entity, &light_pos);

						if(light->type != LT_POINT)
						{
							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].direction", i);
							vec3 light_dir = {0.f, 0.f, 0.f};
							transform_get_absolute_lookat(light_entity, &light_dir);
							vec3_norm(&light_dir, &light_dir);
							shader_set_uniform_vec3(material->shader, uniform_name, &light_dir);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
						}

						if(light->type != LT_DIR)
						{
							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].position", i);
							shader_set_uniform_vec3(material->shader,  uniform_name, &light_pos);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].outer_angle", i);
							shader_set_uniform_float(material->shader, uniform_name, light->outer_angle);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					
							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].inner_angle", i);
							shader_set_uniform_float(material->shader, uniform_name, light->inner_angle);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					
							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].falloff", i);
							shader_set_uniform_float(material->shader, uniform_name, light->falloff);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

							snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].radius", i);
							shader_set_uniform_int(material->shader, uniform_name, light->radius);
							memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
						}
					
						snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].color", i);
						shader_set_uniform_vec3(material->shader,  uniform_name, &light->color);
						memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					
						snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].intensity", i);
						shader_set_uniform_float(material->shader, uniform_name, light->intensity);
						memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					
						snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].type", i);
						shader_set_uniform_int(material->shader, uniform_name, light->type);
						memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
					}

					shader_set_uniform_int(material->shader, "total_active_lights", valid_light_count);
					vec3 camera_pos = {0, 0, 0};
					transform_get_absolute_pos(viewer, &camera_pos);
					shader_set_uniform_vec3(material->shader, "camera_pos", &camera_pos);
				}

				/* Set material pipeline uniforms */
				static struct Render_Settings render_settings;
				renderer_settings_get(&render_settings);
				for(int k = 0; k < array_len(material->pipeline_params); k++)
				{
					struct Uniform* uniform = &material->pipeline_params[k];
					if(strcmp(uniform->name, "view_mat") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &viewer->camera.view_mat);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "fog.mode") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.fog.mode);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "fog.density") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.fog.density);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "fog.start_dist") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.fog.start_dist);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "fog.max_dist") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.fog.max_dist);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "fog.color") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.fog.color);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
					else if(strcmp(uniform->name, "ambient_light") == 0)
					{
						shader_set_uniform(uniform->type, uniform->location, &render_settings.ambient_light);
						renderer_check_glerror("model:render_all:material_pipeline");
					}
				}
		
				for(int j = 0; j < array_len(material->registered_models); j++)
				{
					/* for each registered model, set up uniforms and render */
					struct Entity*    entity    = entity_get(material->registered_models[j]);
					struct Model*     model     = &entity->model;
					struct Transform* transform = &entity->transform;
					struct Geometry*  geometry  = geom_get(model->geometry_index);

					/* Check if model is in frustum */
					int intersection = bv_intersect_frustum_sphere(viewer->camera.frustum, &geometry->bounding_sphere, entity);
					if(intersection == IT_OUTSIDE)
					{
						num_culled++;
						continue;
					}
					else
					{
						num_indices += array_len(geometry->indices);
						num_rendered++;
					}

			
					/* set material params for the model */
					for(int k = 0; k < array_len(model->material_params); k++)
					{
						struct Material_Param* param = &model->material_params[k];
						struct Uniform* uniform = &material->model_params[param->uniform_index];
						shader_set_uniform(uniform->type, uniform->location, param->value);
						renderer_check_glerror("model:render_all:material_param");
					}

					/* Set pipeline uniforms that are derived per model */
					for(int k = 0; k < array_len(material->pipeline_params); k++)
					{
						/* TODO: change this into something better */
						struct Uniform* uniform = &material->pipeline_params[k];
						if(strcmp(uniform->name, "mvp") == 0)
						{
							mat4_identity(&mvp);
							mat4_mul(&mvp, &viewer->camera.view_proj_mat, &transform->trans_mat);
							shader_set_uniform(uniform->type, uniform->location, &mvp);
							renderer_check_glerror("model:render_all:material_pipeline");
						}
						else if(strcmp(uniform->name, "model_mat") == 0)
						{
							shader_set_uniform(uniform->type, uniform->location, &transform->trans_mat);
							renderer_check_glerror("model:render_all:material_pipeline");
						}
						else if(strcmp(uniform->name, "inv_model_mat") == 0)
						{
							mat4 inv_mat;
							mat4_identity(&inv_mat);
							mat4_inverse(&inv_mat, &transform->trans_mat);
							shader_set_uniform(uniform->type, uniform->location, &inv_mat);
							renderer_check_glerror("model:render_all:material_pipeline");
						}
					}
			
					/* Render the geometry */
					//int indices = geom_render_in_frustum(model->geometry_index, &viewer->camera.frustum[0], entity, draw_mode);
					//geom_render(model->geometry_index, draw_mode);
					geom_render(model->geometry_index, GDM_TRIANGLES);

					for(int k = 0; k < array_len(model->material_params); k++)
					{
						/* unbind textures, if any */
						struct Material_Param* param = &model->material_params[k];
						struct Uniform* uniform = &material->model_params[param->uniform_index];
						if(uniform->type == UT_TEX)
						{
							texture_unbind(*(int*)param->value);
							renderer_check_glerror("model:render_all:unbind_texture_uniform");
						}
					}
				}
				shader_unbind();
			}
			editor_debugvar_slot_set_int(num_rendered_slot, num_rendered);
			editor_debugvar_slot_set_int(num_culled_slot, num_culled);
			editor_debugvar_slot_set_int(num_indices_slot, num_indices);

			num_culled = num_rendered = num_indices = 0;
		}
		framebuffer_unbind();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
	}

	/* Final Render */
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

	/* Debug Render */
	struct Hashmap* cvars = config_vars_get();
	if(hashmap_bool_get(cvars, "debug_draw_enabled"))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		vec4 debug_draw_color = hashmap_vec4_get(cvars, "debug_draw_color");
		shader_bind(debug_shader);
		{
			static mat4 mvp;
			shader_set_uniform_vec4(debug_shader, "debug_color", &debug_draw_color);
			struct Entity* entity_list = entity_get_all();
			for(int i = 0; i < array_len(entity_list); i++)
			{
				if(!entity_list[i].renderable) continue;
				struct Model*     model     = &entity_list[i].model;
				struct Transform* transform = &entity_list[i].transform;
				int               geometry  = model->geometry_index;
				mat4_identity(&mvp);
				mat4_mul(&mvp, &active_viewer->camera.view_proj_mat, &transform->trans_mat);
				shader_set_uniform_mat4(debug_shader, "mvp", &mvp);
				geom_render(geometry, hashmap_int_get(cvars, "debug_draw_mode"));
			}
		}
		shader_unbind();
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
	hashmap_vec3_set(cvars,  "fog_color",          &settings->fog.color);
	hashmap_bool_set(cvars,  "debug_draw_enabled", settings->debug_draw_enabled);
	hashmap_int_set(cvars,   "debug_draw_mode",    settings->debug_draw_mode);
	hashmap_vec4_set(cvars,  "debug_draw_color",   &settings->debug_draw_color);
	hashmap_vec3_set(cvars,  "ambient_light",      &settings->ambient_light);
}
