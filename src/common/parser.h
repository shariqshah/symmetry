#ifndef PARSER_H
#define PARSER_H

#include "common.h"

enum Parser_Object_Type
{
    PO_ENTITY,
    PO_MATERIAL,
    PO_MODEL,
    PO_UNKNOWN
};

struct Parser_Object
{
    int   type;
    struct Hashmap* data;
};

struct Parser
{
    char* filename;
    struct Parser_Object* objects;
};

typedef void (*Parser_Assign_Func)(const char* key, const char* value, const char* filename, int current_line);

bool           parser_load(FILE* file, const char* filename, Parser_Assign_Func assign_func, bool return_on_emptyline, int current_line);
struct Parser* parser_load_objects(FILE* file, const char* filename);
void           parser_free(struct Parser* parser);

#endif
