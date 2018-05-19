#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stdarg.h>

#define MAX_CONSOLE_MESSAGE_LEN 256
#define MAX_CONSOLE_MESSAGES 1024

struct Console
{
	bool  visible;
	float text_region_height;
	float line_height;
	int   current_message_index;
	char  console_command_text[MAX_CONSOLE_MESSAGE_LEN];
	char  console_messages[MAX_CONSOLE_MESSAGES][MAX_CONSOLE_MESSAGE_LEN];
};

void console_init(struct Console* console);
void console_toggle(struct Console* console);
void console_update(struct Console* console, struct Gui_State* gui_state, float dt);
void console_destroy(struct Console* console);
void console_on_log_message(struct Console* console, const char* message, va_list args);


#endif