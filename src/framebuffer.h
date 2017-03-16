#ifndef framebuffer_H
#define framebuffer_H

#include "num_types.h"

enum Framebuffer_Attachment
{
	FA_COLOR_ATTACHMENT0 = 0,
	FA_DEPTH_ATTACHMENT,
	FA_NUM_ATTACHMENTS
};

void framebuffer_init(void);
void framebuffer_cleanup(void);
int  framebuffer_create(int width, int height, int has_depth, int has_color, int resizeable);
void framebuffer_bind(int index);
void framebuffer_remove(int index);
void framebuffer_unbind(void);
int  framebuffer_get_width(int index);
int  framebuffer_get_height(int index);
void framebuffer_set_texture(int index, int texture, enum Framebuffer_Attachment attachment);
int  framebuffer_get_texture(int index, enum Framebuffer_Attachment attachment);
uint framebuffer_get_gl_handle(int index);
void framebuffer_resize(int index, int width, int height);
void framebuffer_resize_all(int width, int height);
void framebuffer_resizeable_set(int index, int resizeable);

#endif
