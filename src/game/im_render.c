#include "im_render.h"
#include "entity.h"
#include "camera.h"
#include "gl_load.h"
#include "../common/array.h"
#include "../common/num_types.h"
#include "shader.h"
#include "../common/log.h"
#include "geometry.h"

#include <string.h>
#include <stdlib.h>

#define MAX_IM_VERTICES 2048
#define MAX_IM_GEOMETRIES (MAX_IM_VERTICES / 2)

static struct
{
	struct IM_Vertex vertices[MAX_IM_VERTICES];
	struct IM_Geom   geometries[MAX_IM_GEOMETRIES];
	uint             vao;
	uint             vbo;
	int              im_shader;
	int              curr_geom;
	int              curr_vertex;
}
IM_State;

static struct IM_Geom* active_geom = NULL;
static int active_vertex_index    = 0;

static void im_geom_reset(struct IM_Geom* geom);
static int  im_sort_func(const void* p1, const void* p2);

void im_init(void)
{
	glGenVertexArrays(1, &IM_State.vao);
	glBindVertexArray(IM_State.vao);

	glGenBuffers(1, &IM_State.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, IM_State.vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(struct IM_Vertex) * MAX_IM_VERTICES * MAX_IM_GEOMETRIES, NULL, GL_STREAM_DRAW));

	//Position
	GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct IM_Vertex), 0));
	GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_POSITION));

	////Color
	//GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(struct IM_Vertex), sizeof(vec3)));
	//GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_COLOR));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	memset(&IM_State.geometries[0], 0, sizeof(struct IM_Geom) * MAX_IM_GEOMETRIES);
	memset(&IM_State.vertices[0], 0, sizeof(struct IM_Vertex) * MAX_IM_VERTICES);
	IM_State.curr_geom   = -1;
	IM_State.curr_vertex =  0;

	IM_State.im_shader = shader_create("im_geom.vert", "im_geom.frag");
}

void im_cleanup(void)
{
	shader_remove(IM_State.im_shader);
	glDeleteBuffers(1, &IM_State.vbo);
	glDeleteVertexArrays(1, &IM_State.vao);

	memset(&IM_State.geometries[0], 0, sizeof(struct IM_Geom) * MAX_IM_GEOMETRIES);
	memset(&IM_State.vertices[0], 0, sizeof(struct IM_Vertex) * MAX_IM_VERTICES);
	IM_State.vao         =  0;
	IM_State.vbo         =  0;
	IM_State.curr_geom   = -1;
	IM_State.curr_vertex =  0;
	IM_State.im_shader   = -1;
}

void im_begin(vec3 position, quat rotation, vec3 scale, vec4 color, int draw_mode, int draw_order)
{
	if(active_geom)
	{
		log_error("im_begin", "im_begin called before im_end");
		return;
	}
	IM_State.curr_geom++;
	active_geom = &IM_State.geometries[IM_State.curr_geom];
	im_geom_reset(active_geom);
	active_geom->start_index = IM_State.curr_vertex;
	active_geom->type = IGT_DYNAMIC;
	active_geom->draw_mode = draw_mode;
	active_geom->draw_order = draw_order;
	vec3_assign(&active_geom->position, &position);
	vec3_assign(&active_geom->scale, &scale);
	vec4_assign(&active_geom->color, &color);
	quat_assign(&active_geom->rotation, &rotation);
}

void im_pos(float x, float y, float z)
{
	if(IM_State.curr_vertex == MAX_IM_VERTICES)
	{
		log_error("im_pos", "Buffer full!");
		return;
	}
	vec3_fill(&IM_State.vertices[active_vertex_index].position, x, y, z);
	IM_State.curr_vertex++;
	active_vertex_index++;
}

void im_box(float x, float y, float z, vec3 position, quat rotation, vec4 color, int draw_mode, int draw_order)
{
	if(active_geom)
	{
		log_error("im_box", "im_box called before im_end");
		return;
	}
	IM_State.curr_geom++;
	active_geom = &IM_State.geometries[IM_State.curr_geom];
	im_geom_reset(active_geom);
	active_geom->type = IGT_PRIMITIVE;
	active_geom->draw_mode = draw_mode;
	active_geom->draw_order = draw_order;
	active_geom->prim_geom_index = geom_create_from_file("cube.symbres");
	vec3_assign(&active_geom->position, &position);
	vec3 scale =  { x, y, z}; 
	vec3_assign(&active_geom->scale, &scale);
	vec4_assign(&active_geom->color, &color);
	quat_assign(&active_geom->rotation, &rotation);
	active_geom = NULL;
}

