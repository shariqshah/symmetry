#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "platform.h"

#ifdef __linux__

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

static FILE* log_file = NULL;

void log_init(const char* log_file_name)
{
	char* dir = platform_user_directory_get("SS_Games", "Symmetry");
	char log_file_path[512] = {'\0'};
	snprintf(log_file_path, 512, "%s/%s", dir, log_file_name);
	log_file = fopen(log_file_path, "w");
	if(!log_file)
	{
		log_to_stdout("ERR: (log:init): Failed to create log file at %s", dir);
	}
	else
	{
		time_t current_time;
		time(&current_time);
		fprintf(log_file, "Log Initialized at %s\n", ctime(&current_time));
		fflush(log_file);
	}
	if(dir) free(dir);
}

void log_cleanup(void)
{
	if(log_file) fclose(log_file);
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

void log_message(const char* message, ...)
{
	printf("%sMSG : ", COL_DEFAULT);
	fprintf(log_file, "MSG: ");
	va_list console_list, file_list;
	va_start(console_list, message);
	va_copy(file_list, console_list);
	vfprintf(log_file, message, file_list);
	vprintf(message, console_list);
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
	va_end(console_list);
	va_end(file_list);
	printf("\n%s", COL_RESET);
	fprintf(log_file, "\n");
	fflush(log_file);
}
