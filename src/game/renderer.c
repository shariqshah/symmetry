#include "renderer.h"
#include "gl_load.h"

#include "../common/log.h"
#include "camera.h"
#include "model.h"
#include "texture.h"
#include "framebuffer.h"
#include "../common/array.h"
#include "shader.h"
#include "../common/num_types.h"
#include "light.h"
#include "entity.h"
#include "transform.h"
#include "game.h"
#include "gui.h"
#include "../common/hashmap.h"
#include "geometry.h"
#include "material.h"
#include "editor.h"
#include "sprite.h"
#include "im_render.h"
#include "../common/variant.h"
#include "../system/platform.h"
#include "../system/config_vars.h"
#include "scene.h"
#include "event.h"
#include "debug_vars.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static void renderer_on_framebuffer_size_changed(const struct Event* event);

void renderer_init(struct Renderer* renderer)
{
    assert(renderer);

    glClearColor(0.3f, 0.6f, 0.9f, 1.0f);
	glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    struct Game_State* game_state = game_state_get();
	event_manager_subscribe(game_state->event_manager, EVT_WINDOW_RESIZED, &renderer_on_framebuffer_size_changed);

    struct Hashmap* cvars = game_state->cvars;
    renderer->settings.fog.mode           = hashmap_int_get(cvars,   "fog_mode");
    renderer->settings.fog.density        = hashmap_float_get(cvars, "fog_density");
    renderer->settings.fog.start_dist     = hashmap_float_get(cvars, "fog_start_dist");
    renderer->settings.fog.max_dist       = hashmap_float_get(cvars, "fog_max_dist");
    renderer->settings.fog.color          = hashmap_vec3_get(cvars,  "fog_color");
    renderer->settings.debug_draw_enabled = hashmap_bool_get(cvars,  "debug_draw_enabled");
    renderer->settings.debug_draw_physics = hashmap_bool_get(cvars,  "debug_draw_physics");
    renderer->settings.debug_draw_mode    = hashmap_int_get(cvars,   "debug_draw_mode");
    renderer->settings.debug_draw_color   = hashmap_vec4_get(cvars,  "debug_draw_color");
    renderer->settings.ambient_light      = hashmap_vec3_get(cvars,  "ambient_light");
	
    renderer->debug_shader = shader_create("debug.vert", "debug.frag", NULL);
    renderer->sprite_batch = malloc(sizeof(*renderer->sprite_batch));

    if(!renderer->sprite_batch)
		log_error("renderer:init", "Failed to allocated sprite batch");
    else
		sprite_batch_create(renderer->sprite_batch, "sprite_map.tga", "sprite.vert", "sprite.frag", GL_TRIANGLES);

    im_init();

    // Initialize materials
    for(int i = 0; i < MAT_MAX; i++)
		material_init(&renderer->materials[i], i);
}

