#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "array.h"

struct Array
{
	int    capacity;	// current capacity i.e memory allocated
	int    length;		// current length of array
	size_t object_size; // size per element
	char   data[];		// actual data
};

#define ARRAY_MIN_CAPACITY 2

static struct Array* array_get_ptr(void* array_data);
static struct Array* array_reallocate(struct Array* array);
static void*         array_top(struct Array* array);

static struct Array* array_get_ptr(void* array_data)
{
	return (struct Array*)((char*)array_data - offsetof(struct Array, data));
}

void* array_new_(size_t object_size, int capacity)
{
	int initial_capacity    = capacity == 0 ? ARRAY_MIN_CAPACITY : capacity;
	struct Array* new_array = malloc(sizeof(*new_array) + (object_size * initial_capacity));
	new_array->object_size  = object_size;
	new_array->length       = capacity;
	new_array->capacity     = initial_capacity;
	
	return new_array->data;
}

void array_free(void* array)
{
	struct Array* array_ptr = array_get_ptr(array);
	free(array_ptr);
}

void* array_top(struct Array* array)
{
	return array->data + (array->object_size * (array->length - 1));
}

void* array_grow_(void** array)
{
	struct Array* array_ptr = array_get_ptr(*array);
	/* if capacity is full, double size */
	if(++array_ptr->length > array_ptr->capacity)
	{
		array_ptr->capacity = array_ptr->capacity << 1; /* LShift by 1 means (number * number) */
		char* new_data = realloc(array_ptr, sizeof(*array_ptr) + (array_ptr->object_size * array_ptr->capacity));
		if(new_data)
		{
			array_ptr = (struct Array*)new_data;
			*array = array_ptr->data; /* update the original pointer to the new address */
		}
		else
		{
			array_ptr->length--;
			array_ptr->capacity = array_ptr->capacity >> 1;
			/* TODO: Error handling here! */
		}
	}
	/* Return new pointer to data */
	return array_top(array_ptr);
}

int array_reset_(void** array, int length)
{
	struct Array* array_ptr = array_get_ptr(*array);
	size_t object_size = array_ptr->object_size;
	int new_capacity = length < ARRAY_MIN_CAPACITY ? ARRAY_MIN_CAPACITY : length;
	int new_length = new_capacity;
	array_free(*array);
	array_ptr = calloc(1, sizeof(*array_ptr) + (new_capacity * object_size));
	if(array_ptr)
	{
		array_ptr->length = new_length;
		array_ptr->capacity = new_capacity;
		array_ptr->object_size = object_size;
		*array = array_ptr->data;
	}
	
	return array_ptr ? 1 : 0;
}

int array_pop_(void** array)
{
	struct Array* array_ptr = array_get_ptr(*array);
	int success = 1;
	if(array_ptr->length > 0)
	{
		array_ptr->length--;
		if(!(array_ptr = array_reallocate(array_ptr)))
			success = 0;
		else
			*array = array_ptr->data;
	}
	return success;
}

struct Array* array_reallocate(struct Array* array)
{
	/* If capacity is too big i.e. 4 times larger than length, halve it */
	if((array->length << 2) < array->capacity && array->capacity > ARRAY_MIN_CAPACITY)
	{
		array->capacity = array->capacity >> 1;
		array = realloc(array, sizeof(*array) + (array->object_size * array->capacity));
		/* TODO: Maybe error handling here? */
	}
	return array;
}

int array_remove_at_(void** array, int index)
{
	struct Array* array_ptr = array_get_ptr(*array);
	int success = 1;
	if(array_ptr->length > 0 && index <= array_ptr->length && index >= 0)
	{
		int next_index = index + 1;
		if(next_index < array_ptr->length)
		{
			char* current_location   = array_ptr->data + (array_ptr->object_size * index);
			char* location_after_obj = current_location + array_ptr->object_size;
			memmove(current_location,
					location_after_obj,
					array_ptr->object_size * (array_ptr->length - next_index));
			array_ptr->length--;
			if(!(array_ptr = array_reallocate(array_ptr)))
				success = 0;
			else
				*array = array_ptr->data;
		}
		else
		{
			if(!array_pop(*array))
				success = 0;
		}
	}
	else
	{
		success = 0;
	}
	return success;
}

void* array_begin(struct Array* array)
{
	return array->data;
}

void* array_end(struct Array* array)
{
	return array->data + (array->object_size * array->length);
}

int array_len(void* array)
{
	struct Array* array_ptr = array_get_ptr(array);
	return array_ptr->length;
}

int array_capacity(void* array)
{
	struct Array* array_ptr = array_get_ptr(array);
	return array_ptr->capacity;
}

void array_match_len_cap_(void** array)
{
	struct Array* array_ptr = array_get_ptr(*array);
	array_ptr->length = array_ptr->capacity;
}

void array_inc_cap_by_(void** array, int cap)
{
	struct Array* array_ptr = array_get_ptr(*array);
	if(cap > 0)
	{
		array_ptr->capacity += cap;
		char* new_data = realloc(array_ptr,
								 sizeof(*array_ptr) + (array_ptr->object_size * array_ptr->capacity));
		if(new_data)
		{
			array_ptr = (struct Array*)new_data;
			*array = array_ptr->data; /* update the original pointer to the new address */
		}
		else
		{
			array_ptr->capacity -= cap;
			/* TODO: Error handling here! */
		}
	}
}

int array_copy_(void* array_src, void** array_dest)
{
	int success = 0;
	struct Array* src = array_get_ptr(array_src);
	struct Array* dest = array_get_ptr(*array_dest);
	
	/* determine if copy is possible */
	if(src->object_size != dest->object_size)
		return success;

	if((src->length > dest->length) && (dest->capacity < src->length))
	{
		array_free(*array_dest);
		*array_dest = array_new_(src->object_size, src->capacity);
		dest = *array_dest;
	}
	memcpy(dest->data, src->data, src->object_size * src->length);
	dest->length = src->length;
	return success;
}
