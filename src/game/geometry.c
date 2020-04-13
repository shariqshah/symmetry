#include "geometry.h"
#include "../common/array.h"
#include "../common/string_utils.h"
#include "../common/memory_utils.h"
#include "../common/log.h"
#include "renderer.h"
#include "transform.h"
#include "../system/file_io.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

GLenum* draw_modes = NULL;
static struct Geometry* geometry_list;
static int*             empty_indices;

static void             create_vao(struct Geometry* geometry, vec3* vertices, vec2* uvs, vec3* normals, vec3* vertex_colors, uint* indices);
static struct Geometry* generate_new_index(int* out_new_index);
static void             geom_bounding_volume_generate(struct Geometry* geometry, vec3* vertices);

void geom_init(void)
{
	geometry_list = array_new(struct Geometry);
	empty_indices = array_new(int);
	draw_modes    = array_new_cap(GLenum, GDM_NUM_DRAWMODES);
	draw_modes[GDM_TRIANGLES]    = GL_TRIANGLES;
	draw_modes[GDM_LINES]        = GL_LINES;
	draw_modes[GDM_POINTS]       = GL_POINTS;
	draw_modes[GDM_LINE_STRIP]   = GL_LINE_STRIP;
	draw_modes[GDM_LINE_LOOP]    = GL_LINE_LOOP;
	draw_modes[GDM_TRIANGLE_FAN] = GL_TRIANGLE_FAN;
}

