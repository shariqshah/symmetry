#include "parser.h"
#include "hashmap.h"
#include "log.h"

#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 512
#define MAX_VALUE_LEN 512
#define QUOTE(str) #str
#define KEY_LEN_STR(LEN) QUOTE(LEN)

bool parser_load(FILE* file, const char* filename, Parser_Assign_Func assign_func, bool return_on_emptyline, int current_line)
{
	if(!file)
	{
		log_error("parser:load", "Invalid file handle for file %s", filename);
		return false;
	}

	if(!assign_func)
	{
		log_error("parser:load", "No assign function provided to load file %s", filename);
		return false;
	}
	
	/* Read line by line, ignore comments */
	char format_str[64];
	char key_str[HASH_MAX_KEY_LEN];
	char value_str[MAX_VALUE_LEN];
	char line_buffer[MAX_LINE_LEN];
	memset(key_str, '\0', HASH_MAX_KEY_LEN);
	memset(line_buffer, '\0', MAX_LINE_LEN);
	snprintf(format_str, 64, " %%%d[^: ] : %%%d[^\n]", HASH_MAX_KEY_LEN, MAX_VALUE_LEN);
	
	while(fgets(line_buffer, MAX_LINE_LEN - 1, file))
	{
		current_line++;
		memset(key_str, '\0', HASH_MAX_KEY_LEN);
		memset(value_str, '\0', HASH_MAX_KEY_LEN);

		if(strlen(line_buffer) == 0 || isspace(line_buffer[0]) != 0)
		{
			if(return_on_emptyline)
				return true;
			else
				continue;
		}
		
		if(line_buffer[0] == '#')
			continue;

		if(sscanf(line_buffer, format_str, key_str, value_str) != 2)
		{
			log_warning("Malformed value in config file %s, line %d", filename, current_line);
			continue;
		}

		assign_func(key_str, value_str, filename, current_line);
	}
	return true;
}
