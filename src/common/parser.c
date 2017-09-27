#include "parser.h"
#include "hashmap.h"
#include "log.h"

#include <string.h>
#include <ctype.h>

#define MAX_LINE_LEN 512
#define MAX_VALUE_LEN 512

struct Parser_Object
{
	int type;
	char* key;
	char* value;
	char* object_data;
};

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

bool parser_load_objects(FILE* file, const char* filename)
{
	/* Note, this is isn't really a proper parser and might have lurking bugs, it just has to be
	   good enough for my needs. For example, multiple opening and closing braces on them
	   same line are fine but opening brace on the same line as type is NOT fine. There
	   are probably several others that i don't know of yet. This is just a temporary solution,
	   i may completely change the format in the future or switch to binary or use something like
	   JSON or TOML etc. */
	
	if(!file)
	{
		log_error("parser:load_objects", "Invalid file handle for file %s", filename);
		return false;
	}

	int current_line = 0;
	char line_buffer[MAX_LINE_LEN];
	char type_str[HASH_MAX_KEY_LEN];
	char obj_str[1024];
	
	while(fgets(line_buffer, MAX_LINE_LEN - 1, file))
	{
		current_line++;
		memset(type_str, '\0', HASH_MAX_KEY_LEN);

		if(strlen(line_buffer) == 0 || isspace(line_buffer[0]) != 0)
		{
			continue;
		}
		
		if(line_buffer[0] == '#')
			continue;

		/* Object type */
		if(sscanf(line_buffer, "%s", type_str) != 1)
		{
			log_warning("Malformed line in config file %s, line %d", filename, current_line);
			continue;
		}

		// Check if type string is valid
		int type_str_len = strnlen(type_str, HASH_MAX_KEY_LEN);
		if(type_str_len < 3 || strncmp(type_str, "{", HASH_MAX_KEY_LEN) == 0 || strncmp(type_str, "}", HASH_MAX_KEY_LEN) == 0)
		{
			log_warning("Invalid object type '%s' on line %d", type_str, current_line);
			continue;
		}

		long obj_beginning = -1;
		long obj_ending    = -1;
		int  obj_begin_expexted_at = current_line;
		bool found_next_before_current_ended = false;
		
		/* Opening brace and closing brace */
		char c = ' ';
		int line_len = strnlen(line_buffer, MAX_LINE_LEN);
		int seek_amount = line_len - type_str_len;
		fseek(file, -seek_amount, SEEK_CUR);
		while(!feof(file))
		{
			c = fgetc(file);
			if(c == '\n')
			{
				current_line++;
				continue;
			}

			if(c == '{')
			{
				obj_beginning = ftell(file) - 1;
				c = ' ';
				while(!feof(file))
				{
					c = fgetc(file);
					if(c == '\n')
					{
						current_line++;
						continue;
					}
					
					/* check if we found opening brace of next object,
					if this is true then it means that this object is missing
					it's closing brace and we should stop */
					if(c == '{')
					{
						found_next_before_current_ended = true;
						break;
					}

					if(c == '}')
					{
						obj_ending = ftell(file);
						break;
					}
				}
				if(obj_ending != -1) break;
			}
		}

		if(obj_beginning == -1)
		{
			log_error("parser:load_object", "Syntax error while loading %s, expected '{' at line %d", filename, obj_begin_expexted_at);
			return false;
		}

		if(obj_ending == -1)
		{
			if(found_next_before_current_ended)
				log_error("parser:load_object", "Syntax error while loading %s, expected '}' before line %d but found '{'", filename, current_line);
			else
				log_error("parser:load_object", "Syntax error while loading %s, expected '}' at line %d", filename, current_line);
			return false;
		}

		memset(obj_str, '\0', 1024);
		memset(line_buffer, '\0', MAX_LINE_LEN);
		fseek(file, obj_beginning, SEEK_SET);
		fread(obj_str, obj_ending - obj_beginning, 1, file);
		log_to_stdout("Object found\nType: %s\n%s\n\n", type_str, obj_str);
	}
	
	return true;
}
