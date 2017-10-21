#ifndef LOG_H
#define LOG_H

#include <stdio.h>

void  log_init(const char* log_file_name, const char* user_directory);
void  log_cleanup(void);
void  log_message(const char* message, ...);
void  log_warning(const char* message, ...);
void  log_error(const char* context, const char* error, ...);
void  log_to_stdout(const char* message, ...); /* Only use when logging is not initialized */
void  log_raw(const char* str, ...);
FILE* log_file_handle_get(void);
void  log_file_handle_set(FILE* file);

#endif
