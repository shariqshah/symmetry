#ifndef window_system_H
#define window_system_H

#include <stdbool.h>

struct GLFWwindow;
typedef struct GLFWwindow GLFWwindow;

typedef void (*on_window_close) (void);        // Callback for window close
typedef void (*on_window_resize) (int, int);   // Callback that recieves window resize event
typedef void (*on_key) (int, int , int, int);  // Callback for keyboard events
typedef void (*on_mouse_pos) (double, double); // Callback for mouse position

bool        window_init(const char* title, int width, int height);
void        window_cleanup(void);
void        window_set_size(int width, int height);
void        window_poll_events(void);
void        window_swap_buffers(void);
bool        window_should_close(void);
GLFWwindow* window_get_active(void);

#endif
