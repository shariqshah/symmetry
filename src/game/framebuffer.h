#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "../common/num_types.h"

enum Framebuffer_Attachment
{
	FA_COLOR_ATTACHMENT0 = 0,
	FA_DEPTH_ATTACHMENT,
	FA_NUM_ATTACHMENTS
};

void framebuffer_init(void);
void framebuffer_cleanup(void);
int  framebuffer_create(int width, int height, bool has_depth, bool has_color, bool resizeable);
void framebuffer_bind(int index);
void framebuffer_remove(int index);
void framebuffer_unbind(void);
int  framebuffer_width_get(int index);
int  framebuffer_height_get(int index);
void framebuffer_texture_set(int index, int texture, enum Framebuffer_Attachment attachment);
int  framebuffer_texture_get(int index, enum Framebuffer_Attachment attachment);
uint framebuffer_gl_handle_get(int index);
void framebuffer_resize(int index, int width, int height);
void framebuffer_resize_all(int width, int height);
void framebuffer_resizeable_set(int index, bool resizeable);

#endif
