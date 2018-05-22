#ifndef LOG_H
#define LOG_H

#include <stdio.h>

typedef void (*Log_Message_CB)(const char* message, va_list args);
typedef void (*Log_Warning_CB)(const char* warning_message, va_list args);
typedef void (*Log_Error_CB)(const char* context, const char* error_message, va_list args);

void  log_init(const char* log_file_name, const char* user_directory);
void  log_cleanup(void);
void  log_message(const char* message, ...);
void  log_warning(const char* message, ...);
void  log_error(const char* context, const char* error, ...);
void  log_to_stdout(const char* message, ...); /* Only use when logging is not initialized */
void  log_raw(const char* str, ...);
FILE* log_file_handle_get(void);
void  log_file_handle_set(FILE* file);
void  log_message_callback_set(Log_Message_CB callback);
void  log_warning_callback_set(Log_Warning_CB callback);
void  log_error_callback_set(Log_Error_CB callback);

#endif
