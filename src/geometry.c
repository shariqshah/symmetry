#include "geometry.h"
#include "array.h"
#include "string_utils.h"
#include "file_io.h"
#include "log.h"
#include "renderer.h"
#include "bounding_volumes.h"
#include "transform.h"
#include "gl_load.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

struct Geometry 
{
	char* 		  		   filename;
	int   		  		   draw_indexed;
	uint  		  		   vao;
	uint  		  		   vertex_vbo;
	uint  		  		   uv_vbo;
	uint  		  		   normal_vbo;
	uint  		  		   color_vbo;
	uint  		  		   index_vbo;
	int   		  		   ref_count;
	vec3* 		  		   vertices;
	vec3* 		  		   vertex_colors;
	vec3* 		  		   normals;
	vec2* 		  		   uvs;
	uint* 		  		   indices;
	struct Bounding_Box    bounding_box;
	struct Bounding_Sphere bounding_sphere;
};


/* Data */
static struct Geometry* geometry_list;
static int*             empty_indices;
static GLenum*          draw_modes;

static int              load_from_file(struct Geometry* geometry, const char* filename);
static void             create_vao(struct Geometry* geometry);
static struct Geometry* generate_new_index(int* out_new_index);

void geom_init(void)
{
	geometry_list = array_new(struct Geometry);
	empty_indices = array_new(int);
	draw_modes    = array_new_cap(GLenum, GDM_NUM_DRAWMODES);
	draw_modes[GDM_TRIANGLES] = GL_TRIANGLES;
	draw_modes[GDM_LINES]     = GL_LINES;
	draw_modes[GDM_POINTS]    = GL_POINTS;
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

void geom_bounding_volume_generate(int index)
{
	struct Geometry*        geometry = &geometry_list[index];
	struct Bounding_Box*    box      = &geometry->bounding_box;
	struct Bounding_Sphere* sphere   = &geometry->bounding_sphere;
	
	vec3_fill(&box->max, -FLT_MIN, -FLT_MIN, -FLT_MIN);
	vec3_fill(&box->min,  FLT_MAX,  FLT_MAX,  FLT_MAX);
	vec3_fill(&sphere->center, 0.f, 0.f, 0.f);
	sphere->radius = 0.f;
	
	for(int i = 0; i < array_len(geometry->vertices); i++)
	{
		vec3* vertex = &geometry->vertices[i];
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
	vec3_sub(&len_vec, &box->max, &sphere->center);
	sphere->radius = fabsf(vec3_len(&len_vec));
}

void geom_bounding_volume_generate_all(void)
{
	for(int i = 0; i < array_len(geometry_list); i++)
		geom_bounding_volume_generate(i);
}

static struct Geometry* generate_new_index(int* out_new_index)
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
		
		if(load_from_file(new_geo, name))
		{
			create_vao(new_geo);
			geom_bounding_volume_generate(index);
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
	struct Geometry* new_geo = NULL;
	new_geo = generate_new_index(&index);
	assert(new_geo);
	new_geo->filename = str_new(name);
	new_geo->vertices = array_new_cap(vec3, array_len(vertices));
	array_copy(vertices, new_geo->vertices);
	new_geo->normals = array_new_cap(vec3, array_len(normals));
	array_copy(normals, new_geo->normals);
	new_geo->uvs = array_new_cap(vec2, array_len(uvs));
	array_copy(uvs, new_geo->uvs);
	new_geo->indices = array_new_cap(uint, array_len(indices));
	array_copy(indices, new_geo->indices);
	if(vertex_colors)
	{
		new_geo->vertex_colors = array_new_cap(vec3, array_len(vertex_colors));
		array_copy(vertex_colors, new_geo->vertex_colors);
	}
	else
	{
		new_geo->vertex_colors = array_new(vec3);
	}
	create_vao(new_geo);
	//generateBoundingBox(index);
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
				if(geometry->indices)       array_free(geometry->indices);
				if(geometry->vertices)      array_free(geometry->vertices);
				if(geometry->uvs)           array_free(geometry->uvs);
				if(geometry->normals)       array_free(geometry->normals);
				if(geometry->vertex_colors) array_free(geometry->vertex_colors);
				if(geometry->filename)      free(geometry->filename);
				geometry->indices       = NULL;
				geometry->vertices      = NULL;
				geometry->uvs           = NULL;
				geometry->normals       = NULL;
				geometry->vertex_colors = NULL;
				geometry->filename      = NULL;
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

static int load_from_file(struct Geometry* geometry, const char* filename)
{
	assert(filename);
	int success = 1;
	char* full_path = str_new("models/%s", filename);
			
	FILE* file = io_file_open(DT_INSTALL, full_path, "rb");
	free(full_path);
	if(file)
	{				
		const uint32 INDEX_SIZE = sizeof(uint32);
		const uint32 VEC3_SIZE  = sizeof(vec3);
		const uint32 VEC2_SIZE  = sizeof(vec2);
		uint32 header[4];
		size_t bytes_read = 0;
		if((bytes_read = fread(header, INDEX_SIZE, 4, file)) <= 0)
		{
			log_error("geometry:load_from_file", "Read failed");
			success = 0;
		}
		else
		{
			uint32 indices_count  = header[0];
			uint32 vertices_count = header[1];
			uint32 normals_count  = header[2];
			uint32 uvs_count      = header[3];
			// Indices
			geometry->indices = array_new_cap(uint, indices_count);
			fread(geometry->indices, INDEX_SIZE, indices_count, file);
			array_match_len_cap(geometry->indices);
			// Vertices
			geometry->vertices = array_new_cap(vec3, vertices_count);
			fread(geometry->vertices, VEC3_SIZE, vertices_count, file);
			array_match_len_cap(geometry->vertices);
			// Normals
			geometry->normals = array_new_cap(vec3, normals_count);
			fread(geometry->normals, VEC3_SIZE, normals_count, file);
			array_match_len_cap(geometry->normals);
			// Uvs
			geometry->uvs = array_new_cap(vec2, uvs_count);
			fread(geometry->uvs, VEC2_SIZE, uvs_count, file);
			array_match_len_cap(geometry->uvs);

			geometry->vertex_colors = array_new(vec3);
		}
		fclose(file);
		geometry->filename = str_new(filename);
		geometry->draw_indexed = 1;
		geometry->ref_count++;
	}
	else
	{
		success = 0;
	}
	return success;
}

static void create_vao(struct Geometry* geometry)
{
	// TODO: Add support for different model formats and interleaving VBO
	assert(geometry);
	glGenVertexArrays(1, &geometry->vao);
	glBindVertexArray(geometry->vao);

	glGenBuffers(1, &geometry->vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER,
				 array_len(geometry->vertices) * sizeof(vec3),
				 geometry->vertices,
				 GL_STATIC_DRAW);
	renderer_check_glerror("Geometry::create_vbo::vertex");
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if(array_len(geometry->normals) > 0)
	{
		glGenBuffers(1, &geometry->normal_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->normal_vbo);
		glBufferData(GL_ARRAY_BUFFER,
					 array_len(geometry->normals) * sizeof(vec3),
					 geometry->normals,
					 GL_STATIC_DRAW);
		renderer_check_glerror("Geometry::create_vbo::normal");
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 0, 0);
	}

	if(array_len(geometry->uvs) > 0)
	{
		glGenBuffers(1, &geometry->uv_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->uv_vbo);
		glBufferData(GL_ARRAY_BUFFER,
					 array_len(geometry->uvs) * sizeof(vec2),
					 geometry->uvs,
					 GL_STATIC_DRAW);
		renderer_check_glerror("Geometry::create_vbo::uv");
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(array_len(geometry->vertex_colors) > 0)
	{
		glGenBuffers(1, &geometry->color_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, geometry->color_vbo);
		glBufferData(GL_ARRAY_BUFFER,
					 array_len(geometry->vertex_colors) * sizeof(vec3),
					 geometry->vertex_colors,
					 GL_STATIC_DRAW);
		renderer_check_glerror("Geometry::create_vbo::color");
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if(array_len(geometry->indices) > 0)
	{
		glGenBuffers(1, &geometry->index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					 array_len(geometry->indices) * sizeof(GLuint),
					 geometry->indices,
					 GL_STATIC_DRAW);
		geometry->draw_indexed = 1;
	}
	glBindVertexArray(0);
}

void geom_render(int index, enum Geometry_Draw_Mode draw_mode)
{
	assert((int)draw_mode > -1 && draw_mode < GDM_NUM_DRAWMODES);
	struct Geometry* geo = &geometry_list[index];
	glBindVertexArray(geo->vao);
	if(geo->draw_indexed)
		glDrawElements(draw_modes[draw_mode], array_len(geo->indices), GL_UNSIGNED_INT, (void*)0);
	else
		glDrawArrays(draw_modes[draw_mode], 0, array_len(geo->vertices));
	glBindVertexArray(0);
			
}

int geom_render_in_frustum(int                      index,
							vec4*                   frustum,
							struct Transform*       transform,
							enum Geometry_Draw_Mode draw_mode)
{
	int rendered = 0;
	struct Geometry* geometry = &geometry_list[index];
	int intersection = bv_intersect_frustum_sphere(frustum, &geometry->bounding_sphere, transform);
	if(intersection == IT_INTERSECT || intersection == IT_INSIDE)
	{
		geom_render(index, draw_mode);
		rendered = array_len(geometry->indices);
		/* intersection = bv_intersect_frustum_box(frustum, &geometry->bounding_box, transform); */
		/* if(intersection == IT_INTERSECT || intersection == IT_INSIDE) */
		/* { */
		/* 	geom_render(index, draw_mode); */
		/* 	rendered = array_len(geometry->indices); */
		/* } */
	}
	return rendered;
}

struct Bounding_Sphere* geom_bounding_sphere_get(int index)
{
	assert(index > -1 && index < array_len(geometry_list));
	return &geometry_list[index].bounding_sphere;
}
