#include "sprite.h"
#include "texture.h"
#include "shader.h"
#include "renderer.h"
#include "../common/log.h"
#include "../common/array.h"
#include "gl_load.h"

#include <assert.h>
#include <string.h>

void sprite_batch_create(struct Sprite_Batch* batch, const char* texture_name, const char* vert_shader, const char* frag_shader, int draw_mode)
{
	assert(batch);
	memset(&batch->sprites[0], 0, sizeof(struct Sprite) * SPRITE_BATCH_SIZE);
	glGenVertexArrays(1, &batch->vao);
	
	glBindVertexArray(batch->vao);

	glGenBuffers(1, &batch->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER,
		         sizeof(struct Sprite_Vertex) * MAX_SPRITE_VERTICES * SPRITE_BATCH_SIZE,
		         NULL,
		         GL_STREAM_DRAW));

	// Position
	GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_POSITION, 2, GL_FLOAT, GL_FALSE, sizeof(struct Sprite_Vertex), 0)); 
	GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_POSITION));

	// Uvs
	GL_CHECK(glVertexAttribPointer(ATRRIB_LOC_UV, 2, GL_FLOAT, GL_FALSE, sizeof(struct Sprite_Vertex), sizeof(vec2)));
	GL_CHECK(glEnableVertexAttribArray(ATRRIB_LOC_UV));

	// Color
	GL_CHECK(glVertexAttribPointer(ATTRIB_LOC_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(struct Sprite_Vertex), sizeof(vec2) + sizeof(vec2)));
	GL_CHECK(glEnableVertexAttribArray(ATTRIB_LOC_COLOR));

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int texture = texture_create_from_file(texture_name, TU_DIFFUSE);
	if(texture < 0)
	{
		log_error("sprite_batch:create", "Failed to find texture '%s' when creating sprite batch", texture_name);
		texture = texture_create_from_file("default.tga", TU_DIFFUSE);
	}
	batch->texture = texture;

	int shader = shader_create(vert_shader, frag_shader, NULL);
	if(shader < -1)
	{
		log_error("sprite_batch:create", "Failed to create shader from '%s'/'%s' sprite batch", vert_shader, frag_shader);
		shader = shader_create("default.vert", "default.frag", NULL);
	}
	batch->shader = shader;
	batch->draw_mode = draw_mode;
	batch->current_sprite_count = 0;
}

void sprite_batch_remove(struct Sprite_Batch * batch)
{
	assert(batch);
	glDeleteBuffers(1, &batch->vbo);
	glDeleteVertexArrays(1, &batch->vbo);
	batch->current_sprite_count = 0;
	texture_remove(batch->texture);
	shader_remove(batch->shader);

	batch->texture = -1;
	batch->shader  = -1;
	batch->current_sprite_count = 0;
	batch->vao = 0;
	batch->vbo = 0;
	memset(&batch->sprites[0], 0, sizeof(struct Sprite) * SPRITE_BATCH_SIZE);
}

void sprite_batch_begin(struct Sprite_Batch* batch)
{
	assert(batch);
	batch->current_sprite_count = 0;
}

void sprite_batch_add_sprite(struct Sprite_Batch* batch, struct Sprite* sprite)
{
	assert(batch);
	if(batch->current_sprite_count < SPRITE_BATCH_SIZE)
	{
		batch->current_sprite_count++;
		memcpy(&batch->sprites[batch->current_sprite_count - 1], sprite, sizeof(struct Sprite));
	}
	else
	{
		log_error("sprite_batch:add_sprite", "Batch full");
	}
}

void sprite_batch_add_sprite_new(struct Sprite_Batch* batch, int texture, struct Sprite_Vertex vertices[MAX_SPRITE_VERTICES])
{
	assert(batch);
	if(batch->current_sprite_count < SPRITE_BATCH_SIZE)
	{
		batch->current_sprite_count++;
		memcpy(&batch->sprites[batch->current_sprite_count - 1], &vertices[0], sizeof(struct Sprite));
	}
	else
	{
		log_error("sprite_batch:add_sprite", "Batch full");
	}
}

void sprite_batch_end(struct Sprite_Batch* batch)
{
	assert(batch);
	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER,
					0,
					sizeof(struct Sprite_Vertex) * MAX_SPRITE_VERTICES * batch->current_sprite_count,
		            &batch->sprites[0]));
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sprite_batch_render(struct Sprite_Batch* batch)
{
	assert(batch);
	if(batch->current_sprite_count == 0) return;

	texture_bind(batch->texture);
	glBindVertexArray(batch->vao);
	
	GL_CHECK(glDrawArrays(batch->draw_mode, 0, MAX_SPRITE_VERTICES * batch->current_sprite_count));

	glBindVertexArray(0);
	texture_unbind(batch->texture);
}
