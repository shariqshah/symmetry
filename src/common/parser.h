#ifndef PARSER_H
#define PARSER_H

#include "common.h"

struct Hashmap;

typedef void (*Parser_Assign_Func)(const char* key, const char* value, const char* filename, int current_line);

bool parser_load(FILE* file, const char* filename, Parser_Assign_Func assign_func, bool return_on_emptyline, int current_line);

#endif
