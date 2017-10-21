#include "sprite.h"
#include "texture.h"
#include "shader.h"
#include "renderer.h"
#include "../common/log.h"
#include "../common/array.h"
#include "gl_load.h"

#include <assert.h>


static int sprite_count = 0;

void sprite_init(void)
{
	
}

void sprite_cleanup(void)
{

}

void sprite_batch_create(struct Sprite_Batch* batch, const char* texture_name, const char* vert_shader, const char* frag_shader, int draw_mode)
{
	assert(batch);
	memset(&batch->sprites[0], '\0', sizeof(struct Sprite) * SPRITE_BATCH_SIZE);
	glGenVertexArrays(1, &batch->vao);
	
	glBindVertexArray(batch->vao);

	glGenBuffers(1, &batch->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
	glBufferData(GL_ARRAY_BUFFER,
		         sizeof(struct Vertex) * MAX_SPRITE_VERTICES * SPRITE_BATCH_SIZE,
		         NULL,
		         GL_STREAM_DRAW);
	renderer_check_glerror("sprite_batch_create:glBufferData");

	// Position
	glVertexAttribPointer(AL_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), 0); 
	renderer_check_glerror("sprite_batch_create:glVertexAttribPointer");
	glEnableVertexAttribArray(AL_POSITION);
	renderer_check_glerror("sprite_batch_create:glEnableVertexAttribPointer");

	// Uvs
	glVertexAttribPointer(AL_UV, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), sizeof(vec3));
	renderer_check_glerror("sprite_batch_create:glVertexAttribPointer");
	glEnableVertexAttribArray(AL_UV);
	renderer_check_glerror("sprite_batch_create:glEnableVertexAttribPointer");

	// Color
	glVertexAttribPointer(AL_COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), sizeof(vec3) + sizeof(vec2));
	renderer_check_glerror("sprite_batch_create:glVertexAttribPointer");
	glEnableVertexAttribArray(AL_COLOR);
	renderer_check_glerror("sprite_batch_create:glEnableVertexAttribPointer");

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int texture = texture_create_from_file(texture_name, TU_DIFFUSE);
	if(texture < 0)
	{
		log_error("sprite_batch:create", "Failed to find texture '%s' when creating sprite batch", texture_name);
		texture = texture_create_from_file("default.tga", TU_DIFFUSE);
	}
	batch->texture = texture;

	int shader = shader_create(vert_shader, frag_shader);
	if(shader < -1)
	{
		log_error("sprite_batch:create", "Failed to create shader from '%s'/'%s' sprite batch", vert_shader, frag_shader);
		shader = shader_create("default.vert", "default.frag");
	}
	batch->shader = shader;
	batch->draw_mode = draw_mode;
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

void sprite_batch_add_sprite_new(struct Sprite_Batch* batch, int texture, struct Vertex vertices[MAX_SPRITE_VERTICES])
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
	glBufferSubData(GL_ARRAY_BUFFER,
					0,
					sizeof(struct Vertex) * MAX_SPRITE_VERTICES * batch->current_sprite_count,
		            &batch->sprites[0]);
	renderer_check_glerror("sprite_batch_end:glBufferSubData");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void sprite_batch_render(struct Sprite_Batch* batch)
{
	assert(batch);
	
	texture_bind(batch->texture);
	glBindVertexArray(batch->vao);
	
	glDrawArrays(batch->draw_mode, 0, MAX_SPRITE_VERTICES * batch->current_sprite_count);
	renderer_check_glerror("sprite_batch_render:glDrawArrays");

	glBindVertexArray(0);
	texture_unbind(batch->texture);
}
