#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdbool.h>

typedef struct
{
	char*        data;		    // actual data
	unsigned int capacity;		// current capacity i.e memory allocated
	unsigned int length;		// current length of array
	size_t       object_size;   // size per element
} Array;

Array* array_new_(size_t object_size);
#define array_new(type) array_new_(sizeof(type)); // Use this for array creation
void array_free(Array* array);

// All the macros with _val return by value while the function returns pointer
void* array_get(Array* array, unsigned int index);
#define array_get_val(array, type, index) (*((type*) array_get(array, index)))

void* array_top(Array* array);
#define array_top_val(array, type) (*((type*) array_top(array)))

void* array_add(Array* array);
#define array_add_val(array, type) (*((type*) array_add(array)))

void array_reset(Array* array, unsigned int length); // Resize to length objects whose data is unspecified
#define array_clear(array) array_reset(array, 0)

bool array_pop(Array* array);	// Remove object with highest index
bool array_remove_at(Array* array, unsigned int index);

void* array_begin(Array* array);
void* array_end(Array* array);

void array_sort(Array* array, int (*compar)(const void*, const void*)); // Sort array by providing a comparator function
		                              

#endif
