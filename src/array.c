#include <stdlib.h>
#include <string.h>

#include "array.h"

#define ARRAY_MIN_CAPACITY 2

static bool array_reallocate(Array* array);

Array* array_new_(size_t object_size)
{
	Array* newArray = malloc(sizeof(Array));

	newArray->object_size = object_size;
	newArray->length = 0;
	newArray->capacity = ARRAY_MIN_CAPACITY;
	newArray->data = malloc(newArray->object_size * newArray->capacity);

	return newArray;
}

void array_free(Array* array)
{
	free(array->data);
	free(array);
}

void* array_get(Array* array, unsigned int index)
{
	return array->data + (array->object_size * index);
}

void* array_top(Array* array)
{
	return array->data + (array->object_size * (array->length - 1));
}

void* array_add(Array* array)
{
	/* if capacity is full, double size */
	if(++array->length > array->capacity)
	{
		array->capacity = array->capacity << 1; /* LShift by 1 means (number * number) */
		char* new_data = realloc(array->data, array->object_size * array->capacity);
		if(new_data)
		{
			array->data = new_data;
		}
		else
		{
			array->length--;
			array->capacity = array->capacity >> 1;
			/* TODO: Error handling here! */
		}
	}

	/* return new location added */
	return array->data + (array->object_size * (array->length - 1));
}

void array_reset(Array* array, unsigned int length)
{
	free(array->data);
	array->length = length;
	array->capacity = length < ARRAY_MIN_CAPACITY ? ARRAY_MIN_CAPACITY : length;
	array->data = malloc(array->object_size * array->capacity);
}

bool array_pop(Array* array)
{
	bool success = false;
	if(array->length > 0)
	{
		array->length--;
		success = array_reallocate(array);
	}
	return success;
}

bool array_reallocate(Array* array)
{
	bool success = true;
	/* If capacity is too big i.e. 4 times larger than length, halve it */
	if((array->length << 2) < array->capacity && array->capacity > ARRAY_MIN_CAPACITY)
	{
		array->capacity = array->capacity >> 1;
		char* new_data = realloc(array->data, array->object_size * array->capacity);
		if(new_data)
			array->data = new_data;
		else
			success = false;
	}
	return success;
}

bool array_remove_at(Array* array, unsigned int index)
{
	bool success = false;
	if(array->length > 0)
	{
		unsigned int next_index = index + 1;
		if(next_index < array->length)
		{
			char* current_location   = array->data + (array->object_size * index);
			char* location_after_obj = current_location + array->object_size;
			memmove(current_location,
					location_after_obj,
					array->object_size * (array->length - next_index));
			array->length--;
			success = array_reallocate(array);
		}
		else
		{
			success = array_pop(array);
		}
	}
	return success;
}

void* array_begin(Array* array)
{
	return array->data;
}

void* array_end(Array* array)
{
	return array->data + (array->object_size * array->length);
}

void array_sort(Array* array, int (*compar)(const void*, const void*))
{
	qsort(array->data, array->length, array->object_size, compar);
}
