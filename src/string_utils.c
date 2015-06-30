#include <stdlib.h>
#include <string.h>

#include "string_utils.h"
#include "array.h"

char* str_new(const char* string)
{
	size_t length = strlen(string);
	char* new_string = malloc(length + 1);
	strcpy(new_string, string);
	return new_string;
}

char* str_concat(char* string, const char* str_to_concat)
{
	size_t length = strlen(str_to_concat);
	size_t length_orig = strlen(string);

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
	Array* indices = array_new(unsigned int);
	size_t string_len = strlen(string);

	/* Calculate size of new string and allocate new memory */
	size_t pattern_len = strlen(pattern);
	size_t replacement_len = strlen(replacement);

	bool done = false;
	char* remaining_string = string;
	while(!done)
	{
		char* location = strstr(remaining_string, pattern);
		if(location)
		{
			unsigned int index = location - string;
			unsigned int* new_index = array_add(indices);
			*new_index = index;
			remaining_string = location + pattern_len; /* Find the next occurance in the remaining string */
		}
		else
		{
			done = true;
		}
	}
	
	if(indices->length > 0)
	{
		size_t string_len_without_pattern = string_len - (pattern_len * indices->length);
		size_t new_string_len = string_len_without_pattern + (replacement_len * indices->length);
		char* new_string = malloc(new_string_len);

		if(new_string)
		{
			done = false;
			unsigned int count = 0;
			unsigned int index = array_get_val(indices, unsigned int, count);
			unsigned int prev_index = 0;
			while(!done)
			{
				 char* source_beg = string + prev_index;
				 char* source_end = string + index;

				 strncat(new_string, source_beg, (source_end - source_beg));
				 strcat(new_string, replacement);

				 count++;
				 prev_index = index + pattern_len;
			
				 if(count == indices->length)
				 {
					  done = true;
					  source_beg = string + prev_index;
					  strcat(new_string, source_beg);
				 }
				 else
				 {
					  index = array_get_val(indices, unsigned int, count);
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







