#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"

#ifdef __linux__
#define COLOURED_STDOUT
#endif

#ifdef COLOURED_STDOUT

#define COL_RED     "\e[31m"
#define COL_GREEN   "\e[32m"
#define COL_YELLOW  "\e[33m"
#define COL_DEFAULT "\e[39m"
#define COL_CYAN    "\e[36m"
#define COL_RESET   "\e[0m"

#else

#define COL_RED     ""
#define COL_GREEN   ""
#define COL_YELLOW  ""
#define COL_DEFAULT ""
#define COL_CYAN    ""
#define COL_RESET   ""

#endif

static void log_message_callback_stub(const char* message, va_list args);
static void log_warning_callback_stub(const char* warning_message, va_list args);
static void log_error_callback_stub(const char* context, const char* message, va_list args);

static FILE*          log_file         = NULL;
static Log_Message_CB message_callback = log_message_callback_stub;
static Log_Warning_CB warning_callback = log_warning_callback_stub;
static Log_Error_CB   error_callback   = log_error_callback_stub;

#define MAX_LOG_FILE_PATH_LEN 512

void log_init(const char* log_file_name, const char* user_directory)
{
	char log_file_path[MAX_LOG_FILE_PATH_LEN] = {'\0'};
    snprintf(log_file_path, MAX_LOG_FILE_PATH_LEN, "%s/%s", user_directory, log_file_name);
	log_file = fopen(log_file_path, "w");
	if(!log_file)
	{
        log_to_stdout("ERR: (log:init): Failed to create log file at %s", user_directory);
	}
	else
	{
		time_t current_time;
		time(&current_time);
		fprintf(log_file, "Log Initialized at %s\n", ctime(&current_time));
		fflush(log_file);
	}
    // Disable stdout buffering
    setbuf(stdout, NULL);
}

void log_cleanup(void)
{
	if(log_file) 
	{
		time_t current_time;
		time(&current_time);
		fprintf(log_file, "\nLog closing at %s\n", ctime(&current_time));
		fflush(log_file);
		fclose(log_file);
	}
}

void log_to_stdout(const char* message, ...)
{
	printf("%sMSG : ", COL_CYAN);
	va_list list;
	va_start(list, message);
	vprintf(message, list);
	va_end(list);
	printf("\n%s", COL_RESET);
}

void log_raw(const char* str, ...)
{
	va_list console_list, file_list;
	va_start(console_list, str);
	va_copy(file_list, console_list);
	vfprintf(log_file, str, file_list);
	vprintf(str, console_list);
	va_end(console_list);
	va_end(file_list);
	fflush(log_file);
}

void log_message(const char* message, ...)
{
	printf("%sMSG : ", COL_DEFAULT);
	fprintf(log_file, "MSG: ");
	va_list console_list, file_list;
	va_start(console_list, message);
	va_copy(file_list, console_list);
	vfprintf(log_file, message, file_list);
	vprintf(message, console_list);
	message_callback(message, console_list);
	va_end(console_list);
	va_end(file_list);
	printf("\n%s", COL_RESET);
	fprintf(log_file, "\n");
	fflush(log_file);
}

void log_warning(const char* message, ...)
{
	printf("%sWRN : ", COL_YELLOW);
	fprintf(log_file, "WRN: ");
	va_list console_list, file_list;
	va_start(console_list, message);
	va_copy(file_list, console_list);
	vfprintf(log_file, message, file_list);
	vprintf(message, console_list);
	warning_callback(message, console_list);
	va_end(console_list);
	va_end(file_list);
	printf("\n");
	fprintf(log_file, "\n");
	fflush(log_file);
}

void log_error(const char* context, const char* error, ...)
{
	printf("%sERR (%s) : ", COL_RED, context);
	fprintf(log_file, "ERR (%s) : ", context);
	va_list console_list, file_list;
	va_start(console_list, error);
	va_copy(file_list, console_list);
	vfprintf(log_file, error, file_list);
	vprintf(error, console_list);
	error_callback(context, error, console_list);
	va_end(console_list);
	va_end(file_list);
	printf("\n%s", COL_RESET);
	fprintf(log_file, "\n");
	fflush(log_file);
}

FILE* log_file_handle_get(void)
{
    return log_file;
}

void log_file_handle_set(FILE* file)
{
    log_file = file;
}

void log_message_callback_set(Log_Message_CB callback)
{
	if(callback)
		message_callback = callback;
}

void log_warning_callback_set(Log_Warning_CB callback)
{
	if(callback)
		warning_callback = callback;
}
void log_error_callback_set(Log_Error_CB callback)
{
	if(callback)
		error_callback = callback;
}

void log_message_callback_stub(const char* message, va_list args)
{
	// This is just a stub in-case no callback has been set
}

void log_warning_callback_stub(const char* warning_message, va_list args)
{
	// This is just a stub in-case no callback has been set
}

void log_error_callback_stub(const char* context, const char* message, va_list args)
{
	// This is just a stub in-case no callback has been set
}