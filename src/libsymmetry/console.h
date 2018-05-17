#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

#define MAX_CONSOLE_COMMAND_LEN 128
#define MAX_CONSOLE_LINES 1024
#define MAX_CONSOLE_LINE_LEN 256

struct Console
{
	bool  visible;
	float text_region_height;
	float line_height;
	char  console_command_text[MAX_CONSOLE_COMMAND_LEN];
};

void console_init(struct Console* console);
void console_toggle(struct Console* console);
void console_update(struct Console* console, struct Gui_State* gui_state, float dt);
void console_destroy(struct Console* console);


#endif