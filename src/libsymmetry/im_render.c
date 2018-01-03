#include "im_render.h"
#include "entity.h"
#include "camera.h"
#include "gl_load.h"
#include "../common/array.h"
#include "../common/num_types.h"
#include "shader.h"
#include "../common/log.h"

#define MAX_IM_VERTICES 4096
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
	vec4             default_color;
}
IM_State;

static struct IM_Geom* active_geom = NULL;
static vec4 active_vertex_color  = { 0.f, 0.f, 0.f, 0.f };

void im_init(void)
{
	glGenVertexArrays(1, &IM_State.vao);
	glBindVertexArray(IM_State.vao);

	glGenBuffers(1, &IM_State.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, IM_State.vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(struct IM_Vertex) * MAX_IM_VERTICES, NULL, GL_STREAM_DRAW));

	//Position
	GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct IM_Vertex), 0));
	GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_POSITION));

	//Color
	GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(struct IM_Vertex), sizeof(vec3)));
	GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_COLOR));

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	memset(&IM_State.geometries[0], 0, sizeof(struct IM_Geom) * MAX_IM_GEOMETRIES);
	memset(&IM_State.vertices[0], 0, sizeof(struct IM_Vertex) * MAX_IM_VERTICES);
	IM_State.curr_geom   = -1;
	IM_State.curr_vertex =  0;
	vec4_fill(&IM_State.default_color, 1.f, 0.f, 1.f, 1.f);
	vec4_assign(&active_vertex_color, &IM_State.default_color);

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

void im_begin(vec3 position, quat rotation, vec3 scale, int draw_mode)
{
	if(active_geom)
	{
		log_error("im_begin", "im_begin called before im_end");
		return;
	}
	IM_State.curr_geom++;
	active_geom = &IM_State.geometries[IM_State.curr_geom];
	active_geom->start_index = IM_State.curr_vertex;
	active_geom->draw_mode = draw_mode;
	vec3_assign(&active_geom->position, &position);
	vec3_assign(&active_geom->scale, &scale);
	quat_assign(&active_geom->rotation, &rotation);
}

void im_pos(float x, float y, float z)
{
	vec3_fill(&IM_State.vertices[IM_State.curr_vertex].position, x, y, z);
	vec4_assign(&IM_State.vertices[IM_State.curr_vertex].color, &active_vertex_color);
	IM_State.curr_vertex++;
}

void im_color(float r, float g, float b, float a)
{
	vec4_fill(&active_vertex_color, r, g, b, a);
}

void im_end(void)
{
	active_geom->num_vertices = IM_State.curr_vertex - active_geom->start_index;
	glBindBuffer(GL_ARRAY_BUFFER, IM_State.vbo);
	glBufferSubData(GL_ARRAY_BUFFER,
					sizeof(struct IM_Vertex) * active_geom->start_index,
					sizeof(struct IM_Vertex) * active_geom->num_vertices,
					&IM_State.vertices[active_geom->start_index]);
	renderer_check_glerror("sprite_batch_end:glBufferSubData");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	active_geom = NULL;
	vec4_assign(&active_vertex_color, &IM_State.default_color);
}

void im_render(struct Entity* active_viewer)
{
	if(IM_State.curr_geom == -1)
		return;

	shader_bind(IM_State.im_shader);
	{
		static mat4 mvp, translation, rotation, scale;

		glBindVertexArray(IM_State.vao);
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

			mat4_mul(&mvp, &active_viewer->camera.view_proj_mat, &mvp);

			shader_set_uniform_mat4(IM_State.im_shader, "mvp", &mvp);

			glDrawArrays(geom->draw_mode, geom->start_index, geom->num_vertices);
		}
		glBindVertexArray(0);

	}
	shader_unbind();

	IM_State.curr_geom   = -1;
	IM_State.curr_vertex =  0;
}
