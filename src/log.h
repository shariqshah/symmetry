#ifndef LOG_H
#define LOG_H

void log_init(const char* log_file_name);
void log_cleanup(void);
void log_message(const char* message, ...);
void log_warning(const char* message, ...);
void log_error(const char* context, const char* error, ...);
void log_to_stdout(const char* message, ...); /* Only use when logging is not initialized */

#endif
