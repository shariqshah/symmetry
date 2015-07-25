#include "geometry.h"
#include "array.h"
#include "num_types.h"
#include "linmath.h"
#include "string_utils.h"
#include "file_io.h"
#include "log.h"
#include "renderer.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct Geometry 
{
	char* filename;
	int   draw_indexed;
	uint  vao;
	uint  vertex_vbo;
	uint  uv_vbo;
	uint  normal_vbo;
	uint  color_vbo;
	uint  index_vbo;
	uint  ref_count;
	vec3* vertices;
	vec3* vertex_colors;
	vec3* normals;
	vec2* uvs;
	uint* indices;
	/* BoundingBox               boundingBox; */
	/* BoundingSphere            boundingSphere; */
};


/* Data */
static struct Geometry* geometry_list;
static int* empty_indices;

/* Function definitions */
int load_from_file(struct Geometry* geometry, const char* filename);
void create_vao(struct Geometry* geometry);

void geom_init(void)
{
	geometry_list = array_new(struct Geometry);
	empty_indices = array_new(int);
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

int geom_create(const char* name)
{
	// check if exists
	int index = geom_find(name);
	if(index == -1)
	{
		/* add new geometry object or overwrite existing one */
		struct Geometry* new_geo = NULL;
		int index = -1;
		int empty_len = array_len(empty_indices);
		if(empty_len > 0)
		{
			index = empty_indices[empty_len - 1];
			array_pop(empty_indices);
			new_geo = &geometry_list[index];
		}
		else
		{
			new_geo = array_grow(geometry_list, struct Geometry);
			index = array_len(geometry_list) - 1;
		}
		assert(new_geo);
		
		if(load_from_file(new_geo, name))
		{
			create_vao(new_geo);
			//generateBoundingBox(index);
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

void geom_remove(int index)
{
	if(index >= 0 && index < array_len(geometry_list))
	{
		struct Geometry* geometry = &geometry_list[index];
		array_free(geometry->indices);
		array_free(geometry->vertices);
		array_free(geometry->uvs);
		array_free(geometry->normals);
		array_free(geometry->vertex_colors);
		free(geometry->filename);
		array_push(empty_indices, index, int);
	}
}

void geom_cleanup(void)
{
	for(int i = 0; i < array_len(geometry_list); i++)
		geom_remove(i);
	
	array_free(geometry_list);
	array_free(empty_indices);
}

int load_from_file(struct Geometry* geometry, const char* filename)
{
	assert(filename);
	int success = 1;
	char* full_path = str_new("models/");
	full_path = str_concat(full_path, filename);
			
	FILE* file = io_file_open(full_path, "rb");
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
			// Vertices
			geometry->vertices = array_new_cap(vec3, vertices_count);
			fread(geometry->vertices, VEC3_SIZE, vertices_count, file);
			// Normals
			geometry->normals = array_new_cap(vec3, normals_count);
			fread(geometry->normals, VEC3_SIZE, normals_count, file);
			// Uvs
			geometry->uvs = array_new_cap(vec2, uvs_count);
			fread(geometry->uvs, VEC2_SIZE, uvs_count, file);
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

void create_vao(struct Geometry* geometry)
{
	// TODO : Add support for different model formats and interleaving VBO
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
