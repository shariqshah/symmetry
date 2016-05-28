#include "framebuffer.h"
#include "array.h"
#include "num_types.h"
#include "renderer.h"
#include "log.h"
#include "texture.h"

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <assert.h>

struct FBO
{
	uint handle;
	uint renderbuffer;
	int  texture;
	int  width;
	int  height;
};

struct FBO* fbo_list;
int* empty_indices;

void framebuffer_init(void)
{
	fbo_list = array_new(struct FBO);
	empty_indices = array_new(int);
}

void framebuffer_cleanup(void)
{
	for(int i = 0; i < array_len(fbo_list); i++)
		framebuffer_remove(i);

	array_free(fbo_list);
	array_free(empty_indices);
}

int framebuffer_create(int width, int height, int has_depth, int has_color)
{
	int index = -1;
	GLuint fbo;
	GLuint renderbuffer;
		
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER,
						  GL_DEPTH24_STENCIL8,
						  width,
						  height);
	renderer_check_glerror("framebuffer:create");
	glFramebufferRenderbuffer(GL_FRAMEBUFFER,
							  GL_DEPTH_STENCIL_ATTACHMENT,
							  GL_RENDERBUFFER,
							  renderbuffer);
	renderer_check_glerror("framebuffer:create");
	if(has_color)
	{
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}
		
	if(has_depth)
	{
		glDrawBuffer(GL_NONE);
	}
		
	renderer_check_glerror("framebuffer:create");
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		log_error("framebuffer:create", "Framebuffer not created!");
		renderer_check_glerror("framebuffer:create");
	}
	else
	{
		struct FBO* framebuffer = NULL;
		if(array_len(empty_indices) == 0)
		{
			framebuffer = array_grow(fbo_list, struct FBO);
			index = array_len(fbo_list) - 1;
		}
		else
		{
			index = *array_get_last(empty_indices, int);
			array_pop(empty_indices);
		}
		
		framebuffer->handle       = fbo;
		framebuffer->renderbuffer = renderbuffer;
		framebuffer->texture      = -1;
		framebuffer->width        = width;
		framebuffer->height       = height;
		log_message("Framebuffer created successfully");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	
	return index;
}

void framebuffer_bind(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_list[index].handle);
}

void framebuffer_remove(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	struct FBO* fbo = &fbo_list[index];
	if(fbo->texture != -1) texture_remove(fbo->texture);
	glDeleteRenderbuffers(1, &fbo->renderbuffer);
	glDeleteFramebuffers(1, &fbo->handle);
}

void framebuffer_unbind(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int framebuffer_get_width(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].width;
}

int  framebuffer_get_height(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].height;
}

void framebuffer_set_texture(int index, int texture, int attachment)
{
	assert(index < array_len(fbo_list) && index > -1);
	GLint current_fbo = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);
	renderer_check_glerror("framebuffer:set_texture:glGet");
	framebuffer_bind(index);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
						   attachment,
						   GL_TEXTURE_2D,
						   texture_get_texture_handle(texture),
						   0);
	/* if(!renderer_check_glerror("framebuffer:set_texture:glFramebuffertexture")) */
	/* { */
	/* 	int current_fbo_tex = fbo_list[index].texture; */
	/* 	if(current_fbo_tex > -1) */
	/* 		texture_remove(current_fbo_tex); */

	/* 	fbo_list[index].texture = texture; */
	/* 	texture_inc_refcount(texture); */
	/* } */
	glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
}

int framebuffer_get_texture(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].texture;
}
