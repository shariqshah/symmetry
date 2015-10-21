#ifndef framebuffer_H
#define framebuffer_H

void framebuffer_init(void);
void framebuffer_cleanup(void);
int  framebuffer_create(int width, int height, int has_depth, int has_color);
void framebuffer_bind(int index);
void framebuffer_remove(int index);
void framebuffer_unbind(void);
int  framebuffer_get_width(int index);
int  framebuffer_get_height(int index);
void framebuffer_set_texture(int index, int texture, int attachment);
int  framebuffer_get_texture(int index);

#endif
