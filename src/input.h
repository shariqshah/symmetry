#ifndef input_H
#define input_H

#include "array.h"

typedef struct GLFWwindow GLFWwindow;

typedef enum 
{
	CM_NORMAL = 0,
	CM_LOCKED,
	CM_HIDDEN
	
} Cursor_Mode;


typedef struct
{
	Array*      keys;
	const char* name;
	int         state;
	
} Input_Map;

void input_init(GLFWwindow* window);
void input_cleanup(void);
bool input_mousebutton_state_get(int button, int state_type);
bool input_key_state_get(int key, int state_type);
void input_cursor_pos_get(double* xpos, double* ypos);
void input_cursor_mode_set(Cursor_Mode mode);
void input_update(void);
bool input_map_state_get(const char* map_name, int state);
void input_map_create(const char* name, int* keys, size_t num_keys);
bool input_map_remvove(const char* name);
bool input_map_keys_set(const char* name, int* keys, int num_keys);
bool input_map_name_set(const char* name, const char* new_name);

#endif
