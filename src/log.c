#include <stdarg.h>
#include <stdio.h>

#include "log.h"

void log_message(const char* message, ...)
{
	printf("MSG : ");
	va_list list;
	va_start(list, message);
	vprintf(message, list);
	va_end(list);
	printf("\n");
}

void log_warning(const char* message, ...)
{
	printf("WRN : ");
	va_list list;
	va_start(list, message);
	vprintf(message, list);
	va_end(list);
	printf("\n");
}

void log_error(const char* context, const char* error, ...)
{
	printf("ERR (%s) : ", context);
	va_list list;
	va_start(list, error);
	vprintf(error, list);
	va_end(list);
	printf("\n");
}
