#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "array.h"
#include "GLFW/glfw3.h"

#include "window_system.h"
#include "log.h"

#define KS_INACTIVE -1; 			/* state for input map is set to KS_INACTIVE(KeyState_Inactive) when
									   the key is neither pressed nor released */

struct Input_Map
{
	const char* name;
	int* keys;
	int  state;
};

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods);
static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods);
static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos);
static int  map_find(const char* name);
static const char* get_key_name(int key);

static struct Input_Map* input_map_list;

void input_init(GLFWwindow* window)
{
	glfwSetMouseButtonCallback(window, input_on_mousebutton);
	glfwSetKeyCallback(window, input_on_key);
	glfwSetCursorPosCallback(window, input_on_cursor_move);
	//glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
	
	input_map_list = array_new(struct Input_Map);
}

void input_cleanup(void)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		array_free(map->keys);
	}
	array_free(input_map_list);
}

static void input_on_cursor_move(GLFWwindow* window, double xpos, double ypos)
{
	
}

void input_cursor_pos_get(double* xpos, double* ypos)
{
	assert(xpos && ypos);
	GLFWwindow* window = window_get_active();
	glfwGetCursorPos(window, xpos, ypos);
}

void input_cursor_pos_set(double xpos, double ypos)
{
	GLFWwindow* window = window_get_active();
	glfwSetCursorPos(window, xpos, ypos);
}

static void input_on_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		for(int j = 0; j < array_len(map->keys); j++)
		{
			if(map->keys[j] == key)
			{
				map->state = action;
				break;
			}
		}
	}
}

static void input_on_mousebutton(GLFWwindow* window, int button, int action, int mods)
{
	/* Probably add 'mouse maps', same as input maps for keyvboard but with buttons
	   Do we even need that?
	*/
}

void input_cursor_mode_set(enum Cursor_Mode mode)
{
	GLFWwindow* window = window_get_active();
	int cursor_mode = GLFW_CURSOR_NORMAL;
	if(mode == CM_HIDDEN)
		cursor_mode = GLFW_CURSOR_HIDDEN;
	else if(mode == CM_LOCKED)
		cursor_mode = GLFW_CURSOR_DISABLED;
	
	glfwSetInputMode(window, GLFW_CURSOR, cursor_mode);
}

int input_map_state_get(const char* map_name, int state)
{
	int current_state = KS_INACTIVE;
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(strcmp(map->name, map_name) == 0)
		{
			current_state = map->state;
			break;
		}
	}
	
	if(state == GLFW_PRESS)
	{
		 if(current_state == GLFW_PRESS || current_state == GLFW_REPEAT)
			 return 1;
		 else
			 return 0;
	}
	else
	{
		return state == current_state ? 1 : 0;
	}
}

int input_key_state_get(int key, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetKey(window, key);
	if(state_type == GLFW_PRESS)
	{
		 if(current_state == GLFW_PRESS || current_state == GLFW_REPEAT)
			 return 1;
		 else
			 return 0;
	}
	else
	{
		return state_type == current_state ? 1 : 0;
	}
}

int input_mousebutton_state_get(int button, int state_type)
{
	GLFWwindow* window = window_get_active();
	int current_state  = glfwGetMouseButton(window, button);
	if(state_type == GLFW_PRESS)
	{
		 if(current_state == GLFW_PRESS || current_state == GLFW_REPEAT)
			 return 1;
		 else
			 return 0;
	}
	else
	{
		return state_type == current_state ? 1 : 0;
	}
}

void input_map_create(const char* name, int* keys, size_t num_keys)
{
	assert(name && keys && num_keys > 0);

	struct Input_Map* new_map = array_grow(input_map_list, struct Input_Map);
	new_map->name  = name;
	new_map->keys  = array_new(int);
	new_map->state = KS_INACTIVE;
	for(size_t i = 0; i < num_keys; i++)
	{
		array_push(new_map->keys, keys[i], int);
	}
}

void input_update(void)
{
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(map->state == GLFW_RELEASE)
			map->state = KS_INACTIVE;
	}
}

int input_map_remove(const char* name)
{
	assert(name);
	int success = 0;
    int index = map_find(name);
	if(index > -1)
	{
		array_remove_at(input_map_list, (int)index);
		success = 1;
	}	
	if(!success) log_error("input:map_remove", "Map %s not found", name);
	
	return success;
}

