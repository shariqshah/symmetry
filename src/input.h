#ifndef input_H
#define input_H

#include <stdlib.h>
#include "num_types.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_events.h>

enum Key_State
{
	KS_INACTIVE = -1,
	KS_PRESSED  = SDL_PRESSED,
	KS_RELEASED = SDL_RELEASED
};

enum Mouse_Mode
{
	MM_NORMAL = 0,
	MM_RELATIVE
};

enum Mouse_Button
{
	MB_LEFT   = SDL_BUTTON_LEFT,
	MB_MIDDLE = SDL_BUTTON_MIDDLE,
	MB_RIGHT  = SDL_BUTTON_RIGHT,
	MB_X1     = SDL_BUTTON_X1,
	MB_X2     = SDL_BUTTON_X2
};

enum Keyboard_Key
{
	KEY_A            = SDLK_a,
	KEY_B            = SDLK_b,
	KEY_C            = SDLK_c,
	KEY_D            = SDLK_d,
	KEY_E            = SDLK_e,
	KEY_F            = SDLK_f,
	KEY_G            = SDLK_g,
	KEY_H            = SDLK_h,
	KEY_I            = SDLK_i,
	KEY_J            = SDLK_j,
	KEY_K            = SDLK_k,
	KEY_L            = SDLK_l,
	KEY_M            = SDLK_m,
	KEY_N            = SDLK_n,
	KEY_O            = SDLK_o,
	KEY_P            = SDLK_p,
	KEY_Q            = SDLK_q,
	KEY_R            = SDLK_r,
	KEY_S            = SDLK_s,
	KEY_T            = SDLK_t,
	KEY_U            = SDLK_u,
	KEY_V            = SDLK_v,
	KEY_W            = SDLK_w,
	KEY_X            = SDLK_x,
	KEY_Y            = SDLK_y,
	KEY_Z            = SDLK_z,
	KEY_0            = SDLK_0,
	KEY_1            = SDLK_1,
	KEY_2            = SDLK_2,
	KEY_3            = SDLK_3,
	KEY_4            = SDLK_4,
	KEY_5            = SDLK_5,
	KEY_6            = SDLK_6,
	KEY_7            = SDLK_7,
	KEY_8            = SDLK_8,
	KEY_9            = SDLK_9,
	KEY_BACKSPACE    = SDLK_BACKSPACE,
	KEY_TAB          = SDLK_TAB,
	KEY_RETURN       = SDLK_RETURN,
	KEY_RETURN2      = SDLK_RETURN2,
	KEY_KP_ENTER     = SDLK_KP_ENTER,
	KEY_SHIFT        = SDLK_LSHIFT,
	KEY_CTRL         = SDLK_LCTRL,
	KEY_ALT          = SDLK_LALT,
	KEY_GUI          = SDLK_LGUI,
	KEY_PAUSE        = SDLK_PAUSE,
	KEY_CAPSLOCK     = SDLK_CAPSLOCK,
	KEY_ESCAPE       = SDLK_ESCAPE,
	KEY_SPACE        = SDLK_SPACE,
	KEY_PAGEUP       = SDLK_PAGEUP,
	KEY_PAGEDOWN     = SDLK_PAGEDOWN,
	KEY_END          = SDLK_END,
	KEY_HOME         = SDLK_HOME,
	KEY_LEFT         = SDLK_LEFT,
	KEY_UP           = SDLK_UP,
	KEY_RIGHT        = SDLK_RIGHT,
	KEY_DOWN         = SDLK_DOWN,
	KEY_SELECT       = SDLK_SELECT,
	KEY_PRINTSCREEN  = SDLK_PRINTSCREEN,
	KEY_INSERT       = SDLK_INSERT,
	KEY_DELETE       = SDLK_DELETE,
	KEY_LGUI         = SDLK_LGUI,
	KEY_RGUI         = SDLK_RGUI,
	KEY_APPLICATION  = SDLK_APPLICATION,
	KEY_KP_0         = SDLK_KP_0,
	KEY_KP_1         = SDLK_KP_1,
	KEY_KP_2         = SDLK_KP_2,
	KEY_KP_3         = SDLK_KP_3,
	KEY_KP_4         = SDLK_KP_4,
	KEY_KP_5         = SDLK_KP_5,
	KEY_KP_6         = SDLK_KP_6,
	KEY_KP_7         = SDLK_KP_7,
	KEY_KP_8         = SDLK_KP_8,
	KEY_KP_9         = SDLK_KP_9,
	KEY_KP_MULTIPLY  = SDLK_KP_MULTIPLY,
	KEY_KP_PLUS      = SDLK_KP_PLUS,
	KEY_KP_MINUS     = SDLK_KP_MINUS,
	KEY_KP_PERIOD    = SDLK_KP_PERIOD,
	KEY_KP_DIVIDE    = SDLK_KP_DIVIDE,
	KEY_F1           = SDLK_F1,
	KEY_F2           = SDLK_F2,
	KEY_F3           = SDLK_F3,
	KEY_F4           = SDLK_F4,
	KEY_F5           = SDLK_F5,
	KEY_F6           = SDLK_F6,
	KEY_F7           = SDLK_F7,
	KEY_F8           = SDLK_F8,
	KEY_F9           = SDLK_F9,
	KEY_F10          = SDLK_F10,
	KEY_F11          = SDLK_F11,
	KEY_F12          = SDLK_F12,
	KEY_F13          = SDLK_F13,
	KEY_F14          = SDLK_F14,
	KEY_F15          = SDLK_F15,
	KEY_F16          = SDLK_F16,
	KEY_F17          = SDLK_F17,
	KEY_F18          = SDLK_F18,
	KEY_F19          = SDLK_F19,
	KEY_F20          = SDLK_F20,
	KEY_F21          = SDLK_F21,
	KEY_F22          = SDLK_F22,
	KEY_F23          = SDLK_F23,
	KEY_F24          = SDLK_F24,
	KEY_NUMLOCKCLEAR = SDLK_NUMLOCKCLEAR,
	KEY_SCROLLLOCK   = SDLK_SCROLLLOCK,
	KEY_LSHIFT       = SDLK_LSHIFT,
	KEY_RSHIFT       = SDLK_RSHIFT,
	KEY_LCTRL        = SDLK_LCTRL,
	KEY_RCTRL        = SDLK_RCTRL,
	KEY_LALT         = SDLK_LALT,
	KEY_RALT         = SDLK_RALT
};

