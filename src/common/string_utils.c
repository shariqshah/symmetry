#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "string_utils.h"
#include "array.h"
#include "log.h"

char* str_new(const char* string, ...)
{
	int length       = 0;
	char* new_string = NULL;

	/* First pass, find out the length of the string by passing NULL and zero length */
	va_list list;
	va_start(list, string);
	length = vsnprintf(new_string, 0, string, list);
	va_end(list);

	if(length < 0)
	{
		log_error("str_new", "Could not find length of string");
		return new_string;
	}

	length++;
	new_string = malloc(length);
	if(!new_string)
	{
		log_error("str_new", "Malloc failed, out of memory!");
		new_string = NULL;
		return new_string;
	}
	/* Actual string allocation and copy */
	va_start(list, string);
	length = vsnprintf(new_string, length, string, list);
	if(length < 0)
	{
		log_error("str_new", "Writing to new string failed");
		free(new_string);
		return new_string;
	}
	va_end(list);
	return new_string;
}

char* str_concat(char* string, const char* str_to_concat)
{
	size_t length      = strlen(str_to_concat);
	size_t length_orig = string ? strlen(string) : 0;
	
	char* temp = realloc(string, length + length_orig + 1); /* +1 at the end to cope for null byte */
	if(temp)
	{
		string = temp;
		strcat(string + length_orig, str_to_concat);
	}
	return string;
}

char* str_replace(char* string, const char* pattern, const char* replacement)
{
    int* indices      = array_new(int);
    size_t string_len = strlen(string);

    /* Calculate size of new string and allocate new memory */
    size_t pattern_len     = strlen(pattern);
    size_t replacement_len = strlen(replacement);

    int done = 0;
    char* remaining_string = string;
    while(!done)
    {
	char* location = strstr(remaining_string, pattern);
	if(location)
	{
	    int index = (int)(location - string);
	    array_push(indices, index, int);
	    remaining_string = location + pattern_len; /* Find the next occurance in the remaining string */
	}
	else
	{
	    done = 1;
	}
    }

    int num_indices = array_len(indices);
    if(num_indices > 0)
    {
	size_t string_len_without_pattern = string_len - (pattern_len * num_indices);
	size_t new_string_len = string_len_without_pattern + (replacement_len * num_indices);
	char* new_string = malloc(new_string_len);

	if(new_string)
	{
	    done = 0;
	    int count = 0;
	    int index = indices[count];
	    int prev_index = 0;
	    while(!done)
	    {
		char* source_beg = string + prev_index;
		char* source_end = string + index;

		strncat(new_string, source_beg, (source_end - source_beg));
		strcat(new_string, replacement);

		count++;
		prev_index = index + pattern_len;
			
		if(count == array_len(indices))
		{
		    done = 1;
		    source_beg = string + prev_index;
		    strcat(new_string, source_beg);
		}
		else
		{
		    index = indices[count];
		}
	    }
	    free(string);
	}

	array_free(indices);
	return new_string;
    }
    else
    {
	return NULL;
    }
}
