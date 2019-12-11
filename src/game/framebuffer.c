#include "framebuffer.h"
#include "../common/array.h"
#include "renderer.h"
#include "../common/log.h"
#include "texture.h"
#include "gl_load.h"

#include <assert.h>

struct FBO
{
	uint handle;
	uint depth_renderbuffer;
	uint color_renderbuffer;
	int  texture_attachments[FA_NUM_ATTACHMENTS];
	int  width;
	int  height;
	bool resizeable;
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
	fbo_list      = NULL;
	empty_indices = NULL;
}

int framebuffer_create(int width, int height, bool has_depth, bool has_color, bool resizeable)
{
	int    index              = -1;
	GLuint fbo                =  0;
	GLuint depth_renderbuffer =  0;
	GLuint color_renderbuffer =  0;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if(has_depth)
	{
		glGenRenderbuffers(1, &depth_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER,
							  GL_DEPTH_COMPONENT,
							  width,
							  height));
		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_DEPTH_ATTACHMENT,
								  GL_RENDERBUFFER,
								  depth_renderbuffer));
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	if(has_color)
	{
		glGenRenderbuffers(1, &color_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, color_renderbuffer);
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER,
							  GL_RGBA8,
							  width,
							  height));
		GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER,
								  color_renderbuffer));
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	GL_CHECK(glDrawBuffer(has_color ? GL_COLOR_ATTACHMENT0 : GL_NONE));
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		log_error("framebuffer:create", "Framebuffer not created!");
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
			framebuffer = &fbo_list[index];
		}
		
		framebuffer->handle             = fbo;
		framebuffer->depth_renderbuffer = depth_renderbuffer;
		framebuffer->color_renderbuffer = color_renderbuffer;
		framebuffer->width              = width;
		framebuffer->height             = height;
		framebuffer->resizeable         = resizeable;
		for(int i = 0; i < FA_NUM_ATTACHMENTS; i++) framebuffer->texture_attachments[i] = -1;
		log_message("Framebuffer created successfully");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return index;
}

void framebuffer_bind(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_list[index].handle);
	/* if(fbo_list[index].color_renderbuffer != 0) */
	/* 	glDrawBuffer(GL_COLOR_ATTACHMENT0); */
	/* else */
	/* 	glDrawBuffer(GL_NONE); */
}

void framebuffer_remove(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	struct FBO* fbo = &fbo_list[index];
	for(int i = 0; i < FA_NUM_ATTACHMENTS; i++)
	{
		if(fbo->texture_attachments[i] != -1)
		{
			texture_remove(fbo->texture_attachments[i]);
			fbo->texture_attachments[i] = -1;
		}
	}
	if(fbo->depth_renderbuffer != 0) glDeleteRenderbuffers(1, &fbo->depth_renderbuffer);
	if(fbo->color_renderbuffer != 0) glDeleteRenderbuffers(1, &fbo->color_renderbuffer);
	glDeleteFramebuffers(1, &fbo->handle);
	fbo->color_renderbuffer =  0;
	fbo->depth_renderbuffer =  0;
	fbo->handle             =  0;
	fbo->width              = -1;
	fbo->height             = -1;
	fbo->resizeable         = false;
}

void framebuffer_unbind(void)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int framebuffer_width_get(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].width;
}

int framebuffer_height_get(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].height;
}

void framebuffer_texture_set(int index, int texture, enum Framebuffer_Attachment attachment)
{
	assert(index < array_len(fbo_list) && index > -1);
	GLenum gl_attachment = -1;
	switch(attachment)
	{
	case     FA_COLOR_ATTACHMENT0: gl_attachment = GL_COLOR_ATTACHMENT0; break;
	case     FA_DEPTH_ATTACHMENT:  gl_attachment = GL_DEPTH_ATTACHMENT;  break;
	default: log_error("framebuffer:set_texture", "Invalid attachment type"); return;
	};

	struct FBO* fbo     = &fbo_list[index];
	int current_texture = fbo->texture_attachments[attachment];
	if(current_texture != -1)
	{
		texture_remove(current_texture);
		fbo->texture_attachments[attachment] = -1;
	}

	if(texture == -1) return;
	
	GLint current_fbo = 0;
	GL_CHECK(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo));
	framebuffer_bind(index);
	GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
						   gl_attachment,
						   GL_TEXTURE_2D,
						   texture_get_texture_handle(texture),
						   0));
	fbo->texture_attachments[attachment] = texture;
	if(attachment == FA_COLOR_ATTACHMENT0) glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
}

int framebuffer_texture_get(int index, enum Framebuffer_Attachment attachment)
{
	assert(index < array_len(fbo_list) &&
		   index > -1 &&
		   attachment < FA_NUM_ATTACHMENTS &&
		   (int)attachment > -1);
	return fbo_list[index].texture_attachments[attachment];
}

uint framebuffer_gl_handle_get(int index)
{
	assert(index < array_len(fbo_list) && index > -1);
	return fbo_list[index].handle;
}

void framebuffer_resize(int index, int width, int height)
{
	assert(index > -1 && index < array_len(fbo_list));
	width  -= (width % 2);
	height -= (height % 2);
	GLint current_fbo = 0;
	GL_CHECK(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo));
	struct FBO* fbo = &fbo_list[index];
	if(!fbo->resizeable) return;
	framebuffer_bind(index);
	if(fbo->depth_renderbuffer != 0)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, fbo->depth_renderbuffer);
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER,
							  GL_DEPTH_COMPONENT,
							  width,
							  height));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_DEPTH_ATTACHMENT,
								  GL_RENDERBUFFER,
								  fbo->depth_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	if(fbo->color_renderbuffer != 0)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, fbo->color_renderbuffer);
		GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER,
							  GL_RGBA8,
							  width,
							  height));
		glFramebufferRenderbuffer(GL_FRAMEBUFFER,
								  GL_COLOR_ATTACHMENT0,
								  GL_RENDERBUFFER,
								  fbo->color_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	for(int i = 0; i < FA_NUM_ATTACHMENTS; i++)
	{
		if(fbo->texture_attachments[i] != -1)
		{
			int texture = fbo->texture_attachments[i];
			GLenum gl_attachment = -1;
			switch(i)
			{
			case     FA_COLOR_ATTACHMENT0: gl_attachment = GL_COLOR_ATTACHMENT0; break;
			case     FA_DEPTH_ATTACHMENT:  gl_attachment = GL_DEPTH_ATTACHMENT;  break;
			default: log_error("framebuffer:resize", "Invalid attachment type"); continue;
			};
			
			texture_resize(texture, width, height, NULL);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
								   gl_attachment,
								   GL_TEXTURE_2D,
								   texture_get_texture_handle(texture),
								   0);
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
	fbo->width  = width;
	fbo->height = height;
	log_message("Resized framebuffer to %dx%d", width, height);
}

void framebuffer_resize_all(int width, int height)
{
	for(int i = 0; i < array_len(fbo_list); i++)
	{
		if(fbo_list[i].resizeable) framebuffer_resize(i, width, height);
	}
}

void framebuffer_resizeable_set(int index, bool resizeable)
{
	assert(index > -1 && index < array_len(fbo_list));
	fbo_list[index].resizeable = resizeable ? true : false;
}
