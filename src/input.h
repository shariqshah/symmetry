#ifndef input_H
#define input_H

#include <stdbool.h>

struct GLFWwindow;
typedef struct GLFWwindow GLFWwindow;

typedef enum 
{
	CM_NORMAL = 0,
	CM_LOCKED,
	CM_HIDDEN
} Cursor_Mode;

void input_init(GLFWwindow* window);
void input_cleanup(void);
bool input_get_mousebutton_state(int button, int state_type);
bool input_get_key_state(int key, int state_type);
void input_get_cursor_pos(double* xpos, double* ypos);
void input_set_cursor_mode(Cursor_Mode mode);

#endif
