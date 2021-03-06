#ifndef CONSOLE_H
#define CONSOLE_H

#include "../common/hashmap.h"
#include "../common/limits.h"

#include <stdbool.h>
#include <stdarg.h>


struct Gui;
struct Console; 

typedef void(*Console_Command_Handler)(struct Console* console, const char* command_text);

enum Console_Message_Type
{
	CMT_NONE = 0,
	CMT_MESSAGE,
	CMT_WARNING,
	CMT_ERROR,
	CMT_COMMAND,
	CMT_MAX
};

struct Console_Message
{
	int  type;
	char message[MAX_CONSOLE_MESSAGE_LEN];
};

struct Console
{
	bool                   visible;
	bool                   scroll_to_bottom;
	float                  text_region_height;
	float                  line_height;
	int                    current_message_index;
	int                    current_history_index;
	int                    current_history_browse_index;
	char                   command_history[MAX_CONSOLE_HISTORY][MAX_CONSOLE_MESSAGE_LEN];
	char                   command_text[MAX_CONSOLE_MESSAGE_LEN];
	struct Console_Message messages[MAX_CONSOLE_MESSAGES];
	struct Hashmap*        commands;
};

void console_init(struct Console* console);
void console_toggle(struct Console* console);
void console_update(struct Console* console, struct Gui* gui_state, float dt);
void console_destroy(struct Console* console);
void console_on_log_message(struct Console* console, const char* message, va_list args);
void console_on_log_warning(struct Console* console, const char* warning_message, va_list args);
void console_on_log_error(struct Console* console, const char* context, const char* error, va_list args);


#endif
