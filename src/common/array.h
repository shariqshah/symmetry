#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

/* Private use */
void* array_new_(size_t object_size, int capacity);
void* array_grow_(void** array);
int   array_reset_(void** array, int length); // Resize to length objects whose data is unspecified, previous data is freed
int   array_pop_(void** array);	// Remove object with highest index
int   array_remove_at_(void** array, int index);
void  array_match_len_cap_(void** array);
void  array_inc_cap_by_(void** array, int cap);
int   array_copy_(void* arr_src, void** arr_dest);

/* Public Api */
#define array_new(type)                  (type*) array_new_(sizeof(type), 0); // Use this for array creation
#define array_new_cap(type, capacity)    (type*) array_new_(sizeof(type), capacity); // Use this for array with specific length and capacity
#define array_clear(array)               array_reset(array, 0)
#define array_grow(array, type)          (type*) array_grow_((void**)&array)
#define array_push(array, value, type)   {type* new_val = array_grow(array, type); \
		                                  *new_val      = value;}
#define array_get_last(array, type)      (type*) (&array[array_len(array)])
#define array_set_last(array, val, type) {type* last = array_get_last(array, type);	\
		                                  *last = val}
#define array_pop(array)                 array_pop_((void**)&array)
#define array_remove_at(array, index)    array_remove_at_((void**)&array, index);
#define array_reset(array, length)       array_reset_((void**)&array, length);
#define array_match_len_cap(array)       array_match_len_cap_((void**)&array);
#define array_inc_cap_by(array, cap)     array_inc_cap_by_((void**)&array, cap);
#define array_copy(src, dest)            array_copy_(src, (void**)&dest);
int  array_len(void* array);
int  array_capacity(void* array);
void array_free(void* array);

/* TODO: function to increase capacity of array by certain number similar to reserve in stl vector? */

#endif