int input_map_keys_set(const char* name, int* keys, int num_keys)
{
	assert(name && keys && num_keys > 0);
	int success = 0;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		array_reset(map->keys, num_keys);
		for(int i = 0; i < num_keys; i++)
			map->keys[i] = keys[i];
		success = 1;
	}
	if(!success)
		log_error("input:map_keys_set", "Map %s not found", name);	
	return success;
}

int input_map_name_set(const char* name, const char* new_name)
{
	assert(name && new_name);
	int success = 0;
	int index = map_find(name);
	if(index > -1)
	{
		struct Input_Map* map = &input_map_list[index];
		map->name = new_name;
		success = 1;
	}
	if(!success) log_error("input:map_name_set", "Map %s not found", name);
	return success;
}

static int map_find(const char* name)
{
	int index = -1;
	for(int i = 0; i < array_len(input_map_list); i++)
	{
		struct Input_Map* map = &input_map_list[i];
		if(strcmp(name, map->name) == 0)
		{
			index = i;
			break;
		}
	}
	return index;
}

static const char* get_key_name(int key)
{
    switch (key)
    {
        // Printable keys
        case GLFW_KEY_A:            return "A";
        case GLFW_KEY_B:            return "B";
        case GLFW_KEY_C:            return "C";
        case GLFW_KEY_D:            return "D";
        case GLFW_KEY_E:            return "E";
        case GLFW_KEY_F:            return "F";
        case GLFW_KEY_G:            return "G";
        case GLFW_KEY_H:            return "H";
        case GLFW_KEY_I:            return "I";
        case GLFW_KEY_J:            return "J";
        case GLFW_KEY_K:            return "K";
        case GLFW_KEY_L:            return "L";
        case GLFW_KEY_M:            return "M";
        case GLFW_KEY_N:            return "N";
        case GLFW_KEY_O:            return "O";
        case GLFW_KEY_P:            return "P";
        case GLFW_KEY_Q:            return "Q";
        case GLFW_KEY_R:            return "R";
        case GLFW_KEY_S:            return "S";
        case GLFW_KEY_T:            return "T";
        case GLFW_KEY_U:            return "U";
        case GLFW_KEY_V:            return "V";
        case GLFW_KEY_W:            return "W";
        case GLFW_KEY_X:            return "X";
        case GLFW_KEY_Y:            return "Y";
        case GLFW_KEY_Z:            return "Z";
        case GLFW_KEY_1:            return "1";
        case GLFW_KEY_2:            return "2";
        case GLFW_KEY_3:            return "3";
        case GLFW_KEY_4:            return "4";
        case GLFW_KEY_5:            return "5";
        case GLFW_KEY_6:            return "6";
        case GLFW_KEY_7:            return "7";
        case GLFW_KEY_8:            return "8";
        case GLFW_KEY_9:            return "9";
        case GLFW_KEY_0:            return "0";
        case GLFW_KEY_SPACE:        return "SPACE";
        case GLFW_KEY_MINUS:        return "MINUS";
        case GLFW_KEY_EQUAL:        return "EQUAL";
        case GLFW_KEY_LEFT_BRACKET: return "LEFT BRACKET";
        case GLFW_KEY_RIGHT_BRACKET: return "RIGHT BRACKET";
        case GLFW_KEY_BACKSLASH:    return "BACKSLASH";
        case GLFW_KEY_SEMICOLON:    return "SEMICOLON";
        case GLFW_KEY_APOSTROPHE:   return "APOSTROPHE";
        case GLFW_KEY_GRAVE_ACCENT: return "GRAVE ACCENT";
        case GLFW_KEY_COMMA:        return "COMMA";
        case GLFW_KEY_PERIOD:       return "PERIOD";
        case GLFW_KEY_SLASH:        return "SLASH";
        case GLFW_KEY_WORLD_1:      return "WORLD 1";
        case GLFW_KEY_WORLD_2:      return "WORLD 2";

        // Function keys
        case GLFW_KEY_ESCAPE:       return "ESCAPE";
        case GLFW_KEY_F1:           return "F1";
        case GLFW_KEY_F2:           return "F2";
        case GLFW_KEY_F3:           return "F3";
        case GLFW_KEY_F4:           return "F4";
        case GLFW_KEY_F5:           return "F5";
        case GLFW_KEY_F6:           return "F6";
        case GLFW_KEY_F7:           return "F7";
        case GLFW_KEY_F8:           return "F8";
        case GLFW_KEY_F9:           return "F9";
        case GLFW_KEY_F10:          return "F10";
        case GLFW_KEY_F11:          return "F11";
        case GLFW_KEY_F12:          return "F12";
        case GLFW_KEY_F13:          return "F13";
        case GLFW_KEY_F14:          return "F14";
        case GLFW_KEY_F15:          return "F15";
        case GLFW_KEY_F16:          return "F16";
        case GLFW_KEY_F17:          return "F17";
        case GLFW_KEY_F18:          return "F18";
        case GLFW_KEY_F19:          return "F19";
        case GLFW_KEY_F20:          return "F20";
        case GLFW_KEY_F21:          return "F21";
        case GLFW_KEY_F22:          return "F22";
        case GLFW_KEY_F23:          return "F23";
        case GLFW_KEY_F24:          return "F24";
        case GLFW_KEY_F25:          return "F25";
        case GLFW_KEY_UP:           return "UP";
        case GLFW_KEY_DOWN:         return "DOWN";
        case GLFW_KEY_LEFT:         return "LEFT";
        case GLFW_KEY_RIGHT:        return "RIGHT";
        case GLFW_KEY_LEFT_SHIFT:   return "LEFT SHIFT";
        case GLFW_KEY_RIGHT_SHIFT:  return "RIGHT SHIFT";
        case GLFW_KEY_LEFT_CONTROL: return "LEFT CONTROL";
        case GLFW_KEY_RIGHT_CONTROL: return "RIGHT CONTROL";
        case GLFW_KEY_LEFT_ALT:     return "LEFT ALT";
        case GLFW_KEY_RIGHT_ALT:    return "RIGHT ALT";
        case GLFW_KEY_TAB:          return "TAB";
        case GLFW_KEY_ENTER:        return "ENTER";
        case GLFW_KEY_BACKSPACE:    return "BACKSPACE";
        case GLFW_KEY_INSERT:       return "INSERT";
        case GLFW_KEY_DELETE:       return "DELETE";
        case GLFW_KEY_PAGE_UP:      return "PAGE UP";
        case GLFW_KEY_PAGE_DOWN:    return "PAGE DOWN";
        case GLFW_KEY_HOME:         return "HOME";
        case GLFW_KEY_END:          return "END";
        case GLFW_KEY_KP_0:         return "KEYPAD 0";
        case GLFW_KEY_KP_1:         return "KEYPAD 1";
        case GLFW_KEY_KP_2:         return "KEYPAD 2";
        case GLFW_KEY_KP_3:         return "KEYPAD 3";
        case GLFW_KEY_KP_4:         return "KEYPAD 4";
        case GLFW_KEY_KP_5:         return "KEYPAD 5";
        case GLFW_KEY_KP_6:         return "KEYPAD 6";
        case GLFW_KEY_KP_7:         return "KEYPAD 7";
        case GLFW_KEY_KP_8:         return "KEYPAD 8";
        case GLFW_KEY_KP_9:         return "KEYPAD 9";
        case GLFW_KEY_KP_DIVIDE:    return "KEYPAD DIVIDE";
        case GLFW_KEY_KP_MULTIPLY:  return "KEYPAD MULTPLY";
        case GLFW_KEY_KP_SUBTRACT:  return "KEYPAD SUBTRACT";
        case GLFW_KEY_KP_ADD:       return "KEYPAD ADD";
        case GLFW_KEY_KP_DECIMAL:   return "KEYPAD DECIMAL";
        case GLFW_KEY_KP_EQUAL:     return "KEYPAD EQUAL";
        case GLFW_KEY_KP_ENTER:     return "KEYPAD ENTER";
        case GLFW_KEY_PRINT_SCREEN: return "PRINT SCREEN";
        case GLFW_KEY_NUM_LOCK:     return "NUM LOCK";
        case GLFW_KEY_CAPS_LOCK:    return "CAPS LOCK";
        case GLFW_KEY_SCROLL_LOCK:  return "SCROLL LOCK";
        case GLFW_KEY_PAUSE:        return "PAUSE";
        case GLFW_KEY_LEFT_SUPER:   return "LEFT SUPER";
        case GLFW_KEY_RIGHT_SUPER:  return "RIGHT SUPER";
        case GLFW_KEY_MENU:         return "MENU";
        default:                    return "UNKNOWN";
    }
}
