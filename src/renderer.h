#ifndef renderer_H
#define renderer_H

typedef struct GLFWwindow GLFWwindow;

void renderer_init(GLFWwindow* window);
void renderer_draw(void);
void renderer_cleanup(void);
void renderer_set_clearcolor(float r, float g, float b, float a);

#endif
