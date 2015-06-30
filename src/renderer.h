#ifndef renderer_H
#define renderer_H

typedef struct GLFWwindow GLFWwindow;

void renderer_init(GLFWwindow* window);
void renderer_draw(void);
void renderer_cleanup(void);

#endif
