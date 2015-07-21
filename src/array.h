#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>
#include <stdbool.h>

struct Array
{
	char*        data;		    // actual data
	unsigned int capacity;		// current capacity i.e memory allocated
	unsigned int length;		// current length of array
	size_t       object_size;   // size per element
};


struct Array* array_new_(size_t object_size, int capacity);
#define array_new(type) array_new_(sizeof(type), 0); // Use this for array creation
#define array_new_cap(type, capacity) array_new_(sizeof(type), capacity); // Use this for array with specific capacity
void array_free(struct Array* array);

// All the macros with _val return by value while the function returns pointer
void* array_get(struct Array* array, unsigned int index);
#define array_get_val(array, type, index) (*((type*) array_get(array, index)))
#define array_get_last_val(array, type) array_get_val(array, type, array->length - 1)
#define array_get_raw(array, type) (type*) array->data

void* array_top(struct Array* array);
#define array_top_val(array, type) (*((type*) array_top(array)))

void* array_add(struct Array* array);
#define array_add_val(array, type) (*((type*) array_add(array)))
#define array_push(array, value, type) \
	type* new_val = array_add(array);  \
	*new_val      = value;

void array_reset(struct Array* array, unsigned int length); // Resize to length objects whose data is unspecified
#define array_clear(array) array_reset(array, 0)

bool array_pop(struct Array* array);	// Remove object with highest index
bool array_remove_at(struct Array* array, unsigned int index);

void* array_begin(struct Array* array);
void* array_end(struct Array* array);

void array_sort(struct Array* array, int (*compar)(const void*, const void*)); // Sort array by providing a comparator function
		                              

#endif
