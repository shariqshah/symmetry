#ifndef SPRITE_H
#define SPRITE_H

#include "../common/linmath.h"
#include "../common/num_types.h"

#define SPRITE_BATCH_SIZE 1024
#define MAX_SPRITE_VERTICES 6

struct Sprite_Vertex
{
	vec2 position;
	vec2 uv;
	vec4 color;
};

struct Sprite
{
	struct Sprite_Vertex vertices[MAX_SPRITE_VERTICES];
};

struct Sprite_Batch
{
	int           texture;
	int           shader;
	struct Sprite sprites[SPRITE_BATCH_SIZE];
	int           draw_mode;
	uint          vao;
	uint          vbo;
	int           current_sprite_count;
};

void sprite_batch_create(struct Sprite_Batch* batch, const char* texture_name, const char* vert_shader, const char* frag_shader, int draw_mode);
void sprite_batch_remove(struct Sprite_Batch* batch);
void sprite_batch_begin(struct Sprite_Batch* batch);
void sprite_batch_add_sprite(struct Sprite_Batch* batch, struct Sprite* sprite);
void sprite_batch_add_sprite_new(struct Sprite_Batch* batch, int texture, struct Sprite_Vertex vertices[MAX_SPRITE_VERTICES]);
void sprite_batch_end(struct Sprite_Batch* batch);
void sprite_batch_render(struct Sprite_Batch* batch);

#endif