int geom_find(const char* filename)
{
	int index = -1;
	for(int i = 0; i < array_len(geometry_list); i++)
	{
		struct Geometry* geometry = &geometry_list[i];
		if(strcmp(geometry->filename, filename) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

void geom_bounding_volume_generate(struct Geometry* geometry, vec3* vertices)
{
	struct Bounding_Box*    box    = &geometry->bounding_box;
	struct Bounding_Sphere* sphere = &geometry->bounding_sphere;
	
	vec3_fill(&box->max, -FLT_MIN, -FLT_MIN, -FLT_MIN);
	vec3_fill(&box->min,  FLT_MAX,  FLT_MAX,  FLT_MAX);
	vec3_fill(&sphere->center, 0.f, 0.f, 0.f);
	sphere->radius = 0.f;
	
	for(int i = 0; i < array_len(vertices); i++)
	{
		vec3* vertex = &vertices[i];
		if(vertex->x > box->max.x) box->max.x = vertex->x;
		if(vertex->y > box->max.y) box->max.y = vertex->y;
		if(vertex->z > box->max.z) box->max.z = vertex->z;

		if(vertex->x < box->min.x) box->min.x = vertex->x;
		if(vertex->y < box->min.y) box->min.y = vertex->y;
		if(vertex->z < box->min.z) box->min.z = vertex->z;
	}
	vec3_add(&sphere->center, &box->max, &box->min);
	vec3_scale(&sphere->center, &sphere->center, 0.5f);
	vec3 len_vec;
	vec3 box_vertices[8];
	bv_bounding_box_vertices_get(box, &box_vertices);
	sphere->radius = 0.f;
	for(int i = 0; i < 8; i++)
	{
		float len = fabsf(vec3_distance(box_vertices[i], sphere->center));
		if(len > sphere->radius)
			sphere->radius = len;
	}
}

struct Geometry* generate_new_index(int* out_new_index)
{
	assert(out_new_index);
	int empty_len = array_len(empty_indices);
	struct Geometry* new_geo = NULL;
	if(empty_len > 0)
	{
		*out_new_index = empty_indices[empty_len - 1];
		array_pop(empty_indices);
		new_geo = &geometry_list[*out_new_index];
	}
	else
	{
		new_geo = array_grow(geometry_list, struct Geometry);
		*out_new_index = array_len(geometry_list) - 1;
	}
	return new_geo;
}

int geom_create_from_file(const char* name)
{
	assert(name);
	// check if exists
	int index = geom_find(name);
	if(index == -1)
	{
		/* add new geometry object or overwrite existing one */
		struct Geometry* new_geo = NULL;
		new_geo = generate_new_index(&index);
		assert(new_geo);
		
		char* full_path = str_new("models/%s", name);
		FILE* file = io_file_open(DIRT_INSTALL, full_path, "rb");
		memory_free(full_path);
		if(file)
		{
			const uint32 INDEX_SIZE = sizeof(uint32);
			const uint32 VEC3_SIZE = sizeof(vec3);
			const uint32 VEC2_SIZE = sizeof(vec2);
			uint32 header[4];
			size_t bytes_read = 0;
			if((bytes_read = fread(header, INDEX_SIZE, 4, file)) <= 0)
			{
				log_error("geometry:load_from_file", "Read failed");
				/* TODO: Some error here, find it and fix it */
				array_pop(geometry_list);
				index = -1;
			}
			else
			{
				uint32 indices_count  = header[0];
				uint32 vertices_count = header[1];
				uint32 normals_count  = header[2];
				uint32 uvs_count      = header[3];
				// Indices
				uint* indices = array_new_cap(uint, indices_count);
				fread(indices, INDEX_SIZE, indices_count, file);
				array_match_len_cap(indices);
				// Vertices
				vec3* vertices = array_new_cap(vec3, vertices_count);
				fread(vertices, VEC3_SIZE, vertices_count, file);
				array_match_len_cap(vertices);
				// Normals
				vec3* normals = array_new_cap(vec3, normals_count);
				fread(normals, VEC3_SIZE, normals_count, file);
				array_match_len_cap(normals);
				// Uvs
				vec2* uvs = array_new_cap(vec2, uvs_count);
				fread(uvs, VEC2_SIZE, uvs_count, file);
				array_match_len_cap(uvs);

				new_geo->filename = str_new(name);
				new_geo->draw_indexed = 1;
				new_geo->ref_count++;

				create_vao(new_geo, vertices, uvs, normals, NULL, indices);
				geom_bounding_volume_generate(new_geo, vertices);
				if(vertices)      array_free(vertices);
				if(indices)       array_free(indices);
				if(uvs)           array_free(uvs);
				if(normals)       array_free(normals);
			}
			fclose(file);
		}
		else
		{
			/* TODO: Some error here, find it and fix it */
			array_pop(geometry_list);
			index = -1;
		}
	}
	else
	{
		geometry_list[index].ref_count++;
	}
	return index;
}

int geom_create(const char* name,
				vec3*       vertices,
				vec2*       uvs,
				vec3*       normals,
				uint*       indices,
				vec3*       vertex_colors)
{
	assert(name && vertices && uvs && normals && indices);
	int index = -1;
	/* add new geometry object or overwrite existing one */
	struct Geometry* new_geometry = NULL;
	new_geometry = generate_new_index(&index);
	assert(new_geometry);
	new_geometry->filename = str_new(name);
	create_vao(new_geometry, vertices, uvs, normals, vertex_colors, indices);
	geom_bounding_volume_generate(new_geometry, vertices);
	return index;
}


void geom_remove(int index)
{
	if(index >= 0 && index < array_len(geometry_list))
	{
		struct Geometry* geometry = &geometry_list[index];
		if(geometry->ref_count >= 0)
		{
			geometry->ref_count--;
			if(geometry->ref_count < 0)
			{
				if(geometry->filename) memory_free(geometry->filename);
				geometry->filename = NULL;

				glDeleteBuffers(1, &geometry->vertex_vbo);
				glDeleteBuffers(1, &geometry->color_vbo);
				glDeleteBuffers(1, &geometry->uv_vbo);
				glDeleteBuffers(1, &geometry->normal_vbo);
				glDeleteBuffers(1, &geometry->index_vbo);
				glDeleteVertexArrays(1, &geometry->vao);

				geometry->vertex_vbo      = 0;
				geometry->color_vbo	      = 0;
				geometry->uv_vbo	      = 0;
				geometry->normal_vbo      = 0;
				geometry->index_vbo       = 0;
				geometry->vao             = 0;
				geometry->indices_length  = 0;
				geometry->vertices_length = 0;

				array_push(empty_indices, index, int);
			}
		}
	}
}

void geom_cleanup(void)
{
	for(int i = 0; i < array_len(geometry_list); i++)
		geom_remove(i);
	
	array_free(geometry_list);
	array_free(empty_indices);
	array_free(draw_modes);
}

void create_vao(struct Geometry* geometry,
				vec3*            vertices,
				vec2*            uvs,
				vec3*            normals,
				vec3*            vertex_colors,
				uint*            indices)
{
	// TODO: Add support for different model formats and interleaving VBO
	assert(geometry);
	glGenVertexArrays(1, &geometry->vao);
	glBindVertexArray(geometry->vao);

	glGenBuffers(1, &geometry->vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertex_vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
				 array_len(vertices) * sizeof(vec3),
				 vertices,
				 GL_STATIC_DRAW));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	geometry->vertices_length = array_len(vertices);

	if(array_len(normals) > 0)
	{
		glGenBuffers(1, &geometry->normal_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->normal_vbo);
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
					 array_len(normals) * sizeof(vec3),
					 normals,
					 GL_STATIC_DRAW));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
	}

	if(array_len(uvs) > 0)
	{
		glGenBuffers(1, &geometry->uv_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->uv_vbo);
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
					 array_len(uvs) * sizeof(vec2),
					 uvs,
					 GL_STATIC_DRAW));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(vertex_colors && array_len(vertex_colors) > 0)
	{
		glGenBuffers(1, &geometry->color_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->color_vbo);
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
					 array_len(vertex_colors) * sizeof(vec3),
					 vertex_colors,
					 GL_STATIC_DRAW));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(array_len(indices) > 0)
	{
		glGenBuffers(1, &geometry->index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 array_len(indices) * sizeof(GLuint),
					 indices,
					 GL_STATIC_DRAW);
		geometry->draw_indexed = 1;
		geometry->indices_length = array_len(indices);
	}
	glBindVertexArray(0);

}