void im_sphere(float radius, vec3 position, quat rotation, vec4 color, int draw_mode, int draw_order)
{
	if(active_geom)
	{
		log_error("im_sphere", "im_sphere called before im_end");
		return;
	}
	IM_State.curr_geom++;
	active_geom = &IM_State.geometries[IM_State.curr_geom];
	im_geom_reset(active_geom);
	active_geom->type = IGT_PRIMITIVE;
	active_geom->draw_mode = draw_mode;
	active_geom->draw_order = draw_order;
	active_geom->prim_geom_index = geom_create_from_file("sphere.symbres");
	vec3_assign(&active_geom->position, &position);
	vec3 scale =  { radius, radius, radius }; 
	vec3_assign(&active_geom->scale, &scale);
	quat_assign(&active_geom->rotation, &rotation);
	vec4_assign(&active_geom->color, &color);
	active_geom = NULL;
}

void im_end(void)
{
	active_geom->num_vertices = active_vertex_index + 1;
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, IM_State.vbo));
	GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER,
					sizeof(struct IM_Vertex) * active_geom->start_index,
					sizeof(struct IM_Vertex) * active_geom->num_vertices,
					&IM_State.vertices[0]));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	active_geom = NULL;
	active_vertex_index = 0;
	memset(&IM_State.vertices[0], 0, sizeof(struct IM_Vertex) * MAX_IM_VERTICES);
}

void im_render(struct Camera* active_viewer)
{
	if(IM_State.curr_geom == -1)
		return;

	/* Sort by draw order, geometries with lower draw order get drawn first */
	if(IM_State.curr_geom + 1 > 1)
		qsort(IM_State.geometries, IM_State.curr_geom + 1, sizeof(struct IM_Geom), &im_sort_func);
	
	shader_bind(IM_State.im_shader);
	{
		static mat4 mvp, translation, rotation, scale;

		for(int i = 0; i <= IM_State.curr_geom; i++)
		{
			struct IM_Geom* geom = &IM_State.geometries[i];
			mat4_identity(&mvp);
			
			mat4_identity(&scale);
			mat4_identity(&translation);
			mat4_identity(&rotation);

			mat4_scale(&scale, geom->scale.x, geom->scale.y, geom->scale.z);
			mat4_translate(&translation, geom->position.x, geom->position.y, geom->position.z);
			mat4_from_quat(&rotation, &geom->rotation);

			mat4_mul(&mvp, &mvp, &translation);
			mat4_mul(&mvp, &mvp, &rotation);
			mat4_mul(&mvp, &mvp, &scale);

			mat4_mul(&mvp, &active_viewer->view_proj_mat, &mvp);

			shader_set_uniform_mat4(IM_State.im_shader, "mvp", &mvp);
			shader_set_uniform_vec4(IM_State.im_shader, "geom_color", &geom->color);
			if(geom->type == IGT_DYNAMIC)
			{
				GL_CHECK(glBindVertexArray(IM_State.vao));
				GL_CHECK(glDrawArrays(draw_modes[geom->draw_mode], geom->start_index, geom->num_vertices));
				GL_CHECK(glBindVertexArray(0));
			}
			else
			{
				GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
				geom_render(geom->prim_geom_index, geom->draw_mode);
				GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			}
		}

	}
	shader_unbind();

	IM_State.curr_geom   = -1;
	IM_State.curr_vertex =  0;
}

void im_geom_reset(struct IM_Geom* geom)
{
	vec3_fill(&geom->position, 0.f, 0.f, 0.f);
	quat_identity(&geom->rotation);
	vec3_fill(&geom->scale, 1.f, 1.f, 1.f);
	vec4_fill(&geom->color, 0.f, 0.f, 0.f, 0.f);
	geom->type = -1;
	geom->start_index = -1;
	geom->num_vertices = 0;
	geom->prim_geom_index = -1;
	geom->draw_mode = -1;
	geom->draw_order = -1;
}

int im_sort_func(const void* p1, const void* p2)
{
	struct IM_Geom* g1 = (struct IM_Geom*)p1;
	struct IM_Geom* g2 = (struct IM_Geom*)p2;
	if(g1->draw_order < g2->draw_order)
		return -1;
	else if(g1->draw_order == g2->draw_order)
		return 0;
	else
		return 1;
}

