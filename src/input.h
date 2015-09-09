#ifndef input_H
#define input_H

#include <stdlib.h>

typedef struct GLFWwindow GLFWwindow;

enum Cursor_Mode
{
	CM_NORMAL = 0,
	CM_LOCKED,
	CM_HIDDEN
	
};

void input_init(GLFWwindow* window);
void input_cleanup(void);
int  input_mousebutton_state_get(int button, int state_type);
int  input_key_state_get(int key, int state_type);
void input_cursor_pos_get(double* xpos, double* ypos);
void input_cursor_pos_set(double xpos, double ypos);
void input_cursor_mode_set(enum Cursor_Mode mode);
void input_update(void);
int  input_map_state_get(const char* map_name, int state);
void input_map_create(const char* name, int* keys, size_t num_keys);
int  input_map_remove(const char* name);
int  input_map_keys_set(const char* name, int* keys, int num_keys);
int  input_map_name_set(const char* name, const char* new_name);

#endif