void geom_render(int index, enum Geometry_Draw_Mode draw_mode)
{
	assert((int)draw_mode > -1 && draw_mode < GDM_NUM_DRAWMODES && index >= 0);
	struct Geometry* geo = &geometry_list[index];
	glBindVertexArray(geo->vao);
	if(geo->draw_indexed)
		glDrawElements(draw_modes[draw_mode], geo->indices_length, GL_UNSIGNED_INT, (void*)0);
	else
		glDrawArrays(draw_modes[draw_mode], 0, geo->vertices_length);
	glBindVertexArray(0);
			
}

int geom_render_in_frustum(int                      index,
							vec4*                   frustum,
							struct Entity*          entity,
							enum Geometry_Draw_Mode draw_mode)
{
	vec3 abs_pos, abs_scale;
	transform_get_absolute_position(entity, &abs_pos);
	transform_get_absolute_scale(entity, &abs_scale);

	struct Geometry* geometry         = &geometry_list[index];
	int              indices_rendered = 0;
	int              intersection     = bv_intersect_frustum_sphere(frustum, &geometry->bounding_sphere, &abs_pos, &abs_scale);
	if(intersection == IT_INTERSECT || intersection == IT_INSIDE)
	{
		//geom_render(index, draw_mode);
		//indices_rendered = array_len(geometry->indices);
		intersection = bv_intersect_frustum_box_with_abs_transform(frustum, &geometry->bounding_box, &abs_pos, &abs_scale);
		if(intersection == IT_INTERSECT || intersection == IT_INSIDE)
		{ 
			geom_render(index, draw_mode);
			indices_rendered = array_len(geometry->indices_length);
		}
	}
	return indices_rendered;
}

struct Geometry* geom_get(int index)
{
	assert(index > -1 && index < array_len(geometry_list));
	return &geometry_list[index];
}
