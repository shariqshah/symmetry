#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/*
Convenience methods for handling strings
 */

char* str_new(const char* string, ...);
char* str_concat(char* string, const char* str_to_concat);
char* str_replace(char* string, const char* pattern, const char* replacement);

#endif