void renderer_render(struct Renderer* renderer, struct Scene* scene)
{
	struct Game_State* game_state = game_state_get();
	struct Camera* active_camera = &scene->cameras[scene->active_camera_index];
	int num_rendered = 0, num_culled = 0, num_indices = 0;

	int width = 0, height = 0;
	window_get_drawable_size(game_state->window, &width, &height);
	glViewport(0, 0, width, height);
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(active_camera->clear_color.x,
				 active_camera->clear_color.y,
				 active_camera->clear_color.z,
				 active_camera->clear_color.w);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	static mat4 mvp;
	for(int i = 0; i < MAT_MAX; i++)
	{
		/* for each material, get all the registered models and render them */
		struct Material* material = &renderer->materials[i];
		GL_CHECK(shader_bind(material->shader));

		if(material->lit)	/* Set light information */
		{
			char uniform_name[MAX_UNIFORM_NAME_LEN];
			memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
			int light_count = -1;
			for(int j = 0; j < MAX_SCENE_LIGHTS; j++)
			{
				struct Light* light = &scene->lights[j]; /* TODO: Cull lights according to camera frustum */
				if(!(light->base.flags & EF_ACTIVE) || !light->valid) continue;
				light_count++;

				vec3 light_pos = { 0, 0, 0 };
				transform_get_absolute_position(&light->base, &light_pos);

				if(light->type != LT_POINT)
				{
					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].direction", light_count);
					vec3 light_dir = { 0.f, 0.f, 0.f };
					transform_get_absolute_forward(&light->base, &light_dir);
					vec3_norm(&light_dir, &light_dir);
					shader_set_uniform_vec3(material->shader, uniform_name, &light_dir);
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
				}

				if(light->type != LT_DIR)
				{
					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].position", light_count);
					shader_set_uniform_vec3(material->shader, uniform_name, &light_pos);
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].outer_angle", light_count);
					shader_set_uniform_float(material->shader, uniform_name, TO_RADIANS(light->outer_angle));
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].inner_angle", light_count);
					shader_set_uniform_float(material->shader, uniform_name, TO_RADIANS(light->inner_angle));
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].falloff", light_count);
					shader_set_uniform_float(material->shader, uniform_name, light->falloff);
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

					snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].radius", light_count);
					shader_set_uniform_int(material->shader, uniform_name, light->radius);
					memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
				}

				snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].color", light_count);
				shader_set_uniform_vec3(material->shader, uniform_name, &light->color);
				memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

				snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].intensity", light_count);
				shader_set_uniform_float(material->shader, uniform_name, light->intensity);
				memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);

				snprintf(uniform_name, MAX_UNIFORM_NAME_LEN, "lights[%d].type", light_count);
				shader_set_uniform_int(material->shader, uniform_name, light->type);
				memset(uniform_name, '\0', MAX_UNIFORM_NAME_LEN);
			}

			light_count++; // this variable is going to be sent as a uniform and be used for looping an array so increase its length by one
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_TOTAL_LIGHTS].type, material->pipeline_params[MPP_TOTAL_LIGHTS].location, &light_count));

			vec3 camera_pos = { 0, 0, 0 };
			transform_get_absolute_position(&active_camera->base, &camera_pos);
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_CAM_POS].type, material->pipeline_params[MPP_CAM_POS].location, &camera_pos));
		}

		/* Set material pipeline uniforms */
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_MODE].type, material->pipeline_params[MPP_FOG_MODE].location, &renderer->settings.fog.mode));
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_DENSITY].type, material->pipeline_params[MPP_FOG_DENSITY].location, &renderer->settings.fog.density));
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_START_DIST].type, material->pipeline_params[MPP_FOG_START_DIST].location, &renderer->settings.fog.start_dist));
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_MAX_DIST].type, material->pipeline_params[MPP_FOG_MAX_DIST].location, &renderer->settings.fog.max_dist));
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_COLOR].type, material->pipeline_params[MPP_FOG_COLOR].location, &renderer->settings.fog.color));
		GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_AMBIENT_LIGHT].type, material->pipeline_params[MPP_AMBIENT_LIGHT].location, &renderer->settings.ambient_light));
		if(material->lit) GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_VIEW_MAT].type, material->pipeline_params[MPP_VIEW_MAT].location, &active_camera->view_mat));


		for(int j = 0; j < MAX_MATERIAL_REGISTERED_STATIC_MESHES; j++)
		{
			if(!material->registered_static_meshes[j] || (material->registered_static_meshes[j]->base.flags & EF_SKIP_RENDER)) continue;

			/* for each registered model, set up uniforms and render */
			struct Static_Mesh* mesh = material->registered_static_meshes[j];
			struct Geometry* geometry = geom_get(mesh->model.geometry_index);

			/* Check if model is in frustum */

			int intersection = bv_intersect_frustum_box(&active_camera->frustum, &mesh->base.derived_bounding_box);
			if(intersection == IT_INSIDE || intersection == IT_INTERSECT)
			{
				num_indices += geometry->indices_length;
				num_rendered++;
			}
			else
			{
				num_culled++;
				continue;
			}

			/* set material params for the model */
			for(int k = 0; k < MMP_MAX; k++)
			{
				switch(mesh->model.material_params[k].type)
				{
				case VT_INT:   GL_CHECK(shader_set_uniform(material->model_params[k].type, material->model_params[k].location, &mesh->model.material_params[k].val_int));   break;
				case VT_FLOAT: GL_CHECK(shader_set_uniform(material->model_params[k].type, material->model_params[k].location, &mesh->model.material_params[k].val_float)); break;
				case VT_VEC2:  GL_CHECK(shader_set_uniform(material->model_params[k].type, material->model_params[k].location, &mesh->model.material_params[k].val_vec2));  break;
				case VT_VEC3:  GL_CHECK(shader_set_uniform(material->model_params[k].type, material->model_params[k].location, &mesh->model.material_params[k].val_vec3));  break;
				case VT_VEC4:  GL_CHECK(shader_set_uniform(material->model_params[k].type, material->model_params[k].location, &mesh->model.material_params[k].val_vec4));  break;
				}
			}

			/* Set pipeline uniforms that are derived per model */
			mat4_identity(&mvp);
			mat4_mul(&mvp, &active_camera->view_proj_mat, &mesh->base.transform.trans_mat);
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_MVP].type, material->pipeline_params[MPP_MVP].location, &mvp));

			if(material->lit)
			{
				GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_VIEW_MAT].type, material->pipeline_params[MPP_VIEW_MAT].location, &active_camera->view_mat));
				GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_MODEL_MAT].type, material->pipeline_params[MPP_MODEL_MAT].location, &mesh->base.transform.trans_mat));
				mat4 inv_mat;
				mat4_identity(&inv_mat);
				mat4_inverse(&inv_mat, &mesh->base.transform.trans_mat);
				GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_INV_MODEL_MAT].type, material->pipeline_params[MPP_INV_MODEL_MAT].location, &inv_mat));
			}

			/* Render the geometry */
			geom_render(mesh->model.geometry_index, GDM_TRIANGLES);

			for(int k = 0; k < MMP_MAX; k++)
			{
				/* unbind textures, if any */
				if(material->model_params[k].type == UT_TEX)
					GL_CHECK(texture_unbind(mesh->model.material_params[k].val_int));
			}
		}
		shader_unbind();
	}

	debug_vars_show_int("Rendered", num_rendered);
	debug_vars_show_int("Culled", num_culled);
	debug_vars_show_int("Num Indices", num_indices);

    /* Debug Render */
	if(renderer->settings.debug_draw_enabled)
    {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader_bind(renderer->debug_shader);
		{
			static mat4 mvp;
			shader_set_uniform_vec4(renderer->debug_shader, "debug_color", &renderer->settings.debug_draw_color);
			for(int i = 0; i < MAX_SCENE_STATIC_MESHES; i++)
			{
				struct Static_Mesh* mesh = &scene->static_meshes[i];
				if(!(mesh->base.flags & EF_ACTIVE)) continue;
				struct Model*     model     = &mesh->model;
				struct Transform* transform = &mesh->base.transform;
				int               geometry  = model->geometry_index;
				mat4_identity(&mvp);
				mat4_mul(&mvp, &active_camera->view_proj_mat, &transform->trans_mat);
				shader_set_uniform_mat4(renderer->debug_shader, "mvp", &mvp);
				geom_render(geometry, renderer->settings.debug_draw_mode);
			}
		}
		shader_unbind();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

	//Editor related rendering
	if(game_state->game_mode == GAME_MODE_EDITOR)
	{
		editor_render(game_state->editor, active_camera);
	}

    //Immediate mode geometry render
    im_render(active_camera);

    /* Render 2D stuff */
    shader_bind(renderer->sprite_batch->shader);
    {
		static mat4 ortho_mat;
		mat4_identity(&ortho_mat);
		int width, height;
		struct Game_State* game_state = game_state_get();
		window_get_size(game_state->window, &width, &height);

		mat4_ortho(&ortho_mat, 0.f, (float)width, (float)height, 0.f, -10.f, 10.f);
		shader_set_uniform_mat4(renderer->sprite_batch->shader, "mvp", &ortho_mat);

		sprite_batch_render(renderer->sprite_batch);
    }
    shader_unbind();

    /* Render UI */
    gui_render(game_state->gui_editor, NK_ANTI_ALIASING_ON);
    gui_render(game_state->gui_game, NK_ANTI_ALIASING_ON);
}

void renderer_cleanup(struct Renderer* renderer)
{
    for(int i = 0; i < MAT_MAX; i++)
    {
		material_reset(&renderer->materials[i]);
    }
    im_cleanup();
    sprite_batch_remove(renderer->sprite_batch);
    free(renderer->sprite_batch);
}

void renderer_on_framebuffer_size_changed(const struct Event* event)
{
	int width  = event->window_resize.width;
	int height = event->window_resize.height;

    struct Scene* scene = game_state_get()->scene;
    float aspect = (float)width / (float)height;
    for(int i = 0; i < MAX_SCENE_CAMERAS; i++)
    {
		struct Camera* viewer = &scene->cameras[i];
		if(viewer->resizeable)
		{
			viewer->aspect_ratio = aspect > 0.f ? aspect : 4.f / 3.f;
			camera_update_proj(viewer);
		}
    }
	
    framebuffer_resize_all(width, height);
}

void renderer_clearcolor_set(float red, float green, float blue, float alpha)
{
    glClearColor(red, green, blue, alpha);
}

struct Material* renderer_material_get(int material_type)
{
    return NULL;
}

void renderer_debug_draw_enabled(struct Renderer* renderer, bool enabled)
{
    renderer->settings.debug_draw_mode = enabled;
}