enum Keyboard_Scancode
{
	SCANCODE_UNKNOWN            = SDL_SCANCODE_UNKNOWN,
	SCANCODE_CTRL               = SDL_SCANCODE_LCTRL,
	SCANCODE_SHIFT              = SDL_SCANCODE_LSHIFT,
	SCANCODE_ALT                = SDL_SCANCODE_LALT,
	SCANCODE_GUI                = SDL_SCANCODE_LGUI,
	SCANCODE_A                  = SDL_SCANCODE_A,
	SCANCODE_B                  = SDL_SCANCODE_B,
	SCANCODE_C                  = SDL_SCANCODE_C,
	SCANCODE_D                  = SDL_SCANCODE_D,
	SCANCODE_E                  = SDL_SCANCODE_E,
	SCANCODE_F                  = SDL_SCANCODE_F,
	SCANCODE_G                  = SDL_SCANCODE_G,
	SCANCODE_H                  = SDL_SCANCODE_H,
	SCANCODE_I                  = SDL_SCANCODE_I,
	SCANCODE_J                  = SDL_SCANCODE_J,
	SCANCODE_K                  = SDL_SCANCODE_K,
	SCANCODE_L                  = SDL_SCANCODE_L,
	SCANCODE_M                  = SDL_SCANCODE_M,
	SCANCODE_N                  = SDL_SCANCODE_N,
	SCANCODE_O                  = SDL_SCANCODE_O,
	SCANCODE_P                  = SDL_SCANCODE_P,
	SCANCODE_Q                  = SDL_SCANCODE_Q,
	SCANCODE_R                  = SDL_SCANCODE_R,
	SCANCODE_S                  = SDL_SCANCODE_S,
	SCANCODE_T                  = SDL_SCANCODE_T,
	SCANCODE_U                  = SDL_SCANCODE_U,
	SCANCODE_V                  = SDL_SCANCODE_V,
	SCANCODE_W                  = SDL_SCANCODE_W,
	SCANCODE_X                  = SDL_SCANCODE_X,
	SCANCODE_Y                  = SDL_SCANCODE_Y,
	SCANCODE_Z                  = SDL_SCANCODE_Z,
	SCANCODE_1                  = SDL_SCANCODE_1,
	SCANCODE_2                  = SDL_SCANCODE_2,
	SCANCODE_3                  = SDL_SCANCODE_3,
	SCANCODE_4                  = SDL_SCANCODE_4,
	SCANCODE_5                  = SDL_SCANCODE_5,
	SCANCODE_6                  = SDL_SCANCODE_6,
	SCANCODE_7                  = SDL_SCANCODE_7,
	SCANCODE_8                  = SDL_SCANCODE_8,
	SCANCODE_9                  = SDL_SCANCODE_9,
	SCANCODE_0                  = SDL_SCANCODE_0,
	SCANCODE_RETURN             = SDL_SCANCODE_RETURN,
	SCANCODE_ESCAPE             = SDL_SCANCODE_ESCAPE,
	SCANCODE_BACKSPACE          = SDL_SCANCODE_BACKSPACE,
	SCANCODE_TAB                = SDL_SCANCODE_TAB,
	SCANCODE_SPACE              = SDL_SCANCODE_SPACE,
	SCANCODE_MINUS              = SDL_SCANCODE_MINUS,
	SCANCODE_EQUALS             = SDL_SCANCODE_EQUALS,
	SCANCODE_LEFTBRACKET        = SDL_SCANCODE_LEFTBRACKET,
	SCANCODE_RIGHTBRACKET       = SDL_SCANCODE_RIGHTBRACKET,
	SCANCODE_BACKSLASH          = SDL_SCANCODE_BACKSLASH,
	SCANCODE_NONUSHASH          = SDL_SCANCODE_NONUSHASH,
	SCANCODE_SEMICOLON          = SDL_SCANCODE_SEMICOLON,
	SCANCODE_APOSTROPHE         = SDL_SCANCODE_APOSTROPHE,
	SCANCODE_GRAVE              = SDL_SCANCODE_GRAVE,
	SCANCODE_COMMA              = SDL_SCANCODE_COMMA,
	SCANCODE_PERIOD             = SDL_SCANCODE_PERIOD,
	SCANCODE_SLASH              = SDL_SCANCODE_SLASH,
	SCANCODE_CAPSLOCK           = SDL_SCANCODE_CAPSLOCK,
	SCANCODE_F1                 = SDL_SCANCODE_F1,
	SCANCODE_F2                 = SDL_SCANCODE_F2,
	SCANCODE_F3                 = SDL_SCANCODE_F3,
	SCANCODE_F4                 = SDL_SCANCODE_F4,
	SCANCODE_F5                 = SDL_SCANCODE_F5,
	SCANCODE_F6                 = SDL_SCANCODE_F6,
	SCANCODE_F7                 = SDL_SCANCODE_F7,
	SCANCODE_F8                 = SDL_SCANCODE_F8,
	SCANCODE_F9                 = SDL_SCANCODE_F9,
	SCANCODE_F10                = SDL_SCANCODE_F10,
	SCANCODE_F11                = SDL_SCANCODE_F11,
	SCANCODE_F12                = SDL_SCANCODE_F12,
	SCANCODE_PRINTSCREEN        = SDL_SCANCODE_PRINTSCREEN,
	SCANCODE_SCROLLLOCK         = SDL_SCANCODE_SCROLLLOCK,
	SCANCODE_PAUSE              = SDL_SCANCODE_PAUSE,
	SCANCODE_INSERT             = SDL_SCANCODE_INSERT,
	SCANCODE_HOME               = SDL_SCANCODE_HOME,
	SCANCODE_PAGEUP             = SDL_SCANCODE_PAGEUP,
	SCANCODE_DELETE             = SDL_SCANCODE_DELETE,
	SCANCODE_END                = SDL_SCANCODE_END,
	SCANCODE_PAGEDOWN           = SDL_SCANCODE_PAGEDOWN,
	SCANCODE_RIGHT              = SDL_SCANCODE_RIGHT,
	SCANCODE_LEFT               = SDL_SCANCODE_LEFT,
	SCANCODE_DOWN               = SDL_SCANCODE_DOWN,
	SCANCODE_UP                 = SDL_SCANCODE_UP,
	SCANCODE_NUMLOCKCLEAR       = SDL_SCANCODE_NUMLOCKCLEAR,
	SCANCODE_KP_DIVIDE          = SDL_SCANCODE_KP_DIVIDE,
	SCANCODE_KP_MULTIPLY        = SDL_SCANCODE_KP_MULTIPLY,
	SCANCODE_KP_MINUS           = SDL_SCANCODE_KP_MINUS,
	SCANCODE_KP_PLUS            = SDL_SCANCODE_KP_PLUS,
	SCANCODE_KP_ENTER           = SDL_SCANCODE_KP_ENTER,
	SCANCODE_KP_1               = SDL_SCANCODE_KP_1,
	SCANCODE_KP_2               = SDL_SCANCODE_KP_2,
	SCANCODE_KP_3               = SDL_SCANCODE_KP_3,
	SCANCODE_KP_4               = SDL_SCANCODE_KP_4,
	SCANCODE_KP_5               = SDL_SCANCODE_KP_5,
	SCANCODE_KP_6               = SDL_SCANCODE_KP_6,
	SCANCODE_KP_7               = SDL_SCANCODE_KP_7,
	SCANCODE_KP_8               = SDL_SCANCODE_KP_8,
	SCANCODE_KP_9               = SDL_SCANCODE_KP_9,
	SCANCODE_KP_0               = SDL_SCANCODE_KP_0,
	SCANCODE_KP_PERIOD          = SDL_SCANCODE_KP_PERIOD,
	SCANCODE_NONUSBACKSLASH     = SDL_SCANCODE_NONUSBACKSLASH,
	SCANCODE_APPLICATION        = SDL_SCANCODE_APPLICATION,
	SCANCODE_POWER              = SDL_SCANCODE_POWER,
	SCANCODE_KP_EQUALS          = SDL_SCANCODE_KP_EQUALS,
	SCANCODE_F13                = SDL_SCANCODE_F13,
	SCANCODE_F14                = SDL_SCANCODE_F14,
	SCANCODE_F15                = SDL_SCANCODE_F15,
	SCANCODE_F16                = SDL_SCANCODE_F16,
	SCANCODE_F17                = SDL_SCANCODE_F17,
	SCANCODE_F18                = SDL_SCANCODE_F18,
	SCANCODE_F19                = SDL_SCANCODE_F19,
	SCANCODE_F20                = SDL_SCANCODE_F20,
	SCANCODE_F21                = SDL_SCANCODE_F21,
	SCANCODE_F22                = SDL_SCANCODE_F22,
	SCANCODE_F23                = SDL_SCANCODE_F23,
	SCANCODE_F24                = SDL_SCANCODE_F24,
	SCANCODE_EXECUTE            = SDL_SCANCODE_EXECUTE,
	SCANCODE_HELP               = SDL_SCANCODE_HELP,
	SCANCODE_MENU               = SDL_SCANCODE_MENU,
	SCANCODE_SELECT             = SDL_SCANCODE_SELECT,
	SCANCODE_STOP               = SDL_SCANCODE_STOP,
	SCANCODE_AGAIN              = SDL_SCANCODE_AGAIN,
	SCANCODE_UNDO               = SDL_SCANCODE_UNDO,
	SCANCODE_CUT                = SDL_SCANCODE_CUT,
	SCANCODE_COPY               = SDL_SCANCODE_COPY,
	SCANCODE_PASTE              = SDL_SCANCODE_PASTE,
	SCANCODE_FIND               = SDL_SCANCODE_FIND,
	SCANCODE_MUTE               = SDL_SCANCODE_MUTE,
	SCANCODE_VOLUMEUP           = SDL_SCANCODE_VOLUMEUP,
	SCANCODE_VOLUMEDOWN         = SDL_SCANCODE_VOLUMEDOWN,
	SCANCODE_KP_COMMA           = SDL_SCANCODE_KP_COMMA,
	SCANCODE_KP_EQUALSAS400     = SDL_SCANCODE_KP_EQUALSAS400,
	SCANCODE_INTERNATIONAL1     = SDL_SCANCODE_INTERNATIONAL1,
	SCANCODE_INTERNATIONAL2     = SDL_SCANCODE_INTERNATIONAL2,
	SCANCODE_INTERNATIONAL3     = SDL_SCANCODE_INTERNATIONAL3,
	SCANCODE_INTERNATIONAL4     = SDL_SCANCODE_INTERNATIONAL4,
	SCANCODE_INTERNATIONAL5     = SDL_SCANCODE_INTERNATIONAL5,
	SCANCODE_INTERNATIONAL6     = SDL_SCANCODE_INTERNATIONAL6,
	SCANCODE_INTERNATIONAL7     = SDL_SCANCODE_INTERNATIONAL7,
	SCANCODE_INTERNATIONAL8     = SDL_SCANCODE_INTERNATIONAL8,
	SCANCODE_INTERNATIONAL9     = SDL_SCANCODE_INTERNATIONAL9,
	SCANCODE_LANG1              = SDL_SCANCODE_LANG1,
	SCANCODE_LANG2              = SDL_SCANCODE_LANG2,
	SCANCODE_LANG3              = SDL_SCANCODE_LANG3,
	SCANCODE_LANG4              = SDL_SCANCODE_LANG4,
	SCANCODE_LANG5              = SDL_SCANCODE_LANG5,
	SCANCODE_LANG6              = SDL_SCANCODE_LANG6,
	SCANCODE_LANG7              = SDL_SCANCODE_LANG7,
	SCANCODE_LANG8              = SDL_SCANCODE_LANG8,
	SCANCODE_LANG9              = SDL_SCANCODE_LANG9,
	SCANCODE_ALTERASE           = SDL_SCANCODE_ALTERASE,
	SCANCODE_SYSREQ             = SDL_SCANCODE_SYSREQ,
	SCANCODE_CANCEL             = SDL_SCANCODE_CANCEL,
	SCANCODE_CLEAR              = SDL_SCANCODE_CLEAR,
	SCANCODE_PRIOR              = SDL_SCANCODE_PRIOR,
	SCANCODE_RETURN2            = SDL_SCANCODE_RETURN2,
	SCANCODE_SEPARATOR          = SDL_SCANCODE_SEPARATOR,
	SCANCODE_OUT                = SDL_SCANCODE_OUT,
	SCANCODE_OPER               = SDL_SCANCODE_OPER,
	SCANCODE_CLEARAGAIN         = SDL_SCANCODE_CLEARAGAIN,
	SCANCODE_CRSEL              = SDL_SCANCODE_CRSEL,
	SCANCODE_EXSEL              = SDL_SCANCODE_EXSEL,
	SCANCODE_KP_00              = SDL_SCANCODE_KP_00,
	SCANCODE_KP_000             = SDL_SCANCODE_KP_000,
	SCANCODE_THOUSANDSSEPARATOR = SDL_SCANCODE_THOUSANDSSEPARATOR,
	SCANCODE_DECIMALSEPARATOR   = SDL_SCANCODE_DECIMALSEPARATOR,
	SCANCODE_CURRENCYUNIT       = SDL_SCANCODE_CURRENCYUNIT,
	SCANCODE_CURRENCYSUBUNIT    = SDL_SCANCODE_CURRENCYSUBUNIT,
	SCANCODE_KP_LEFTPAREN       = SDL_SCANCODE_KP_LEFTPAREN,
	SCANCODE_KP_RIGHTPAREN      = SDL_SCANCODE_KP_RIGHTPAREN,
	SCANCODE_KP_LEFTBRACE       = SDL_SCANCODE_KP_LEFTBRACE,
	SCANCODE_KP_RIGHTBRACE      = SDL_SCANCODE_KP_RIGHTBRACE,
	SCANCODE_KP_TAB             = SDL_SCANCODE_KP_TAB,
	SCANCODE_KP_BACKSPACE       = SDL_SCANCODE_KP_BACKSPACE,
	SCANCODE_KP_A               = SDL_SCANCODE_KP_A,
	SCANCODE_KP_B               = SDL_SCANCODE_KP_B,
	SCANCODE_KP_C               = SDL_SCANCODE_KP_C,
	SCANCODE_KP_D               = SDL_SCANCODE_KP_D,
	SCANCODE_KP_E               = SDL_SCANCODE_KP_E,
	SCANCODE_KP_F               = SDL_SCANCODE_KP_F,
	SCANCODE_KP_XOR             = SDL_SCANCODE_KP_XOR,
	SCANCODE_KP_POWER           = SDL_SCANCODE_KP_POWER,
	SCANCODE_KP_PERCENT         = SDL_SCANCODE_KP_PERCENT,
	SCANCODE_KP_LESS            = SDL_SCANCODE_KP_LESS,
	SCANCODE_KP_GREATER         = SDL_SCANCODE_KP_GREATER,
	SCANCODE_KP_AMPERSAND       = SDL_SCANCODE_KP_AMPERSAND,
	SCANCODE_KP_DBLAMPERSAND    = SDL_SCANCODE_KP_DBLAMPERSAND,
	SCANCODE_KP_VERTICALBAR     = SDL_SCANCODE_KP_VERTICALBAR,
	SCANCODE_KP_DBLVERTICALBAR  = SDL_SCANCODE_KP_DBLVERTICALBAR,
	SCANCODE_KP_COLON           = SDL_SCANCODE_KP_COLON,
	SCANCODE_KP_HASH            = SDL_SCANCODE_KP_HASH,
	SCANCODE_KP_SPACE           = SDL_SCANCODE_KP_SPACE,
	SCANCODE_KP_AT              = SDL_SCANCODE_KP_AT,
	SCANCODE_KP_EXCLAM          = SDL_SCANCODE_KP_EXCLAM,
	SCANCODE_KP_MEMSTORE        = SDL_SCANCODE_KP_MEMSTORE,
	SCANCODE_KP_MEMRECALL       = SDL_SCANCODE_KP_MEMRECALL,
	SCANCODE_KP_MEMCLEAR        = SDL_SCANCODE_KP_MEMCLEAR,
	SCANCODE_KP_MEMADD          = SDL_SCANCODE_KP_MEMADD,
	SCANCODE_KP_MEMSUBTRACT     = SDL_SCANCODE_KP_MEMSUBTRACT,
	SCANCODE_KP_MEMMULTIPLY     = SDL_SCANCODE_KP_MEMMULTIPLY,
	SCANCODE_KP_MEMDIVIDE       = SDL_SCANCODE_KP_MEMDIVIDE,
	SCANCODE_KP_PLUSMINUS       = SDL_SCANCODE_KP_PLUSMINUS,
	SCANCODE_KP_CLEAR           = SDL_SCANCODE_KP_CLEAR,
	SCANCODE_KP_CLEARENTRY      = SDL_SCANCODE_KP_CLEARENTRY,
	SCANCODE_KP_BINARY          = SDL_SCANCODE_KP_BINARY,
	SCANCODE_KP_OCTAL           = SDL_SCANCODE_KP_OCTAL,
	SCANCODE_KP_DECIMAL         = SDL_SCANCODE_KP_DECIMAL,
	SCANCODE_KP_HEXADECIMAL     = SDL_SCANCODE_KP_HEXADECIMAL,
	SCANCODE_LCTRL              = SDL_SCANCODE_LCTRL,
	SCANCODE_LSHIFT             = SDL_SCANCODE_LSHIFT,
	SCANCODE_LALT               = SDL_SCANCODE_LALT,
	SCANCODE_LGUI               = SDL_SCANCODE_LGUI,
	SCANCODE_RCTRL              = SDL_SCANCODE_RCTRL,
	SCANCODE_RSHIFT             = SDL_SCANCODE_RSHIFT,
	SCANCODE_RALT               = SDL_SCANCODE_RALT,
	SCANCODE_RGUI               = SDL_SCANCODE_RGUI,
	SCANCODE_MODE               = SDL_SCANCODE_MODE,
	SCANCODE_AUDIONEXT          = SDL_SCANCODE_AUDIONEXT,
	SCANCODE_AUDIOPREV          = SDL_SCANCODE_AUDIOPREV,
	SCANCODE_AUDIOSTOP          = SDL_SCANCODE_AUDIOSTOP,
	SCANCODE_AUDIOPLAY          = SDL_SCANCODE_AUDIOPLAY,
	SCANCODE_AUDIOMUTE          = SDL_SCANCODE_AUDIOMUTE,
	SCANCODE_MEDIASELECT        = SDL_SCANCODE_MEDIASELECT,
	SCANCODE_WWW                = SDL_SCANCODE_WWW,
	SCANCODE_MAIL               = SDL_SCANCODE_MAIL,
	SCANCODE_CALCULATOR         = SDL_SCANCODE_CALCULATOR,
	SCANCODE_COMPUTER           = SDL_SCANCODE_COMPUTER,
	SCANCODE_AC_SEARCH          = SDL_SCANCODE_AC_SEARCH,
	SCANCODE_AC_HOME            = SDL_SCANCODE_AC_HOME,
	SCANCODE_AC_BACK            = SDL_SCANCODE_AC_BACK,
	SCANCODE_AC_FORWARD         = SDL_SCANCODE_AC_FORWARD,
	SCANCODE_AC_STOP            = SDL_SCANCODE_AC_STOP,
	SCANCODE_AC_REFRESH         = SDL_SCANCODE_AC_REFRESH,
	SCANCODE_AC_BOOKMARKS       = SDL_SCANCODE_AC_BOOKMARKS,
	SCANCODE_BRIGHTNESSDOWN     = SDL_SCANCODE_BRIGHTNESSDOWN,
	SCANCODE_BRIGHTNESSUP       = SDL_SCANCODE_BRIGHTNESSUP,
	SCANCODE_DISPLAYSWITCH      = SDL_SCANCODE_DISPLAYSWITCH,
	SCANCODE_KBDILLUMTOGGLE     = SDL_SCANCODE_KBDILLUMTOGGLE,
	SCANCODE_KBDILLUMDOWN       = SDL_SCANCODE_KBDILLUMDOWN,
	SCANCODE_KBDILLUMUP         = SDL_SCANCODE_KBDILLUMUP,
	SCANCODE_EJECT              = SDL_SCANCODE_EJECT,
	SCANCODE_SLEEP              = SDL_SCANCODE_SLEEP,
	SCANCODE_APP1               = SDL_SCANCODE_APP1,
	SCANCODE_APP2               = SDL_SCANCODE_APP2
};

void input_init(void);
void input_cleanup(void);
int  input_mousebutton_state_get(uint button, int state_type);
int  input_key_state_get(int key, int state_type);
void input_mouse_pos_get(int* xpos, int* ypos);
void input_mouse_delta_get(int* xpos, int* ypos); // Use with relative mouse mode
void input_mouse_pos_set(int xpos, int ypos);
void input_mouse_mode_set(enum Mouse_Mode mode);
int  input_mouse_mode_get(void);
void input_update(void);
int  input_map_state_get(const char* map_name, int state);
void input_map_create(const char* name, int* keys, size_t num_keys);
int  input_map_remove(const char* name);
int  input_map_keys_set(const char* name, int* keys, int num_keys);
int  input_map_name_set(const char* name, const char* new_name);

#endif
