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
	
    renderer->quad_geo = geom_create("Quad", vertices, uvs, normals, indices, NULL);
    array_free(vertices);
    array_free(uvs);
    array_free(normals);
    array_free(indices);

    int width = -1, height = -1;
    window_get_size(game_state->window, &width, &height);
    renderer->def_albedo_tex = texture_create("def_albedo_texture",
											  TU_DIFFUSE,
											  width, height,
											  GL_RGB,
											  GL_RGB16F,
											  GL_FLOAT,
											  NULL);
    texture_set_param(renderer->def_albedo_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture_set_param(renderer->def_albedo_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture_set_param(renderer->def_albedo_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_set_param(renderer->def_albedo_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    renderer->def_depth_tex = texture_create("def_depth_texture",
											 TU_SHADOWMAP4,
											 width, height,
											 GL_DEPTH_COMPONENT,
											 GL_DEPTH_COMPONENT32F,
											 GL_FLOAT,
											 NULL);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    texture_set_param(renderer->def_depth_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    renderer->def_fbo = framebuffer_create(width, height, true, false, true);
    framebuffer_texture_set(renderer->def_fbo, renderer->def_albedo_tex, FA_COLOR_ATTACHMENT0);
    framebuffer_texture_set(renderer->def_fbo, renderer->def_depth_tex, FA_DEPTH_ATTACHMENT);
    renderer->composition_shader = shader_create("fbo.vert", "fbo.frag", NULL);
    renderer->debug_shader       = shader_create("debug.vert", "debug.frag", NULL);

    // renderer->num_culled_slot   = editor_debugvar_slot_create("Culled Geom",   VT_INT);
    // renderer->num_rendered_slot = editor_debugvar_slot_create("Rendered Geom", VT_INT);
    // renderer->num_indices_slot  = editor_debugvar_slot_create("Total Indices", VT_INT);

    renderer->sprite_batch = malloc(sizeof(*renderer->sprite_batch));
    if(!renderer->sprite_batch)
    {
		log_error("renderer:init", "Failed to allocated sprite batch");
    }
    else
    {
		sprite_batch_create(renderer->sprite_batch, "sprite_map.tga", "sprite.vert", "sprite.frag", GL_TRIANGLES);
    }

    im_init();

    // Initialize materials
    for(int i = 0; i < MAT_MAX; i++)
    {
		material_init(&renderer->materials[i], i);
    }
}

void renderer_render(struct Renderer* renderer, struct Scene* scene)
{
	struct Camera* camera = &scene->cameras[scene->active_camera_index];
	int fbo = camera->fbo == -1 ? renderer->def_fbo : camera->fbo;
	framebuffer_bind(fbo);
	{
		glViewport(0, 0, framebuffer_width_get(fbo), framebuffer_height_get(fbo));
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearColor(camera->clear_color.x,
			camera->clear_color.y,
			camera->clear_color.z,
			camera->clear_color.w);
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
				for(int j = 0; j < MAX_LIGHTS; j++)
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
				transform_get_absolute_position(&camera->base, &camera_pos);
				GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_CAM_POS].type, material->pipeline_params[MPP_CAM_POS].location, &camera_pos));
			}

			/* Set material pipeline uniforms */
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_MODE].type, material->pipeline_params[MPP_FOG_MODE].location, &renderer->settings.fog.mode));
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_DENSITY].type, material->pipeline_params[MPP_FOG_DENSITY].location, &renderer->settings.fog.density));
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_START_DIST].type, material->pipeline_params[MPP_FOG_START_DIST].location, &renderer->settings.fog.start_dist));
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_MAX_DIST].type, material->pipeline_params[MPP_FOG_MAX_DIST].location, &renderer->settings.fog.max_dist));
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_FOG_COLOR].type, material->pipeline_params[MPP_FOG_COLOR].location, &renderer->settings.fog.color));
			GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_AMBIENT_LIGHT].type, material->pipeline_params[MPP_AMBIENT_LIGHT].location, &renderer->settings.ambient_light));
			if(material->lit) GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_VIEW_MAT].type, material->pipeline_params[MPP_VIEW_MAT].location, &camera->view_mat));


			for(int j = 0; j < MAX_MATERIAL_REGISTERED_STATIC_MESHES; j++)
			{
				if(!material->registered_static_meshes[j] || (material->registered_static_meshes[j]->base.flags & EF_SKIP_RENDER)) continue;

				/* for each registered model, set up uniforms and render */
				struct Static_Mesh* mesh = material->registered_static_meshes[j];
				struct Geometry*    geometry = geom_get(mesh->model.geometry_index);

				/* Check if model is in frustum */

				int intersection = bv_intersect_frustum_box(&camera->frustum, &mesh->base.transform.bounding_box);
				if(intersection == IT_INSIDE || intersection == IT_INTERSECT)
				{
					renderer->num_indices += array_len(geometry->indices);
					renderer->num_rendered++;
				}
				else
				{
					renderer->num_culled++;
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
				mat4_mul(&mvp, &camera->view_proj_mat, &mesh->base.transform.trans_mat);
				GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_MVP].type, material->pipeline_params[MPP_MVP].location, &mvp));

				if(material->lit)
				{
					GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_VIEW_MAT].type, material->pipeline_params[MPP_VIEW_MAT].location, &camera->view_mat));
					GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_MODEL_MAT].type, material->pipeline_params[MPP_MODEL_MAT].location, &mesh->base.transform.trans_mat));
					mat4 inv_mat;
					mat4_identity(&inv_mat);
					mat4_inverse(&inv_mat, &mesh->base.transform.trans_mat);
					GL_CHECK(shader_set_uniform(material->pipeline_params[MPP_INV_MODEL_MAT].type, material->pipeline_params[MPP_INV_MODEL_MAT].location, &inv_mat));
				}

				/* Render the geometry */
				//int indices = geom_render_in_frustum(model->geometry_index, &viewer->camera.frustum[0], entity, draw_mode);
				//geom_render(model->geometry_index, draw_mode);
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

		debug_vars_show_int("Rendered", renderer->num_rendered);
		debug_vars_show_int("Culled", renderer->num_culled);
		debug_vars_show_int("Num Indices", renderer->num_indices);
		//editor_debugvar_slot_set_int(renderer->num_rendered_slot, renderer->num_rendered);
		//editor_debugvar_slot_set_int(renderer->num_culled_slot, renderer->num_culled);
		//editor_debugvar_slot_set_int(renderer->num_indices_slot, renderer->num_indices);

		renderer->num_culled = renderer->num_rendered = renderer->num_indices = 0;
	}
	framebuffer_unbind();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

    /* Final Render */
    struct Camera* active_camera = &scene->cameras[scene->active_camera_index];
    int width, height;
    struct Game_State* game_state = game_state_get();
    //window_get_size(game_state->window, &width, &height);
	window_get_drawable_size(game_state->window, &width, &height);
    glViewport(0, 0, width, height);
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    shader_bind(renderer->composition_shader);
    int final_render_tex = active_camera->render_tex == -1 ? renderer->def_albedo_tex : active_camera->render_tex;
    texture_bind(final_render_tex);
    geom_render(renderer->quad_geo, GDM_TRIANGLES);
    texture_unbind(final_render_tex);
    shader_unbind();

    /* Debug Render */
	if(renderer->settings.debug_draw_enabled)
    {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		shader_bind(renderer->debug_shader);
		{
			static mat4 mvp;
			shader_set_uniform_vec4(renderer->debug_shader, "debug_color", &renderer->settings.debug_draw_color);
			for(int i = 0; i < MAX_STATIC_MESHES; i++)
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

    // Debug Physics render
	if(renderer->settings.debug_draw_physics)
    {
		static vec4 physics_draw_color = { 0.f, 0.f, 1.f, 1.f };
		for(int i = 0; i < MAX_STATIC_MESHES; i++)
		{
			struct Static_Mesh* mesh = &scene->static_meshes[i];
			if(!(mesh->base.flags & EF_ACTIVE) || (!mesh->collision.collision_shape && !mesh->collision.rigidbody)) continue;

			//Get collision mesh and it's props then render it
			vec3 pos = {0.f};
			quat rot = {0.f, 0.f, 0.f, 1.f };
			if(mesh->collision.rigidbody)
			{
				physics_body_position_get(mesh->collision.rigidbody, &pos.x, &pos.y, &pos.z);
				physics_body_rotation_get(mesh->collision.rigidbody, &rot.x, &rot.y, &rot.z, &rot.w);
			}
			else
			{
				physics_cs_position_get(mesh->collision.collision_shape, &pos.x, &pos.y, &pos.z);
				physics_cs_rotation_get(mesh->collision.collision_shape, &rot.x, &rot.y, &rot.z, &rot.w);
			}

			int collision_shape_type = physics_cs_type_get(mesh->collision.collision_shape);
			switch(collision_shape_type)
			{
			case CST_SPHERE:
			{
				float radius = physics_cs_sphere_radius_get(mesh->collision.collision_shape);
				im_sphere(radius, pos, rot, physics_draw_color, GDM_TRIANGLES, 1);
			}
			break;
			case CST_BOX:
			{
				float x = 0.f, y = 0.f, z = 0.f;
				physics_cs_box_params_get(mesh->collision.collision_shape, &x, &y, &z);
				im_box(x, y, z, pos, rot, physics_draw_color, GDM_TRIANGLES, 1);
			};
			break;
			default: break;
			}
		}
    }

	//Editor related rendering
	if(game_state->game_mode == GAME_MODE_EDITOR)
	{
		editor_render(game_state->editor, active_camera);
	}

    //Immediate mode geometry render
	// Maybe try binding the old frame buffer, clear the drawing but let the depth buffer remain to check depth?
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
    gui_render(game_state->gui, NK_ANTI_ALIASING_ON);
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
    geom_remove(renderer->quad_geo);
    framebuffer_remove(renderer->def_fbo);
    texture_remove(renderer->def_albedo_tex);
    texture_remove(renderer->def_depth_tex);
}

void renderer_on_framebuffer_size_changed(const struct Event* event)
{
	int width  = event->window_resize.width;
	int height = event->window_resize.height;

    struct Scene* scene = game_state_get()->scene;
    float aspect = (float)width / (float)height;
    for(int i = 0; i < MAX_CAMERAS; i++)
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
